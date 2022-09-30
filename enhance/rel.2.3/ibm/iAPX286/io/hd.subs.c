static char *uportid = "@(#)hd.subs.c	Microport Rev Id 2.2.  4/20/87";
/*
 *      This module contains subroutines that are called
 *      by the WD1010 disk driver.
 *
 *
 *      VERSION #1                      Jeffrey Golden
 *	REVISED - LARRY WEAVER 1/30/86 TO ADD POLLED RAW I/O
 *	Special version for TeleVideo Controller - Weaver - 7/1/86
 *	VTOC and 386 changes conditioned on MP386  1/7/87 lew
 *	Changes to allow for VTOC filesystems
 *	Lances fixes for 386 port
 *
 *	M001	uport!rex	2/13/87
 *		added recognition of drive type 0 from 138B6, lineno 178
 *	M002	uport!dean, Thu Mar 26 11:31:24 PST 1987
 *		changed hdinit to hdbinit to not clash with boot time
 *		hdinit, add added rex's change for Compaq 40 initparms
 *	M003	3/12/87 lew
 *		names changed to hd-- and 286 - 386 made conditional on MP386
 *	M004	uport!mdg, Sun Mar 29 15:58:09 PST 1987
 *		Added a call to splx() where it was necessary.
 *	4/20/87	uport!rex
 *		integrated Locus modifications, LCCFIX and MERGE ifdef's
 *		inserted and turned on LANCEFIX for write pre-comp handling
 *		    and cylinder register initialization
 */

#define	LANCEFIX

#include <sys/misc.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/iobuf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/hd.h>

#ifdef MP386
#include <sys/ivlab.h>
#include <sys/vtoc.h>
#include <sys/alttbl.h>
#endif

#include <sys/tvi.h>
#include <sys/errno.h>

#ifndef MP386
#include <sys/mmu.h>
#endif	/* MP386 */

#include <sys/seg.h>
#include <sys/8259.h>
#define a20bit 0

int db2=0,db3=0;
#ifdef MP386
#include <sys/immu.h>
#define physaddr(x) phystokv((kvtophys(x)))
#else
long mapin(), physaddr();
#endif

extern unsigned char TVI, WNEXTEND;
extern int instatus;	/* interrupt status for polled seeks */
extern  struct i1010cfg  i1010cfg[];      /* per-board configuration */
extern  struct i1010drtab i1010drtab[];   /* per-unit configuration */
extern  struct i1010drtab i1010drdmy[];   /* per-unit configuration */
extern  struct i1010dev  i1010dev[];      /* per-board device-data-structures */
extern  struct  iobuf   i1010tab[];      /* buffer headers per board */
extern  struct  buf     i1010rbuf[];     /* raw buffer headers per board */
extern struct incache inc;		/* current cache contents */
extern  int     i1010fmaj;               /* for board index computation */
extern  struct  i1010dev *i1010bdd[];     /* board-idx -> "dev" map */
extern  unsigned i1010minor[];          /* minor number bit map */
extern  unsigned char partitab[];
extern  struct  i1010slice slice1[];
extern  struct  bad_track_map;
extern  struct  bad_track_map this_track[];    /* bad track map */
extern  struct  bad_tracks i1010bad_track[];    /* bad track map */
extern  short   i1010maxmin;             /* largest minor number posible */
extern unsigned char hdcache[17][512];
static int initial = 1 ; /*set to zero at end of hdbinit */
/* 
 * readcmos
 *  	reads the CMOS ram and returns the value in location offset
 */

readcmos(offset)
int	offset;
{
	outb(CMOSADR,offset);
	return (inb(CMOSDATA));
}

/*
 * hdbinit
 *      Initialize a board, and the parameter blocks.
 */
hdbinit(dev,board)
dev_t dev;
unsigned board;
{
        register struct i1010cfg *cf;
        register struct i1010dev *dd;
        register struct i1010drtab *dr;
        struct iobuf *iobuf;
        int             i;
        unsigned        type,k, x,unit;
BUG("hdbinit");
	dev = minor(dev);
        x = splbio();
        
        cf = &i1010cfg[board];   /* part of configuration */
        dd = &i1010dev[board];   /* i1010dev was zapped during load */

        dd->d_state.s_state = READING_LABEL;
        /*
         * Reset the board
         */

	if(TVI)
	{
		WNEXTEND = 0x20; 
		outb(CONTROL,RESET);
		delay(1);
		outb(CONTROL,0);
	}
	else 
	{
		WNEXTEND = 0xA0;
        	outb(TASKF+0x0206,0x00 | WD_HDW_RESET);
        	delay(1);                               /* hold the reset for 100us */
        	outb(TASKF+0x0206,0x00);      /* clear wd0 reset */
	};
        delay(60);                              /* wait for the controller */
	for (i=60000 ; i ; i--);
	outb(TASKF+6,0x10);
	waitabit(300);
	outb(TASKF+6,0x00);   /* deselect drive */
	waitabit(300);
        splx( x );
        x = splbio();
BUG("hdbinit: post reset");

        /*
         * Set up various constant fields.
         */
        TASKF = cf->c_io_base;  /* local copy */
        dd->d_state.s_level = cf->c_level;      /* local copy */
        dd->d_state.s_bufh = &i1010tab[ board ]; /* back-pointer to hdr */
        dd->d_state.s_active = IO_IDLE;         /* active flag */

        /*
         * initialize buffer header
         */

        iobuf = &i1010tab[ board ];
        iobuf->b_forw = (struct buf *)iobuf;
        iobuf->b_back = (struct buf *)iobuf;
        iobuf->b_actf = (struct buf *)iobuf;
        iobuf->b_actl = (struct buf *)iobuf;
        iobuf->io_nreg = sizeof ( struct i1010eregs );
        iobuf->io_addr = (caddr_t)i1010cfg[ board ].c_io_base;

        /*
         * set board to 'dev' map
         */
        i1010bdd[board] = dd;
	if ( inb(STATUS) & WD_ST_BSY) printf("TIMEOUT AFTER RESET\n");
        dd->d_state.s_exists = ((( inb(STATUS) & WD_ST_BSY)
                        == 0 ) ? 1 : 0 );

        /*
         * init to not open state
         */
        dd->d_state.s_active = IO_IDLE;
	type = readcmos(0x12);	/* get fd drive type */
        for( i = 0; i < NUMSPINDLE; i++ ) { 
                dd->d_state.s_flags[ i ] = 0;
		k =  i ? type & 0xF : type >>4 ;
printf("drive %d type= %d \n",i,k);

                dd->d_drtab[ i ] = &i1010drtab[ i ];	/* leave trigger for renit */

                dr = dd->d_drtab[ i ];
                /*
                 * do a set parms on all drives on all controllers
                 * if the operation is succussful record d_state.s_exists
                 */
		if (k)		/* M001 */
		{		/* bypass unready & noninstalled drives */
		 	setparms(dd,i);
                	dd->d_state.s_devcode[i] = ((( inb(STATUS)
                            & WD_ST_RDY) != 0 ) ? cf->c_devcod[0] : INVALID );
		}
		else
		{
                	dd->d_state.s_devcode[i] =  INVALID ;
			goto endinit;
		};

 BUG("hdbinit:post setparams\n");

                if ((dd->d_state.s_devcode[i] != INVALID)  )
		{
		 recalibrate(dd,i);
			/* master BOOT blk*/
                        readblock(dd,0,0,1,&bblock[0]);

			if (*((unsigned short *) &bblock[0x1FE]) != 0xAA55)
			{
				printf ("\nNO PARTITION TABLE\n");
#ifdef MP386
			if (init_from_VTOC(dd)) goto initparms;
#endif
				goto endinit;
			}
			else
			{
			/*	save partition table	*/
				moveb(&bblock[0x01BE],&partitab[0],66);
			};
#ifdef MP386
			if (init_from_VTOC(dd)) goto initparms;
#endif

                        if (read_pboot_eblock(dd))
			{
				printf ("\nCan't Read Active Partition End Record\n");
				goto endinit;
			} ;
			if (*((unsigned short *) &bblock[0x1FC]) == 0xAA55)
			{

#ifdef MP386
				int length;	
				struct i1010slice *f, *t;
BUG("MOVING SLICE TABLE");
				f= (struct i1010slice *) (&bblock[0] + sizeof(struct i1010drtab));
				t= (struct i1010slice *)&dr->dr_slice[0];
			/* slice 10 to slice 0 */
				length= sizeof(struct i1010slice);
				moveb (f+10,t,length);
			/* slice 0-3 to slice 1-4*/
				length= sizeof(struct i1010slice) * 4;
				moveb (f,t+1,length);
				if (lwdb) for (i=0 ; i<5 ;i++)
				{
					printf(" %d    %d %d    %d %d \n",
						(f+i)->p_fsec , (f+i)->p_nsec,
							(t+i)->p_fsec ,(t+i)->p_nsec );
				}; 
			
#else	/* MP386 */
				char *f,*t;
				struct i1010slice *s;
				int length;
BUG("MOVING SLICE TABLE");
				f= &bblock[0] + sizeof(struct i1010drtab);
				t= (char *)&dr->dr_slice[0];
				length= sizeof(struct i1010slice) * 16;
				moveb (f,t,length);
				if (dd->d_iopb.i_unit==0)
				{
					f+= length;
					t =(char *) &i1010minor[0];
					length = sizeof(i1010minor[0])*40;
					moveb (f,t,length);
#ifdef REMOVE
				/* if swapdev is on this disk, set numbers */
				/* no good reason for swapdev to be on disk 0 */
					if ((major(swapdev) == 0) && (rootdev != swapdev) && (UNIT(minor(swapdev)) == i))
#endif REMOVE
#ifdef LANCEFIX
					if ((major(swapdev) == 0) && (UNIT(minor(swapdev)) == i))
					{
						swplo = 0;
						s = &dr->dr_slice[ SLICE(minor(swapdev))];
						nswap =  s->p_nsec;
					};
#else LANCEFIX
			/* if we're the root, set up swap device */
					if (rootdev == makedev(0,ROOTDEV))
					{
						swapdev = makedev(0,SWAPDEV);
						swplo = 0;
						s = &dr->dr_slice[ SLICE(SWAPDEV)];
						nswap =  s->p_nsec;
					};
#endif	/* LANCEFIX */
				};
#endif	/* MP386 */
			}  /* end if bblock[0x1fc] == 0xAA55 */
			else
                        	get_slice_table(dd);

			if (*((unsigned short *) &bblock[0x1FE]) == 0xAA55)
                        {
BUG("checking drtab");
                        	if (get_drive_table(dd))
	                        read_bad_track_table(dd);
	                };
#ifdef MP386
initparms:
#endif
		setparms(dd,i);
		};
endinit: 
BUG("hdbinit: post table init");
        }
	initial--;
	inc.in_unit = -1 ; /* mark cache invalid */
BUG("hdbinit: all done");
        splx(x);
}

/*
 *  moveb  (from,to,length) moves a structure 
 *
*/
moveb (from, to, length )
 byte *from,*to;
 int length;
{

	for ( ;length;length -- ) *to++ = *from++ ;

}
/*
 * hdstart
 *      Start an I/O request.
 *   Return 1 if nothing to do
 */
hdstart(dd)
register struct i1010dev *dd;
{
        register struct iobuf *bufh;
        register struct buf *bp;
        register unit;
        register dev;
        register unsigned x;
BUG("hdstart");

        bufh = dd->d_state.s_bufh;
        /*
         * if busy or nothing to do then quit
         */
        if( dd->d_state.s_active & IO_BUSY ||
            ( bp = dd->d_curbuf ) == (struct buf *) bufh ) {
                return 1;
        }

        /*
         * get dev and unit
         */
        dev = minor( bp->b_dev );
        unit = UNIT( dev );

        /*
         * check to see if re-init is needed
         */
        if( dd->d_drtab[ unit ] != &i1010drtab[ DRTAB( dev ) ] ) {
                hdsweep( dd, dev );
                return 0;
        }

        x = splbio();
        if( dd->d_state.s_active & IO_BUSY ||
            ( bp = dd->d_curbuf ) == (struct buf *) bufh ) 
                return 1;
        /*
         * must be something to do
         */
	 if (db2==1)
	 printf("T");
	dd->d_state.s_state = NOTHING;
        dd->d_state.s_active |= IO_BUSY;
        bufh->b_errcnt = 0;
        hdio(dd, bp, IO_OP(bp),UNIT(bp->b_dev));
	BUG("WNSTART EXIT");
        splx(x);
	return 0;
}

/*
 * hdsweep
 *      Preform an init-sweep.
 *
 * The WD1010 controller runs a diagnostic and clears its RAM whenever unit 0
 * is initialized.  When floppy units are initialized, some of RAM is cleared.
 * Thus, the simplest solution is to do a complete init-sweep on every re-init.
 *
 * Sweep uses current drtab entries for all units.
 */
hdsweep(dd, dev)
register struct i1010dev *dd;
dev_t   dev;
{
        unsigned        unit;

BUG("hdsweep");
        /*
         * Figure unit & drtab entry.
         */

	dev = minor(dev);
        unit = UNIT(dev);
        dd->d_drtab[unit] = &i1010drtab[DRTAB(dev)];
        /*
         * Do the init-sweep.
         */
        dd->d_state.s_state = INITIALIZING;
        dd->d_state.s_active |= IO_BUSY;
        hdio( dd, (char *)0, INIT_OP, 0 );
}

/*
 * hdio
 *      Mechanics of starting an IO.  Parameters are already stored in "dd".
 *      Starts the WD1010 by setting up the iopb and sending
 *      a wakeup signal to the controller.
 */
hdio(dd, bp, op, unit)
register struct i1010dev *dd;
register struct buf     *bp;
int     op;
int unit;
{
        extern struct seg_desc gdt[];
        register struct i1010slice *p;
        register struct i1010drtab *dr;
        unsigned x;
        char *dest;
BUG("hdio");

        /*
         * protect ourself
         */
        x = splbio();

        /*
         * Set up IOPB.
         * if using a 215G, turn on 24bit mode
         * Note: the calls to physaddr are dangerous if
         *      thay are executed at interrupt level because
         *      the correct user area may not be maped in.
         */
        dr = dd->d_drtab[unit];

        switch(op) {
                case STATUS_OP:
BUG("STATUS-OP");
                        dd->d_iopb.i_addr = physaddr( &dd->d_eregs.e_error );
                        dd->d_iopb.i_funct = WD_READ_OP;
                        outb(TASKF+2,0x02);
                        break;

                case INIT_OP:
BUG("INIT-OP");
                        dd->d_iopb.i_funct = WD_SET_PARM_OP;

	if(lwdb >0) printf ("\nunit: %d dr: %lx",unit,dr);

                        if( (dr->dr_nfhead) > 0x08 ) {
                                outb(( TASKF)+0x0206,0x08);
                        } else {
                                outb(( TASKF)+0x0206,0x00 );
                        }

                        dd->d_iopb.i_actcylinder = (dr->dr_ncyl)-1;
                        dd->d_iopb.i_acthead = (dr->dr_nfhead)-1;
                        dd->d_iopb.i_sector = (dr->dr_nsec);
#ifdef LANCEFIX
/* BIG DISK BUG */
/* without this line, all disks are precomp'd at the 512'th cylinder */
/* because the init routine was the only thing to set the precomp */
/* and worked off a default disk entry */
/* allows Maxtor/Hitachi disks to be used beyond 512'th cylinder */
			outb(TASKF+1,dr->dr_precomp/4);
#endif LANCEFIX
                        outb(TASKF+2,0x11);
                        break;
                        
                case FORMAT_OP:
BUG("FORMAT-OP");
                        dd->d_iopb.i_addr = physaddr( &dd->d_format );
                        dd->d_iopb.i_funct = WD_FORMAT_TRK_OP;  /*M000*/
                        outb(TASKF+2,0x11);
                        break;

                case WRITE_OP:
BUG("WRITE-OP");
                        dd->d_iopb.i_funct = WD_WRITE_OP;       /*M000*/
                        break;

                case READ_OP:
BUG("READ-OP");
                        dd->d_iopb.i_funct = WD_READ_OP;
                        break;
        } /* end of switch (op) */
BUG("CommonOP");

        dd->d_iopb.i_device = dd->d_state.s_devcode[unit] & DEVMASK;
        dd->d_iopb.i_unit = dd->d_state.s_unit[unit];
        
        if (op != STATUS_OP && op != INIT_OP) {
                daddr_t secno;     /* temporary for sector number */

	/* compute sector count */
 		dd->d_iopb.i_xfrcnt = 
			(bp->b_bcount+(dr->dr_secsiz)-1)/(dr->dr_secsiz);
#ifdef MP386
                dd->d_iopb.i_addr = physaddr(paddr(bp));
#else
                dd->d_iopb.i_addr = bp->b_physaddr;
#endif
	    /*	rgc 4/20/87 - removed since dr has already been initialized
		dr = dd->d_drtab[unit];
	     */
                dd->d_iopb.i_cylinder = dr->dr_spc ? bp->b_secno / dr->dr_spc : 0;
                secno = bp->b_secno % dr->dr_spc;
                dd->d_iopb.i_head = secno / dr->dr_nsec;
                dd->d_iopb.i_sector = (secno % dr->dr_nsec)+1;

		dd->d_iopb.i_actual = dd->d_iopb.i_xfrcnt;
		dd->d_iopb.i_usect = dd->d_iopb.i_sector;
                dd->d_iopb.i_uaddr = dd->d_iopb.i_addr;
/* see if request can be satisfied from cache */
		if (bp->b_flags & B_READ)
		{
			if (in_cache(dd))
			{
				if (db2==1)
				printf("W");
				dodone(dd);
				goto hdiox;
			};
		};
        };
if (lwdb) printf("dr: %lx spc: %d  nhead: %d un: %d \n",dr,dr->dr_spc,dr->dr_nfhead,unit);
        dd->d_state.s_opunit = unit;
        if ( dd->d_state.s_bufh->b_active == 0 ) {
                dd->d_state.s_bufh->io_start = lbolt;
        }
        dd->d_state.s_bufh->b_active++;
	if (db2 >1) printf("C: bp: %lx \n",bp);
	hdcmd(dd);	/* start operation */
        if((dd->d_iopb.i_funct == WD_READ_OP) || 
                        (dd->d_iopb.i_funct == WD_WRITE_OP)) {
                dd->d_state.s_substate = SUBSTATE_SEEK;
                outb(STATUS,WD_SEEK_OP);
        } else {
                outb(STATUS,lobyte(dd->d_iopb.i_funct));
        	};
BUG ("WNIO EXIT");
hdiox:
	splx(x);
}
/*
 *
 * here is where we will fill in the command table 
 *
 *
 */
hdcmd(dd)
register struct i1010dev *dd;

{
        register struct i1010drtab *dr;

        dr = dd->d_drtab[ dd->d_iopb.i_unit ];
BUG("WNCMD");
	adjust_xfrcnt(dd);
        if (dd->d_iopb.i_funct != WD_SET_PARM_OP)
        	map_bad_track(dd);
BUG("POST BAD MAP");

{
 if (db3|lwdb)
{
 printf("cy:%d,hd:%d,sc:%d,ad:%lx,fun:%x cnt:%d\n",
		dd->d_iopb.i_actcylinder,dd->d_iopb.i_acthead,
                        dd->d_iopb.i_sector,dd->d_iopb.i_addr,
                                dd->d_iopb.i_funct,
                                dd->d_iopb.i_xfrcnt);
	};
};                         
        busywait(21,dd);
/*
                if( (dd->d_iopb.i_acthead) >= 0x08 ) {
*/
                        if( (dr->dr_nfhead) > 0x08 ) {
                        outb(TASKF+0x0206,0x08);
if (lwdb) printf("CMD: %x: %x\n",TASKF+0x0206,inb(TASKF+0x0206));
                } else {
                        outb(TASKF+0x0206,0x00 );
                }
	outb(TASKF+1,dr->dr_precomp >> 2);
        outb(TASKF+3,dd->d_iopb.i_sector);
        outb(TASKF+4,lobyte(dd->d_iopb.i_actcylinder));
        outb(TASKF+5,hibyte(dd->d_iopb.i_actcylinder) & 0x03);
        outb(TASKF+6,
                (dd->d_iopb.i_acthead) | (dd->d_iopb.i_unit << 4) | 
				WNEXTEND);
        if (dd->d_iopb.i_funct == WD_READ_OP)  
	{
		inc.in_unit = dd->d_iopb.i_unit ;
		inc.in_cyl  = dd->d_iopb.i_cylinder ;
		inc.in_head  = dd->d_iopb.i_head ;
	};
        if ((dd->d_iopb.i_funct == WD_WRITE_OP)  && 
	   ( dd->d_iopb.i_unit == inc.in_unit) &&
		(dd->d_iopb.i_cylinder == inc.in_cyl) &&
			(dd->d_iopb.i_head == inc.in_head))
/* invalidate cache if track written */
			inc.in_unit  = -1; 
       waitabit(30); 
BUG("WNCMD EXIT");
}
adjust_xfrcnt(dd)
register struct i1010dev *dd;
        
{
	unsigned int secno;
        register struct i1010drtab *dr;

BUG("ADJUST_XFRCNT");
        dr = dd->d_drtab[dd->d_iopb.i_unit];
	secno = (dr->dr_nsec - dd->d_iopb.i_sector) + 1;/*sectors in track*/
	if (dd->d_iopb.i_xfrcnt > secno)
	{
		dd->d_iopb.i_cntfill = dd->d_iopb.i_xfrcnt - secno ;
		dd->d_iopb.i_xfrcnt = secno;
	} else dd->d_iopb.i_cntfill = 0;
}

/* see if request can be satisfied from cache */
in_cache(dd)
register struct i1010dev *dd;
{
	unsigned int secno;
        register struct i1010drtab *dr;

	BUG("IN_CACHE");
        dr = dd->d_drtab[dd->d_iopb.i_unit];
	secno = min((dr->dr_nsec - dd->d_iopb.i_usect) + 1,dd->d_iopb.i_actual);    
	  if (db2)
	  printf ( "du: %d iu: %d dc: %d ic: %d dh: %d ih:%d\n",
	  	dd->d_iopb.i_unit , inc.in_unit ,
		dd->d_iopb.i_cylinder , inc.in_cyl ,
			dd->d_iopb.i_head , inc.in_head);
	  if ( dd->d_iopb.i_unit == inc.in_unit &&
		dd->d_iopb.i_cylinder == inc.in_cyl &&
			dd->d_iopb.i_head == inc.in_head)
	  {	/* fill as much as possible from cache */
		BUG("BCOPY");
		if (lwdb)
		printf("addr: %lx usect: %d secno: %d \n",
			&hdcache[dd->d_iopb.i_usect -1][0],
			dd->d_iopb.i_usect ,secno);
#ifdef MP386
		bcopy(&hdcache[dd->d_iopb.i_usect -1][0],
			dd->d_iopb.i_uaddr,
			 secno * dr->dr_secsiz);
#else
		bcopy(&hdcache[dd->d_iopb.i_usect -1][0],
			mapin(dd->d_iopb.i_uaddr,WNDMASEL),
			 secno * dr->dr_secsiz);
#endif
		dd->d_iopb.i_uaddr += secno * dr->dr_secsiz;
		dd->d_iopb.i_actual -= secno;
		if (dd->d_iopb.i_actual == 0)
			return (1); /* cache was sufficient */
		dd->d_iopb.i_usect = 1; /*start next track */
		if( ++dd->d_iopb.i_head > dr->dr_nfhead -1)
		{
			dd->d_iopb.i_head = 0;
			++dd->d_iopb.i_cylinder;
		};
	  };
/*	set up for remainder of read */
	dd->d_iopb.i_sector = 1;
	dd->d_iopb.i_xfrcnt = 17;
	dd->d_iopb.i_addr = physaddr(hdcache);
return 0;
}
map_bad_track(dd)
register struct i1010dev *dd;
{
        unsigned int    cyl;
        unsigned char   trk,board;
        register struct bad_track_map *tp;
BUG("MAP-BAD-TRACK");

        trk = dd->d_iopb.i_head;
        cyl = dd->d_iopb.i_cylinder;
        dd->d_iopb.i_acthead = trk;
        dd->d_iopb.i_actcylinder = cyl;

#ifdef MP386
	if ((dd->d_state.s_flags[dd->d_iopb.i_unit] & SF_VTOC)
		|| (( minor((dd->d_curbuf)->b_dev)) == 0)) /* no map on whole disk */
		 return;
#endif
        tp = &i1010bad_track[dd->d_iopb.i_unit].this_track[0];
        if( tp->bad_cylinder > cyl ) {
                return;

        } else {
                while( tp->bad_cylinder <= cyl ) {
                        if( ( tp->bad_cylinder == cyl ) && 
                                ( tp->bad_track == trk ))  {
                                dd->d_iopb.i_actcylinder = tp->new_cylinder;
                                dd->d_iopb.i_acthead = tp->new_track;
if (lwdb <0 )
printf("bcyl %d ncyl %d bhead %d nhead %d\n", cyl,tp->new_cylinder,trk,tp->new_track);
                                return;

                        } else {
                                ++tp;
                        }
                }
        }
}

readblock(dd,cyl,head,sec,buf)
register struct i1010dev *dd;
int cyl,head,sec;
unsigned char *buf;
{
        int     stater, i;
if (lwdb)
printf("Readblock: cyl: %d head: %d sec: %d \n",cyl,head,sec);
	stater = dd->d_state.s_state;
	dd->d_state.s_state = READING_LABEL; /* throw away interrupt*/
        dd->d_iopb.i_sector = sec;
        dd->d_iopb.i_actcylinder = cyl;
        dd->d_iopb.i_acthead = head;
        if(!(i = do_wcommand(dd)))
	{
		hdrdma(physaddr(&buf[0]));
		if (TVI) outb(CONTROL,CBRDY|RGATE);
	}
	else printf ("Failed \n");
	dd->d_state.s_state = stater; 
	return i;
}

#ifdef MP386
writeblock(dd,cyl,head,sec,buf)
register struct i1010dev *dd;
int cyl,head,sec;
unsigned char *buf;
{
        int     savsec,stater,i;
if (lwdb) printf("Writeblock: cyl: %d head: %d sec: %d \n",cyl,head,sec);
	stater = dd->d_state.s_state;
	dd->d_state.s_state = READING_LABEL; /* throw away interrupt*/
        savsec = dd->d_iopb.i_sector ;
        dd->d_iopb.i_sector = sec;
        dd->d_iopb.i_xfrcnt = 1;
        dd->d_iopb.i_actcylinder = cyl;
        dd->d_iopb.i_acthead = head;
	if (!TVI)
	{
        if(!(i = polled_io(dd)))
	{
	 	hdwdma(physaddr(&buf[0]));
	};
	}
	else
	{
		outb(CONTROL,CPUC|IRQA|WGATE);
		waitabit(10);
		outb(CONTROL,WGATE);
		waitabit(10);
		hdwdma(physaddr(&buf[0]));
		outb(CONTROL,CBRDY|WGATE);
        	i = polled_io(dd);
		outb(CONTROL,CPUC|IRQA|WGATE);
		waitabit(10);
		outb(CONTROL,WGATE);
	}; /* end if !TVI else */

	busywait(3,dd);
  dd->d_iopb.i_sector = savsec;
	dd->d_state.s_state = stater; 
	return i;
}
#endif

/* global sectors/track variable */
static int spt;

read_pboot_eblock(dd)
register struct i1010dev *dd;
{
        int     i,k,cyl,hd;
        unsigned char *bbp,*fbp;
BUG("read-pboot");
	bbp = &partitab[0];  /* search for active partition */
        for( i = 0; i < 4; i++ ) {
                if((*bbp) == 0x80) {
                        fbp = bbp;
			break;
        }
        bbp += 0x0010;
        }
	if (i == 4) return (1);
/* last sector of partition is # of sectors in track */
        spt = *(fbp+6) & 0x1F;
        cyl= *(fbp+7) | (((short )(*(fbp+6)) & 0xC0) << 2);
        hd = *(fbp+5);
        k = readblock(dd,cyl,hd,spt,&bblock[0]);
	return(k);
}

/* if there is a drive-table pointer in the partition-end-record
 * copy the drive parameters from the beginning of the PER into
 * the kernel drive tables, leaving the slice pointer unchanged
*/
get_drive_table(dd)
register struct i1010dev *dd;
{
        int     i,j,count;
	register struct i1010drtab *dr;
        unsigned char *fbp,*bbp;
	dr = (struct i1010drtab *)&bblock[0];
	/* check for sanity */
	if (dr->dr_nfhead <= 0 ) return 0;
BUG("head ok");
	if (dr->dr_ncyl <= 0 ) return 0;
BUG("cyl ok");
	if (dr->dr_nsec != 17 ) return 0;
BUG("sec ok");
	if (dr->dr_secsiz != 512 ) return 0;
BUG("size ok");
	if (dr->dr_spc != (dr->dr_nsec * dr->dr_nfhead )) return 0;
BUG("spc ok");
BUG("get-drives");
        bbp = (unsigned char *) &i1010drtab[dd->d_iopb.i_unit];

	fbp = &bblock[0];
	for (i=0 ;
	     i < (sizeof(struct i1010drtab) -
		 sizeof(struct i1010slice *));
	     i++)
		*bbp++ = *fbp++;
	return 1;
}
/*
	release 1.2 version of slice table init */
get_slice_table(dd)
register struct i1010dev *dd;
{
        int     i,j,count;
        unsigned char *bbp,*fbp;
	register struct i1010drtab *dr;
        register struct i1010slice *lsbp,*ldbp;
   /* find active partition */
BUG("get-slices");
	dr = dd->d_drtab[dd->d_iopb.i_unit];
	spt = dr->dr_nsec; 
   	bbp = &partitab[0];
        for(i=0; i<4; i++) {
                if((*bbp) == 0x80) {fbp = bbp; break;};
        	bbp += 0x0010;
        }

       /* the last active partition(/root) is loaded into the slice  entry */

        lsbp = (struct i1010slice *) fbp;
        lsbp++;
#ifdef MP386
        ldbp = &dr->dr_slice[1];
#else
        ldbp = &dr->dr_slice[0];
#endif
        ldbp->p_fsec = lsbp->p_fsec;

/* allow for bad tracks */
        ldbp->p_nsec = lsbp->p_nsec - (ALTTRACKS * spt);
#ifdef MP386
/* slice 0 is entire disk */
	 ldbp = &dr->dr_slice[0]; 
	 ldbp->p_fsec = 0;
	 ldbp->p_nsec = spt * dr->dr_nfhead * dr->dr_ncyl;
#else
/* set up special slice for minordevice 255 - last sector of active partition
*/
	 ldbp = &dr->dr_slice[0x0B]; 
	 ldbp->p_fsec = lsbp->p_fsec + lsbp->p_nsec - spt;
	 ldbp->p_nsec = spt;
/* slices #1-4 are now loaded with the partitions from the master boot record */
        lsbp = (struct i1010slice *) &partitab[8];
        ldbp = &dr->dr_slice[1];
        for(i = 0; i < 4; i++) {
                ldbp->p_fsec = lsbp->p_fsec;
        	ldbp->p_nsec = lsbp->p_nsec /* - BTSECTORS( lsbp->p_nsec) */ ;
                lsbp+= 2;	/* skip 4 words */
                ldbp++;
        }
#endif
       return(0); 
}

read_bad_track_table(dd)
register struct i1010dev *dd;
{
        int     i,j,count;
        unsigned char *adr,*bbp,*fbp;
BUG("read-bad-trax");
        count = 0;
        fbp = &bblock[0x01f6];
        dd->d_iopb.i_sector = *(fbp+1) & 0x1F;
        dd->d_iopb.i_actcylinder = *(fbp+2) | (((short )(*(fbp+1)) & 0xC0) << 2);
        dd->d_iopb.i_acthead = *fbp;
	adr = physaddr((char *)&i1010bad_track[dd->d_iopb.i_unit]); 
if (lwdb >0) printf ("\nadr: %lx ",adr);
	if (dd->d_iopb.i_sector == 16)	/* kludge validity check */
          if (!do_wcommand(dd))
	  {
	  	hdrdma(adr);
		if (TVI) outb(CONTROL,CBRDY|RGATE);
	  };
}
#ifdef MP386

/**********************************************************************/
static int chead, csec ; /* current head & sector */

unsigned char *vget(dd,p,buf)
register struct i1010dev *dd;
unsigned long p;
unsigned char *buf;
{
	int vbyt,vhead,vsector; 
	struct i1010drtab *dr;
BUG("VGET");
	dr = (struct i1010drtab *)&i1010drtab[dd->d_iopb.i_unit];
	vbyt = p % dr->dr_secsiz ; 
	vsector = ((p / dr->dr_secsiz ) % dr->dr_nsec ) + 1;
	vhead =( p / dr->dr_secsiz ) / dr->dr_nsec ; 
	if (vhead > 1) 
	{
		printf("VGET: %lx not in 1st 2 tracks\n",p);
		printf("Track %d Sector %d Byte %d \n",vhead,vsector,vbyt);
		return 0;
	};
	if (!((vhead == chead) && (vsector == csec))) readblock(dd,0,vhead,vsector,buf);
	chead = vhead;
	csec = vsector;
return (&buf[0] + vbyt);	/* adjusted pointer */
}
/**********************************************************************/

/*  Intel Volume  - initialize tables from  VTOC  */
/* By fiat (or maybe renault) the pdinfo structure is in  */
/* cylinder 0, track 1, sector 1 of all hardisks which  */
/* are recognized by decent people. The VTOC follows , but */
/* we check that against the pdinfo VTOC pointer just to have */
/* something to do to postpone doing productive work	*/
/*
/* This code assumes that all tables are wholly contained within
/* a single disk sector.
*/
init_from_VTOC(dd)
register struct i1010dev *dd;
{
struct pdinfo *pd;
struct vtoc *vt;
struct partition *pt;
struct alt_table *at;
struct i1010drtab *dr;
struct i1010slice *sl;
int i,j;
BUG("init_from_VTOC");
	readblock(dd,0,1,1,&bblock[0]);	/* read pdinfo/VTOC block */
	csec = chead = 1;
	pd = (struct pdinfo *)&bblock[0];
/* test validity */
	if (pd->sanity != VALID_PD )
	{
		printf("Invalid PD: %lx \n",pd->sanity); 
		return 0 ; 
	};

/* copy drive params to drive_table */
	dr = (struct i1010drtab *)&i1010drtab[dd->d_iopb.i_unit];
	dr->dr_ncyl = pd->cyls;
	dr->dr_nfhead = pd->tracks;
	dr->dr_nsec = pd->sectors;
	dr->dr_secsiz = pd->bytes;
	dr->dr_spc = dr->dr_nfhead *  dr->dr_nsec ;


/* coyly pretend to look for VTOC, even though we know its right
	behind the sofa */
	vt = (struct vtoc *)vget(dd,pd->vtoc_ptr,&bblock[0]);	
	if (vt->v_sanity != VTOC_SANE)
	{
		printf("Insane VTOC: %lx \n",vt->v_sanity);
		return 0;
	};
/* get partition slices to slice table */
/*	PARTITION	REFERS TO		SLICE

#ifdef MP386
	0		whole disk		0
	1		root			1
	2		swap			2
	3		usr			3
#else
	0		whole disk		10
	1		root			0
	2		swap			1
	3		usr			2
#endif
*/
if (lwdb)
printf("Partitions:\n");
	pt = (struct partition *) &vt->v_part[0];
	sl = dr->dr_slice;
	i = 0 ;
	j = 0;
	for ( i= 0; i < V_NUMPAR; i++ )	/* fill slice table with partitions */
	{
		(sl + i)->p_fsec = (pt+j)->p_start;
		(sl + i)->p_nsec = (pt+j++)->p_size;
	};
	if (rootdev == makedev(0,ROOTDEV))
		nswap = (sl + SWAPDEV)->p_nsec;

/* get alternate sector tables into badtrack spaces */
	at = (struct alt_table *)vget(dd,pd->alt_ptr,
			&i1010bad_track[dd->d_iopb.i_unit]);	
	if (at->sanity != ALT_SANITY)
	{
		printf("Insane Alt.Sector Tbl.: %lx \n",at->sanity);
		return 0;
	};
	dd->d_state.s_flags[dd->d_iopb.i_unit] |= SF_VTOC;	/* mark as VTOC unit */
	return(1);
}
/***********************************************************************/
struct dadrs{ int cyl,head,sect;	};

brkdadr(dd,sector,dad)	/* break sector into disk address */
struct i1010dev *dd;
daddr_t sector;
struct dadrs *dad;
{
	struct i1010drtab *dr;
	int secno;

         dr = dd->d_drtab[dd->d_iopb.i_unit];
         dad->cyl = sector / dr->dr_spc;
         secno = sector % dr->dr_spc;
         dad->head = secno / dr->dr_nsec;
         dad->sect = (secno % dr->dr_nsec)+1;
}

/* Perform spare sector fixups after hardware I/O */
do_spare_sex(read,dd)
int read;		/* true if read, false if write */
struct i1010dev *dd;
{
	int i, j, k;
	struct alt_table *at;
	struct alt_ent *ae;
	struct dadrs alt,bad;
BUG("DO_SPARE_SEX");
/*	Look for any spared sectors in current track
*/
	/* point to table */
	at = (struct alt_table *)&i1010bad_track[dd->d_iopb.i_unit];
	for (i = 0 ; i < at->alts_used ; i++)
	{
		/* point to entry */
		ae = (struct alt_ent *)&(at->alts[i]);
		brkdadr(dd,ae->badsec,&bad);
		brkdadr(dd,ae->goodsec,&alt);
		if (( bad.cyl == dd->d_iopb.i_cylinder)
			&& ( bad.head == dd->d_iopb.i_head))
		{
		/* If read, copy them into track-cache at appropriate point*/
			if (read)
			{
if (lwdb)
printf("BAD: bc: %d  bh: %d bs: %d \n",bad.cyl,bad.head,bad.sect);
if (lwdb)
printf("SPARE_SEX %d %d %d\n", alt.cyl,alt.head,alt.sect);
				readblock(dd,alt.cyl,alt.head,alt.sect,
						&hdcache[bad.sect -1][0]);
			}
			else
			{
		/*	If write, look for spared sectors in this transfer
			 and rewrite */
				if ((bad.sect >= dd->d_iopb.i_usect )
				&& (bad.sect < dd->d_iopb.i_usect
					 + dd->d_iopb.i_actual))
				{
					/* save last sector */
if (lwdb)
printf("BAD: bc: %d  bh: %d bs: %d \n",bad.cyl,bad.head,bad.sect);
if (lwdb)
printf("SPARE_SEX %d %d %d\n", alt.cyl,alt.head,alt.sect);
					j = bad.sect - dd->d_iopb.i_usect;
					writeblock(dd,alt.cyl,alt.head,
						alt.sect,
                				dd->d_iopb.i_uaddr + (512*j));
				}
;
			};
		};
	};	
}

/***********************************************************************/
#endif

/* do a polled read on the device */
do_wcommand(dd)
register struct i1010dev *dd;
{
	int i,j;
BUG("do-wcommand");
	dd->d_iopb.i_funct = WD_READ_OP; /* set up for polled read */
	dd->d_iopb.i_xfrcnt = 1;
	if ( TVI)
	{
		outb(CONTROL,CPUC|IRQA|CBRDY|RGATE);
		for (j = 10; j ; j--) ;/*wajt a bjt****/
		outb(CONTROL,CBRDY|RGATE);
	};
	i = polled_io(dd);
	if (TVI)
	{
		outb(CONTROL,IRQA|RGATE);
		for (j = 10; j ; j--) ;/*wajt a bjt****/
		outb(CONTROL,RGATE);
	};
	return (i);
}
/***********************************************************************/
/* do a polled read or write of one sector */


polled_io(dd)
register struct i1010dev *dd;
{
int i,x,w;
long timer;
struct i1010drtab *dr;

BUG("polled-io");
       	dr = &i1010drtab[dd->d_iopb.i_unit];
	timer = lbolt;
        while(inb(STATUS) & WD_ST_BSY) {
		x = spl0();
          	if (lbolt - timer > 1000)
		{
			printf("BUSY TIMEDOUT\007\n");
			printf("STATUS: %x\n",inb(STATUS));
			reset(dd);
			return(1)      ;
		};
	    splx(x);
        }
if(lwdb)
 printf("Poll: cy:%d,hd:%d,sc:%d,ad:%lx,func: %x\n",
		dd->d_iopb.i_actcylinder,dd->d_iopb.i_acthead,
                        dd->d_iopb.i_sector,dd->d_iopb.i_addr,
                                dd->d_iopb.i_funct);

polled_read:
BUG("polled read");
/*
        if (dd->d_iopb.i_acthead  >= 0x08 ) {
*/
                if( (dr->dr_nfhead) > 0x08 ) {
                        outb(TASKF+0x0206,0x08);
if (lwdb) printf("POLLED:nfhead: %d \n", dr->dr_nfhead);
                } else {
                        outb(TASKF+0x0206,0x00 );
                }
		waitabit(20);
	outb(TASKF+1,dr->dr_precomp >> 2);
        outb(TASKF+2, dd->d_iopb.i_xfrcnt);
	waitabit(20);
        outb(TASKF+3,dd->d_iopb.i_sector);
	waitabit(20);
        outb(TASKF+4,lobyte(dd->d_iopb.i_actcylinder));
	waitabit(20);
        outb(TASKF+5,hibyte(dd->d_iopb.i_actcylinder) & 0x03);
	waitabit(20);
        outb(TASKF+6,
                (dd->d_iopb.i_acthead) | (dd->d_iopb.i_unit << 4) | WNEXTEND);
if(lwdb) printf("SDH: %x \n",(dd->d_iopb.i_acthead) | (dd->d_iopb.i_unit << 4) | WNEXTEND);
       waitabit(300); 
	instatus = 0;
	w=0;
	if (TVI)
	{
		if (dd->d_iopb.i_funct == WD_READ_OP) w = 0xc;
		if (dd->d_iopb.i_funct == WD_WRITE_OP) w = 0x4;
	};

        outb(STATUS,dd->d_iopb.i_funct | w);

	if (dd->d_iopb.i_funct == WD_SET_PARM_OP) return(0);
	timer = lbolt;
        while(!(instatus))
	{
		if (((dd->d_iopb.i_funct == WD_FORMAT_OP)
		 || (dd->d_iopb.i_funct == WD_WRITE_OP))
				 && (inb(STATUS) & WD_ST_DRQ))
			goto pollret;
		x=spl0();
          	if (lbolt - timer > 1000)
		{
			printf("TIMED OUT awaiting INTERRUPT\007\n");
			printf("STATUS: %x\n",inb(STATUS));
			reset(dd);
			splx(x);			/* M004 */
			return(1);
		};
	    splx(x);
        };
            if ((i= inb(STATUS)) & WD_ST_ERROR) 
		{
	printf("\nWN I/O Error:stat %x estat: %x  drstat %x \n",
					 i,inb(TASKF+1),inb(TASKF+6));
			reset(dd);
			return (1)   ;
		};
pollret:
BUG("polled return");
if (lwdb) printf ("RET.STATUS %x i: %x \n", inb(STATUS),i);        
	return (0);
}
/* test and set IO BUSY */
tsbusy(dd)
register struct i1010dev *dd;
{
	int i,x;
	i = 0;
	do
	{
		x=splbio();
		if (!(dd->d_state.s_active & IO_BUSY))
		{
			dd->d_state.s_active |= IO_BUSY;
			i++;
		};
		splx(x);
	} while (i == 0);
}
reset(dd)
register struct i1010dev *dd;
{
	int i,saves;
BUG("RESET");        
	if (lwdb) printf("RESET\n");
	if ( TVI)
	{
		outb(CONTROL,RESET);
		waitabit(100);
		outb(CONTROL,0);
	}
	else
	{
        	outb(TASKF+0x0206,0x00 | WD_HDW_RESET);
		waitabit(100);
        	outb(TASKF+0x0206,0x00);      /* clear wd0 reset */
	};
        waitabit(600);                              /* wait for the controller */
	outb(TASKF+6,0x10);
	waitabit(300);
	outb(TASKF+6,0x00);   /* deselect drive */
	waitabit(300);
	saves = dd->d_state.s_state ;
	dd->d_state.s_state = READING_LABEL; /* ignore interrupts */
	if (lwdb) printf("POST-RESET STATUS %x \n",inb(STATUS));
	for (i= 0; i< 2; i++)
	{
		if (dd->d_state.s_devcode[i] != INVALID)
		{
			setparms(dd,i);
			if (lwdb) printf("POST-SETPARM STATUS %x \n",inb(STATUS));
			recalibrate(dd,i);	/* restore drive */
			if (lwdb) printf("POST-RECAL STATUS %x \n",inb(STATUS));
		};
	};
	dd->d_state.s_state = saves;
}


recalibrate(dd,unit)
register struct i1010dev *dd;
int unit;
{
	long timer;
	int x, saves;
/*   recalibrate drive   */
BUG("RECALIB");
	saves = dd->d_state.s_state ;
	dd->d_state.s_state = RESTORING;
	outb(TASKF+2,0);
	outb(TASKF+3,0);
	outb(TASKF+4,0);
	outb(TASKF+5,0);
	outb(TASKF+6,(unit << 4) | WNEXTEND);
	waitabit(30);
	outb(STATUS,WD_RECAL_OP);
	timer = lbolt;
        while(inb(STATUS) & WD_ST_BSY) {
		x=spl0();
          	if (lbolt - timer > 1000)
		{
			printf("BUSY TIMEDOUT after RECAL\007\n");
			printf("STATUS: %x\n",inb(STATUS));
			return(1)      ;
		};
	    splx(x);
        }
	dd->d_state.s_state = saves;
}
/* set parameters for  current unit */
setparms(dd,i)
register struct i1010dev *dd;
int i;
{
	int precomps;
	long timer;
	int x;
	struct i1010drtab *dr;
                dr = dd->d_drtab[ i ];
/*
	 printf ("\nunit: %d dr: %lx",i,dr);
printf("cyls: %d heads: %d \n",dr->dr_ncyl,dr->dr_nfhead);
*/
		if (dr == 0) return;
                dd->d_state.s_unit[i] = i;
                dd->d_iopb.i_device = DEVWINI;
		dd->d_iopb.i_unit = i;
                precomps = (dr->dr_precomp) >> 2;

                if( (dr->dr_nfhead) > 0x08 ) {
                        outb(TASKF+0x0206,0x08 );
                } else {
                        outb(TASKF+0x0206,0x00 );
                }

#ifdef LANCEFIX
		/* code magically works without these 2 lines because
		 * routine was only called after a reset, and reset
		 * leaves cylinder registers set to 1023
		 */
		outb(TASKF+4,lobyte(dr->dr_ncyl-1));
		outb(TASKF+5,hibyte(dr->dr_ncyl-1) & 0x03);
#endif LANCEFIX
                outb(TASKF+1,lobyte(precomps));
                outb(TASKF+2,dr->dr_nsec);
                outb(TASKF+6,((dr->dr_nfhead)-1) |
                         ((i<<4) & 0x010) | WNEXTEND);
		waitabit(300);	
		x = inb(STATUS);
		if (x & WD_ST_RDY)
		{
                	outb(STATUS,WD_SET_PARM_OP);
		BUG("Waiting NOT BUSY");
       			busywait(22,dd);
			recalibrate(dd,i);
		};
}
waitabit(n)
int n;
{
	for (; n ;n--);

}
