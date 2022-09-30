static char *uportid = "@(#)hd1.c	Microport Rev Id 2.3  4/20/87";

/*
 * Modification History
 *
 *	4/20/87	uport!rex
 *		integrated Locus modifications, LCCFIX and MERGE ifdef's
 *	M008, uport!dean, Sat Apr  4 00:03:07 PST 1987
 *		add code for second drive in hdinit
 *	Modified 31mar87mdg 18h49
 *		Added code to hdinit to initialize the drive table for
 *		the first fixed disk.
 *	M006, uport!dean, Thu Mar 26 13:06:05 PST 1987
 *		changed hdinit to hdbinit to not clash with standard hdinit
 *		at boot time, and added stub of standard hdinit.
 *	3/12/87 lew	
 *		names changed to hd-- and 286 - 386 made conditional on MP386
 *	1/12/87	modified to make TVI a patchable variable:
 *			1 for Televideo systems
 *			0 for non-TVI systems
 *	1/7/87	lew	modified to make 286 - 386 conditional on MP386
 *	M003: uport!rex		Tue Dec 23 13:26:12 PST 1986
 *		Added copyout to the ioctl(I1010_NTRACK) call
 *	M000 physaddr and eoi changes for 386 port
 *
 * ******    modified for Televideo HD controller - Weaver - 7/1/86
 *
 *      WD1010 Driver for System 5 UNIX on the PCAT
 * Modification History:
 *                      Revision #1 - Jeffrey Golden 12/10/85
 *			Revision #2 - Larry weaver 1/30/86
 */
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
#include <sys/tvi.h>
#include <sys/utsname.h>
#include <sys/elog.h>
#include <sys/errno.h>
#include <sys/trap.h>

#ifndef MP386
#include <sys/mmu.h>
#endif 	/* MP386 */

#ifdef MP386
#include <sys/ivlab.h>
#include <sys/vtoc.h>
#include <sys/alttbl.h>
#include <sys/immu.h>
#else
#include <sys/erec.h>
#define NBPSCTR LBLOCK
#endif
#include <sys/seg.h>
#include <sys/misc.h>

unsigned char TVI=0, WNEXTEND = 0xA0 ;
int lwdb = 0; /* debug switch */
int intime = 0; /*interrupt delay*/
int intcount = 0; /* interrupt reentrance count */
extern int db2;
/* get canonical virtual address */
#ifdef MP386
#define physaddr(x) phystokv((kvtophys(x)))
struct  iotime  hdstat[2];         /* status structure */
#else
long physaddr();
extern struct  iotime  hdstat[];         /* status structure */
#endif

#ifdef MP386
#define BIOS_FDPT_LEN 16	/* ROM BIOS fixed disk parm table entry length */
#endif

extern  int     N1010;
extern  struct i1010cfg  i1010cfg[];      /* per-board configuration */
extern  struct i1010drtab i1010drtab[];   /* per-unit configuration */
extern  struct i1010drtab i1010drdmy[];   /* dummy per-unit configuration */
extern  struct i1010dev  i1010dev[];      /* per-board device-data-structures */
extern  struct  iobuf   i1010tab[];       /* buffer headers per board */
extern  struct  buf     i1010rbuf[];      /* raw buffer headers per board */
extern  int     i1010fmaj;                /* for board index computation */
extern  struct  i1010dev *i1010bdd[];     /* board-idx -> "dev" map */
extern  unsigned        i1010minor[];     /* minor number bit map */
extern  struct bad_tracks i1010bad_track[]; /* bad track map */
extern  short   i1010maxmin;              /* largest minor number posible */
extern hdcache[17][512];
int instatus,funk;

static char opencount = 0,firsttime = 1;

#ifdef MP386
/*
 * hdinit
 * To satisfy reference in standard config
 */

/* These variables are initialized here so they won't end up in BSS */
long dpt1 = -1;			/* Ptr to first fixed disk's parm table */
long dpt2 = -1;			/* Ptr to second fixed disk's parm table */

hdinit() {
	char *source;		 			/* Ptr for block move			*/
	struct i1010drtab *dest;			/* Destination ptr for block move	*/

		/* Pick up pointers to the fixed disk drive parameter tables.
		 * This was done in hdinit() but for some reason the pointers
		 * are zeroed by the time we get there. NOTE! The pointers do
		 * NOT get zeroed if the debugger is linked in. Curiouser and
		 * curiouser...
		 */
	if (dpt1 == -1) dpt1 = *(long *)phystokv(0x104);
	if (dpt2 == -1) dpt2 = *(long *)phystokv(0x108);
/* We must initialize the drive table with values from the ROM
 * BIOS drive table.  The drive table is normally initialized in
 * init_from_VTOC() [which is called by hdbinit()], but if there is
 * no VTOC on the fixed disk, it's up to *US* to take care of it.
 */

	if (dpt1 != 0) {
			/* Massage 8086 style segment:offset pointer to a usable
			 * kernel virtual memory pointer. (dpt1 is assigned a value
			 * in mlsetup() in startup.c)
			 */
		source = (char *) phystokv( ((dpt1 >> 12) & 0xFFFF0) +
					(dpt1 & 0xFFFF) );

			/* Copy the disk parms into the table used by the HD driver */
		dest = &i1010drtab[0];

			/* The byte count is the length of one entry in the
			 * ROM BIOS drive table, which is 16 bytes at the time
			 * of this writing.
			 * Now, move them bytes!
			 */
		bcopy(source, (char *) dest, BIOS_FDPT_LEN);

			/* The ROM BIOS drive table doesn't contain all the data
			 * needed in i1010drtab[] so let's compute the remaining
			 * value(s).
			 * There's only one to compute -- # of sectors/cylinder,
			 * computed by multiplying # of sectors/track by # of
			 * heads.
			 */
		 dest->dr_spc = dest->dr_nsec * dest->dr_nfhead;
	}

		/* Now do the same for the second drive */
	if (dpt2 != 0) {
		source = (char *) phystokv( ((dpt2 >> 12) & 0xFFFF0) +
					(dpt2 & 0xFFFF) );

			/* Copy the disk parms into the table used by the HD driver */
		dest = &i1010drtab[1];

			/* The byte count is the length of one entry in the
			 * ROM BIOS drive table, which is 16 bytes at the time
			 * of this writing.
			 * Now, move them bytes!
			 */
		bcopy(source, (char *) dest, BIOS_FDPT_LEN);

			/* The ROM BIOS drive table doesn't contain all the data
			 * needed in i1010drtab[] so let's compute the remaining
			 * value(s).
			 * There's only one to compute -- # of sectors/cylinder,
			 * computed by multiplying # of sectors/track by # of
			 * heads.
			 */
		 dest->dr_spc = dest->dr_nsec * dest->dr_nfhead;
	}
		/* Get back */
	return;
}
#endif	/* MP386 */

/*
 * hdopen
 *      Open a unit.
 */
hdopen(dev, flag)
dev_t   dev;
int     flag;
{
        register struct i1010dev *dd;
        register unsigned       board;
        unsigned unit;
        static char openwait = 0;
        static long bad_map_ptr[2];
BUG("hdopen");
	if(minor(dev) == 255) dev = 11;
        /*
         * see if someone else is opening a disk
         */
	opencount++;
	if (lwdb) printf("WNOPEN:opencount: %d \n",opencount);
        openwait++;
        while ( openwait != 1 ) {
                sleep( (caddr_t)&openwait, PRIBIO+1 );
        }

        /*
         * if this is the first open ever then initialize the world
         */
        if( firsttime ) {
                firsttime--;
                for( board = 0; board < 1; board++ ) {
                        bad_map_ptr[board] = (long) &i1010bad_track[board];
                        hdbinit( minor(dev), board );
                }
        }

        board = BOARD(dev);
        dd = i1010bdd[board];

        /*
         * check range and board present
         */
        if( board >= N1010 || minor(dev) > i1010maxmin || !dd->d_state.s_exists ) {
                printf("hdopen io check fails dev = %x\n",dev);
		printf("board: %d   dev: %d  exists %d \n",board,dev,dd->d_state.s_exists);

                u.u_error = ENXIO;
                return;
        }

        dd->d_state.s_flags[ UNIT( dev ) ] |= SF_OPEN|SF_READY;

        /*
         * set pointer to io stats
         */
        dd->d_state.s_bufh->io_stp = &hdstat[ board ].ios;

        /*
         * if anyone is waiting to open wake them up
         */
        openwait--;
        if ( openwait ) {
                wakeup( (caddr_t)&openwait );
        }
}

/*
 * hdclose
 *      Close a unit, does nothing.
 */
hdclose(dev)
register dev_t  dev;
{
BUG("hdclose");
if (lwdb) printf ("WNCLOSE: OPENCOUNT:%d \n",opencount);
opencount=0; /* only called on last close */
}

int mode = 0;
/*
 * hdstrategy
 *      Queue an I/O Request, and start it if not busy already.
 *      Check legality, and adjust for slice.
 *      Reject request if unit is not-ready.
 */
hdstrategy(bp)
register struct buf *bp;
{
        register struct i1010dev  *dd;
        register struct i1010drtab *dr;
        register struct i1010slice *p;
        register int    a20bit;
        struct  buf     *ap;
        unsigned        bcyl,bhead,sectorcount,x;
/* special kludge to let boot get last sector of active partition */
	if (minor(bp->b_dev) == 255) 
	{ 
		bp->b_dev = 11;	/* point to extra slice */
	}
       	dd = i1010bdd[ BOARD( bp->b_dev ) ];
       	dr = &i1010drtab[ DRTAB( bp->b_dev ) ];
       	p = &dr->dr_slice[ SLICE( bp->b_dev ) ];
        /*
         * update stats
         */
BUG("hdstrategy");
if (lwdb)printf("DEV: %x\n",bp->b_dev);
        hdstat[ BOARD( bp->b_dev ) ].io_cnt++;
        hdstat[ BOARD( bp->b_dev ) ].io_bcnt += btoc( bp->b_bcount );
        bp->b_start = lbolt;

        /*
         * Figure sector number from block number. 
         * Check if ready, and see if fits in slice.
         * Adjust sector # for slice.
         *
         * Note: if format, b_blkno is already the correct sector number.
         */
                bp->b_secno = ( ( bp->b_blkno * NBPSCTR ) / dr->dr_secsiz );

        /*
         * check for not ready, or off the end
         */
        if (((dd->d_state.s_flags[UNIT(bp->b_dev)] & SF_READY) == 0) 
		|| (bp->b_secno > p->p_nsec)) {
		if (lwdb)
		{
			printf("STRAT: NRDY or off end: bsec: %d nsec: %d \n",
            		bp->b_secno , p->p_nsec);
       		printf("slice: %x dev: %x \n", SLICE( bp->b_dev ),bp->b_dev );
		};
                bp->b_flags |= B_ERROR;
                bp->b_error = ENXIO;
                iodone(bp);
                return;
        }
        if (bp->b_secno == p->p_nsec) {
if (lwdb) printf("STRAT: At end: bsec: %d nsec: %d \n",
            bp->b_secno , p->p_nsec);
                if (bp->b_flags & B_READ)
                        bp->b_resid = bp->b_bcount;
                else {
                        bp->b_error = ENXIO;
                        bp->b_flags |= B_ERROR;
                }
                iodone(bp);
                return;
        }
BUG("NOT OFF END");
if (lwdb) printf("\nSTRATEGY:bsec= %ld fsec= %ld nsec= %ld \n",bp->b_secno,p->p_fsec,p->p_nsec);
	/* compute no of sectors required to satisfy byte count */
	bp->b_resid = 0;
 	sectorcount = (bp->b_bcount+(dr->dr_secsiz)-1)/(dr->dr_secsiz);
if (lwdb) printf ("Sectorcount: %d \n",sectorcount);
        while ((bp->b_secno + sectorcount) > p->p_nsec) {
		sectorcount --;
		bp->b_bcount -= dr->dr_secsiz;
		bp->b_resid += dr->dr_secsiz;
        };

	if (sectorcount < 1 )
		{
	printf("Weird EOF- bcount %d secno %d nsec %d\n",bp->b_bcount,bp->b_secno,p->p_nsec); /* shouldnt happen */
		 bp->b_resid = bp->b_bcount;
		 iodone(bp);
		 return;
		};

BUG("NOT EOF");
        bp->b_secno += p->p_fsec;

        a20bit = (int)(dd->d_state.s_devcode[0]) != DEVWINI;
#ifndef MP386
/*	get physical address of buffer		*/
	bp->b_physaddr = PHYSADDR(bp->b_un.b_addr); 
#endif
        /*
         * Add request to queue, and start it.
         */
        x = splbio();
        ap = dd->d_state.s_bufh->b_actf;

                bhead = (bp->b_secno % dr->dr_spc) / dr->dr_nsec;
        		 bcyl = dr->dr_spc ? bp->b_secno / dr->dr_spc : 0;
        /*
         * find the right place to put this buffer into the list
         * by cylinder number and head no.
         */
        while( ap != (struct buf *)dd->d_state.s_bufh )
	   {
			int ahead,acyl;
                ahead = (ap->b_secno % dr->dr_spc) / dr->dr_nsec;
        		acyl = dr->dr_spc ? ap->b_secno / dr->dr_spc : 0;
                if(( bcyl < acyl ) || 
			(bhead < ahead) )
                        ap = ap->av_forw;
                else
                        break;
        }

        /*
         * now insert buffer into the list
         */

        bp->av_back = ap->av_back;
        ap->av_back->av_forw = bp;
        bp->av_forw = ap;
        ap->av_back = bp;

        /*
         * if the controler is not busy then start it
         */
        if( ! ( dd->d_state.s_active & IO_BUSY ) )
	{
	if (db2>1) printf("S: bp: %lx \n",bp);
                dd->d_curbuf = bp;
        };
	 if (db2==1)
	 printf("S");
        hdstart(dd);
        splx(x);
}
/*
 * hdintr
 *      Handle interrupt.
 *
 * Interrupt procedure for 1010 driver.  Gets status of last operation
 * and performs service function according to the type of interrupt.
 * If it was an operation complete interrupt, switches on the current
 * driver state and either declares the operation done, or starts the
 * next operation and sets dd->d_state.s_state to the next state.
 */
hdintr(level)
int     level;                  /* interrupt level */
{
        extern   struct seg_desc gdt[];
        register struct i1010dev *dd;
        register struct buf     *bp, *ap;
        register struct i1010drtab *dr;
        struct iobuf    *bufh;
        unsigned char   device;
        unsigned        board;
        unsigned char   status;
        unsigned char   drv_status;
        unsigned char   estatus;
        unsigned        eoc,i,x,j;
	long ii;
        /*
         * mask the interrupt to prevent re-entering, and
         * ack the interrupt so the next one wont be missed
         * read status port once before the eoi
         */
        x = splbio();
        
        /*
         * find the board for this interrupt level
         */
        for( board = 0; i1010cfg[ board ].c_level != level; board++);
        dd = &i1010dev[ board ];
#ifndef MP386
#ifndef	PICFIX1
	eoi(level);
#endif /* ! PICFIX1 */ 
#endif
        /*
         * Pick up the controller status.
         */
	if (!TVI)
	{
		drv_status = inb(TASKF+6);
		estatus = inb(TASKF+1);
        	status = inb(STATUS);
	};

if (lwdb) printf("I");
        bufh = dd->d_state.s_bufh;
        bp = dd->d_curbuf;
        dr = dd->d_drtab[dd->d_state.s_opunit];
	if (db2==1) printf("I");

        switch(dd->d_state.s_state) {

                /*
                 * Normal operation complete interrupt.
                 * Check status for error; on error get
                 * status information from controller.
                 * Otherwise declare operation done.
                 */

                case NOTHING:
                if((dd->d_state.s_active &IO_BUSY) == 0)
		{
		goto intex;
		};
		if (TVI )
		{
			if ((eoc = inb(0x3f7) & 0x10))
			{
	       			status = inb(STATUS);
	       			estatus = inb(TASKF+1);
				drv_status = inb(TASKF+6);
			} else goto skipstatus;
		};

        	if (status & WD_ST_ERROR) 
		{
			printf("\nDisk I/O Error: Fun: %x : Cy: %d Hd: %d Sec: %d Status: %x Estat: %x Drstat: %x\n",
				dd->d_iopb.i_funct,dd->d_iopb.i_actcylinder,
				dd->d_iopb.i_acthead,dd->d_iopb.i_sector,
				status , estatus, drv_status);
	/* no retries for now */
intabort:
			bp->b_error = EIO;
			bp->b_flags |= B_ERROR;
			reset(dd);
			goto mioabort;	/* screw Dykstra !!! */
		};
skipstatus:
/********************************************************************/
/* If no error    */
                if(dd->d_state.s_substate == SUBSTATE_SEEK) {
BUG("SUBSTATE_SEEK");
			if (bp->b_flags & B_DONE) 
			{
				printf("X");
				goto intex;
			};
                        dd->d_state.s_substate = SUBSTATE_EXECUTE;
/*  set up TVI controls if READ */
                        if(TVI && (bp->b_flags & B_READ))
			{
				outb(CONTROL,CPUC|IRQA|CBRDY|RGATE);
				for (i = 10; i ; i--) ;/*wait a bit****/
				outb(CONTROL,CBRDY|RGATE);
			};
        		busywait(1,dd);
			outb(TASKF+2,dd->d_iopb.i_xfrcnt); /* sector count */
			funk = dd->d_iopb.i_funct ;
			if (TVI)
			{
                        	if(!(bp->b_flags & B_READ)) 
				{
					outb(CONTROL,CPUC|IRQA|WGATE);
					for (i = 10; i ; i--) ;/*wait a bit****/
					outb(CONTROL,WGATE);
                               		hdwdma(dd->d_iopb.i_addr);
			       		outb(CONTROL,CBRDY|WGATE);
                               		dd->d_iopb.i_addr += (dr->dr_secsiz);
					if (db2==1)
						printf("W: bp: %lx fg: %x\n",bp,bp->b_flags);
					funk |= 0x04;	
				} else
				{
					if (db2<0) printf("R");
					funk |= 0x0c;	
				};
			};  /* end if TVI */
			outb(STATUS, lobyte(funk));
			if (!TVI)
			{
                        	if(!(bp->b_flags & B_READ)) 
				{
					while(!(inb(STATUS) & WD_ST_DRQ)) 
						{ ; } /* wait for DRQ */
                               		hdwdma(dd->d_iopb.i_addr);
                               		dd->d_iopb.i_addr += (dr->dr_secsiz);
                        	};
			}; /* end if !TVI */
intex:
#ifndef	PICFIX1
			asm("	cli");	/* prevent reentry */
#endif /* ! PICFIX1 */ 
                        splx( x );
                        return;

                } else {
/*  A read or write operation generated an interrupt. 
We may be done or in the middle of a multi sector operation.
See if theres more to do, */
              		if( TVI && (dd->d_iopb.i_xfrcnt == 0) )  
				goto tryeoc;

                        if(!(bp->b_flags & B_READ)) 
			{
				if (TVI)
				{
					outb(CONTROL,IRQA|WGATE);
					for (i = 10; i ; i--) ;/*wait a bit****/
					outb(CONTROL,WGATE);
				};
			} else {
				if (TVI)
				{
					outb(CONTROL,IRQA|RGATE);
					for (i = 10; i ; i--) ;/*wait a bit****/
					outb(CONTROL,RGATE);
				};

                                hdrdma(dd->d_iopb.i_addr);
			  	dd->d_iopb.i_addr += dr->dr_secsiz;
				if (TVI)
				{
					outb(CONTROL,IRQA|RGATE);
					outb(CONTROL,CBRDY|RGATE);
					for (i = 10; i ; i--) ;/*wait a bit****/
				};
			};

              		if(--(dd->d_iopb.i_xfrcnt) > 0) 
			{
			    bumpit(dd);
                            if(!(bp->b_flags & B_READ)) 
				{
                                	hdwdma(dd->d_iopb.i_addr);
			  		dd->d_iopb.i_addr += dr->dr_secsiz;
					if (TVI)
					{
						outb(CONTROL,CBRDY|WGATE);
						for (i = 10; i ; i--) ;/*wait a bit****/
					};
				};
			    goto execex ;
			}
			else
			{
				if (TVI)
				{
tryeoc:
					if (eoc ||(!(inb(STATUS)&0x2)))
						goto cntck;
#ifndef	PICFIX1
					asm("	cli");	/* prevent reentry */
#endif /* ! PICFIX1 */ 
					splx(x); /* await EOC */
					return;
				};
			};
cntck:
#ifdef MP386
		/* Read/write completed - check for spares */
			if (dd->d_state.s_flags[dd->d_iopb.i_unit]
				 & SF_VTOC)
				 do_spare_sex(bp->b_flags & B_READ,dd);
#endif

            	          if (bp->b_flags & B_READ) 
			      {
			      if (in_cache(dd)) goto miodone;
			      };
                          if ((!(bp->b_flags & B_READ)) 
				&& (dd->d_iopb.i_cntfill == 0))
                               	goto miodone; 	/*screw Wirth, too !!!*/
			  /* else start next track */
			  
			
               if (!(bp->b_flags & B_READ)) 
			{
				dd->d_iopb.i_xfrcnt=dd->d_iopb.i_cntfill;
				bumpit(dd);
			};
			hdcmd(dd); /* set up for next track */
               if (!(bp->b_flags & B_READ)) 
			{
				dd->d_iopb.i_actual = dd->d_iopb.i_xfrcnt;
				dd->d_iopb.i_usect = dd->d_iopb.i_sector;
               	dd->d_iopb.i_uaddr = dd->d_iopb.i_addr;
			};
			outb(TASKF+2,dd->d_iopb.i_xfrcnt);

                        if( TVI && (!(bp->b_flags & B_READ)))
			{
                               hdwdma(dd->d_iopb.i_addr);
			       outb(CONTROL,CBRDY|WGATE);
                               dd->d_iopb.i_addr += (dr->dr_secsiz);
			};
			outb(STATUS,lobyte(funk));

                        if((!(bp->b_flags & B_READ)) && (!TVI)) 
			{
				while(!(inb(STATUS) & WD_ST_DRQ)) 
					{ ; } /* wait for DRQ */
                               hdwdma(dd->d_iopb.i_addr);
                               dd->d_iopb.i_addr += (dr->dr_secsiz);
	                };


/* we get here from normal and new-track code */
execex:
#ifndef	PICFIX1
		asm("	cli");	/* prevent reentry */
#endif /* ! PICFIX1 */ 
		splx(x);
		return;
        }; /* end SUBSTATE EXEC */
                /*
                 * reading label used for nonfs accesses.
                 */
                 case READING_LABEL:
			instatus++;
#ifndef	PICFIX1
			asm("	cli");	/* prevent reentry */
#endif /* ! PICFIX1 */ 
                        splx( x );
                        return;

		/* Recalibrating. */

		case RESTORING:
			splx(x);
			return;
                /*
                 * step to the next unit and start initializing it.
                 * dont initialize invalid units.
                 * restart io after initializing last unit.
                 */
                 case INITIALIZING:
			if (TVI)
			{
				outb(CONTROL,IRQA);
				for (i=10;i;i--);
				outb(CONTROL,0);
			};
                        dd->d_state.s_init[dd->d_state.s_opunit++] = status;
                        if(dd->d_state.s_devcode[dd->d_state.s_opunit]==INVALID)
                                 dd->d_state.s_opunit += NEXT_REMOVE_UNIT;
 
                        if( dd->d_state.s_opunit < NUMSPINDLE ) {
                             hdio(dd, (char *)0, INIT_OP,dd->d_state.s_opunit);
#ifndef	PICFIX1
				 asm("	cli");	/* prevent reentry */
#endif /* ! PICFIX1 */ 
                                 splx( x );
                                 return;
                         } else {
                                 /*
                                  * all are inited, reset flags
                                  * and wake up init'er
                                  */
                                 dd->d_state.s_state = NOTHING;
                                 dd->d_state.s_active &= ~IO_BUSY;
                                 hdstart( dd );
#ifndef	PICFIX1
				 asm("	cli");	/* prevent reentry */
#endif /* ! PICFIX1 */ 
                                 splx( x );
                                 return;
                         }
                }


BUG("FELL THRU SWITCH");
mioabort:
miodone:
BUG("MIODONE");
lwdb=0;
	dodone(dd);
#ifndef	PICFIX1
	asm("	cli");	/* prevent reentry */
#endif /* ! PICFIX1 */ 

        /*
         * I/O Done.  Complete it & start next.
         */
        splx( x );
}
        
bumpit(dd)
        register struct i1010dev *dd;
{
        register struct i1010drtab *dr;
        dr = dd->d_drtab[dd->d_state.s_opunit];
        dd->d_iopb.i_sector++;
        if(dd->d_iopb.i_sector > dr->dr_nsec) {
                dd->d_iopb.i_sector = 1;
                dd->d_iopb.i_head++;
        if(dd->d_iopb.i_head > (dr->dr_nfhead)-1) {
                 dd->d_iopb.i_head = 0;
                 dd->d_iopb.i_cylinder++;
                          } 
                         } 
}

dodone(dd)
        register struct i1010dev *dd;
{
	int x;
        register struct buf     *bp, *ap;
        struct iobuf    *bufh;
        bufh = dd->d_state.s_bufh;
        bp = dd->d_curbuf;
        bufh->b_active = 0;

        /*
        ** get next buffer while checking direction
        */
	x= splbio();
        if ( dd->d_state.s_active & IO_BACK ) {
                ap = bp->av_back;
                if ( ap == (struct buf *)dd->d_state.s_bufh ) {
                        ap = bp->av_forw;
                        dd->d_state.s_active &= ~IO_BACK;
                }
        } else {
                ap = bp->av_forw;
                if ( ap == (struct buf *)dd->d_state.s_bufh ) {
                        ap = bp->av_back;
                        dd->d_state.s_active |= IO_BACK;
                }
        }
	if (db2>1) printf("APZ: %lx\n",ap);
        	dd->d_curbuf = ap;
        bp->av_back->av_forw = bp->av_forw;
        bp->av_forw->av_back = bp->av_back;
        dd->d_state.s_active &= ~IO_BUSY;
        iodone(bp);
	 if (db2==1)
	 printf("D");
	hdstart(dd);	/* start any queued op */
	splx(x);
}
/* wait for busy to go away */
busywait(k,dd)
int k;
register struct i1010dev *dd;
{
	long timer;
	timer = 500000;
	while (timer)
	{
		if ((inb(STATUS) & WD_ST_BSY) == 0) return 0;
		timer--;
	}
	printf("BUSY TIMEOUT %d: STATUS : %x\n",k,inb(STATUS));
	return 1;
}
/*
 * hdread
 *      "Raw" read.  Use physio().
 */
hdread(dev)
dev_t   dev;
{ 
#ifdef MP386
	int hdbreakup();
#endif
        register struct i1010drtab *dr;
        register struct i1010slice *p;
BUG("hdread");
	if (minor(dev) == 255) dev = 11;	/* special kludge for boot track */
        dr = &i1010drtab[ DRTAB( dev ) ];
        p = &dr->dr_slice[SLICE(dev)];
#ifdef MP386
	physio(hdbreakup,&i1010rbuf[BOARD(dev)],dev,B_READ);
#else
        if (physck(((p->p_nsec * dr->dr_secsiz ) >> LBSHIFT), B_READ))
                physio(hdstrategy, &i1010rbuf[BOARD(dev)], dev, B_READ);
#endif
}

/*
 * hdwrite
 *      "Raw" write.  Use physio().
 */
hdwrite(dev)
dev_t   dev;
{
        register struct i1010drtab *dr;
#ifdef MP386
	int hdbreakup();
#endif
        register struct i1010slice *p;
BUG("hdwrite");
	if (minor(dev) == 255) dev = 11;	/* special kludge for boot track */
        dr = &i1010drtab[ DRTAB( dev ) ];
        p = &dr->dr_slice[SLICE(dev)];
#ifdef MP386
	physio(hdbreakup,&i1010rbuf[BOARD(dev)],dev,B_WRITE);
#else
        if (physck(((p->p_nsec * dr->dr_secsiz ) >> LBSHIFT), B_WRITE))
                physio(hdstrategy, &i1010rbuf[BOARD(dev)],dev, B_WRITE);
#endif
}

#ifdef MP386
hdbreakup(bp)
register struct buf *bp;
{
	dma_breakup(hdstrategy,bp);
}
#endif
/*
 * hdioctl
 *
 * I1010_FLOPPY command:
 * Returns the # of 512 byte blocks on the floppy device if called for a floppy
 * device, or zero if called for a winchester device.
 *
 * I1010_CHAR command:
 * Returns device characteristics as specified in the corresponding i1010drtab
 * structure into the structure pointed at by "cmdarg".  The dr_slice field is
 * not copied to user space.  (This avoids different model problems.)
 *
 * I1010_NTRACK command:
 * Returns # of tracks on device into the daddr_t pointed to by "cmdarg".
 *
 * I1010_IOC_FMT command:
 * Waits for and uses the "raw" buffer for the board, and the d_format field
 * in i1010dev.  Use of the "d_format" field is mutexed by use of the "raw"
 * buffer.
 *
 * The format ioctl allows the caller to specify all necessary data to assign
 * alternate tracks.  This code has only the intelligence to format the track;
 * no testing/etc of the track is done.
 *
 * I1010_RAWIO command:
 * ARG points to IOPB initialized with function(read or write),unit,
 * actual cylinder, actual head, sector and buffer. Reads or writes
 * entire sector. Buffer pointer must be 4-byte pointer.
 *
 * i100_SETDB sets the debug control variable to the value of *cmdarg
 * to enable or disable debug printouts.
 *
 * i1010_REINIT resets the firsttime switch so a subsequent hdopen
 * call will reload the minor device, slice, drive, and bad-track
 * tables from the disk. This is used in hard-disk installation
 * to avoid reboots.
 *
 * Assumes access to "raw" device is restricted via file-system
 * to knowledgeable programs/users.
 */
hdioctl(dev, cmd, cmdarg, flag)
dev_t   dev;
int     cmd;
struct  i1010ftk *cmdarg;
int     *flag;
{
#ifdef MP386
	struct disk_parms *dp;
	union io_arg *ia;
#endif
        register struct buf *bp;
        register struct iobuf *bufh;
        register struct i1010dev *dd;
        int      i;
	long timer;
        unsigned board;
        unsigned x,op;
        struct i1010drtab *drtabp;
        char *p;
        struct i1010ftk  ftk;
	register struct i1010iopb *io;
	/* union to reference parts of virtual address */
union
	{
	struct
		{
		ushort off;	/*offset*/
		ushort seg;	/* segment */
		} sep;
	char *full;
	} laddr;

unsigned stater;

BUG("hdioctl");
        board = BOARD(dev);
        dd = i1010bdd[board];
        bp = &i1010rbuf[board];
        drtabp = &i1010drtab[DRTAB(dev)];
#ifdef MP386
/* This code interprets ISC ioctls for VTOC-based systems */
	if (cmd == V_GETPARMS)
	{
		dp = (struct disk_parms *)cmdarg;
		dp->dp_type = DPT_WINI;
		dp->dp_heads = drtabp->dr_nfhead;
		dp->dp_cyls = drtabp->dr_ncyl;
		dp->dp_sectors = drtabp->dr_nsec;
		dp->dp_secsiz = drtabp->dr_secsiz;
		return;
	};
	if (cmd == V_CONFIG)
	{
		BUGG(" IOCTL V_CONFIG\n");
		ia = (union io_arg *)cmdarg;
		if (ia->ia_cd.secsiz != drtabp->dr_secsiz)
		{
			u.u_error = EINVAL;
			return;
		};
		drtabp->dr_nsec = ia->ia_cd.nsec ;
		drtabp->dr_ncyl = ia->ia_cd.ncyl ;
		drtabp->dr_nfhead = ia->ia_cd.nhead ;
		drtabp->dr_spc =ia->ia_cd.nhead * ia->ia_cd.nsec ;
		
printf("s: %d c: %d h:%d spc: %d\n", drtabp->dr_nsec, drtabp->dr_ncyl , drtabp->dr_nfhead, drtabp->dr_spc );
		return;
	};
	if (cmd == V_ADDBAD)
	{
		struct alt_table *at;
		struct alt_ent *ae;
		at = (struct alt_table *)&i1010bad_track[dd->d_iopb.i_unit];
		if (at->alts_used == at->alts_present)
		{
			u.u_error = ENOSPC;
			return;
		};
		for (i = 0 ; i < at->alts_present ; i++)
		{
		/* point to entry */
			ae = (struct alt_ent *)&(at->alts[i]);
			if (ae->goodsec == ia->ia_abs.bad_sector)
			{
				if (i < at->alts_used)
				{
					u.u_error = EINVAL;
					return;
				}
				else
				/**  unassigned alternate is bad - remove **/
					ae->goodsec = -1 ;

			};
		};
		/* point to first unused entry */
		ae = (struct alt_ent *)&(at->alts[at->alts_used ]);
		if (ae->goodsec == -1)
		{
			ae++;	/* skip any bad alternates */
			if (++(at->alts_used) == at->alts_present)
			{
				u.u_error = ENOSPC;
				return;
			};
		};
		ia = (union io_arg *)cmdarg;
		ae->badsec = ia->ia_abs.bad_sector;
		ia->ia_abs.new_sector = ae->goodsec;
		at->alts_used++;
		return;
	};
	if (cmd == V_FORMAT)
	{
		int cy,tr;
		ia = (union io_arg *)cmdarg;
		for (i = ia->ia_fmt.num_trks; i ;i--)
		{
			cy = ia->ia_fmt.start_trk / drtabp->dr_nfhead;
			tr = ia->ia_fmt.start_trk++ % drtabp->dr_nfhead;
			hdformat(dev,dd,bp,cy,tr,ia->ia_fmt.intlv);
		};
		return;
	};

#endif
	if (cmd == I1010_SETDB)
	{
		if (cmdarg)	/* zero is CTEST */
			lwdb = *((int *)cmdarg);
			else reset(dd); 
		return;
	};

	if (cmd == I1010_REINIT 
#ifdef MP386
				|| cmd == V_REMOUNT /*REMOUNT is 386 only */
#endif
						)
	{
		if (opencount <= 1)
		{
			firsttime = 1;
		}
		else
		{
			u.u_error = EBUSY;
		};
		return;
	};

       if (cmd == I1010_RAWIO)
       {
BUG("RAWIO");
		tsbusy(dd);
		stater = dd->d_state.s_state;
		dd->d_state.s_state = READING_LABEL; /* throw away interrupt*/
       		io = (struct i1010iopb *) cmdarg; /* convert ptr */
		dd->d_iopb.i_unit = UNIT(dev);
		dd->d_iopb.i_sector = io->i_sector;
       		dd->d_iopb.i_actcylinder = io->i_actcylinder;
       		dd->d_iopb.i_acthead = io->i_acthead;
		dd->d_iopb.i_xfrcnt = 1;
		dd->d_iopb.i_funct = io->i_funct;

		if (io->i_funct == WD_FORMAT_OP)
		{
			drtabp->dr_nfhead = io->i_acthead;
			drtabp->dr_nsec = io->i_sector;
			drtabp->dr_precomp = io->i_actcylinder;
		};
if (lwdb >0)
printf("\nRAWIO cy: %d hd: %d sec: %d unit: %d fnc: %x addr: %lx\n",
	io->i_actcylinder,io->i_acthead,io->i_sector,dd->d_iopb.i_unit,io->i_funct,io->i_addr);
		if (!TVI)
		{
			if (polled_io(dd)) goto iocerr;

       			if ( io->i_funct == WD_WRITE_OP)
				hdwdma(physaddr(io->i_addr)); /* fill buffer if write */
			if ( io->i_funct == WD_READ_OP)
		     		hdrdma(physaddr(io->i_addr)); /* mt buffr if read */
		}
		else
		{
                 	x = (dd->d_iopb.i_funct == WD_READ_OP) ? CBRDY|RGATE : WGATE;
       			if ( (dd->d_iopb.i_funct  == WD_READ_OP)
       				||  (dd->d_iopb.i_funct  == WD_WRITE_OP))
			{
				outb(CONTROL,CPUC|IRQA|x);
				for (i = 10; i ; i--) ;/*wait a bit****/
				outb(CONTROL,x);
       				if ( dd->d_iopb.i_funct == WD_WRITE_OP)
				{
					hdwdma(physaddr(io->i_addr)); /* fill buffer if write */
					outb(CONTROL,CBRDY|WGATE);
				}
				if (polled_io(dd)) goto iocerr;

				x &= ~CBRDY;
				outb(CONTROL,IRQA|x);
				for (i = 10; i ; i--) ;/*wait a bit****/
				outb(CONTROL,x);
		
		
				if ( dd->d_iopb.i_funct == WD_READ_OP)
				{
		     			hdrdma(physaddr(io->i_addr));
					outb(CONTROL,CBRDY|x);
				};
			} else {
				if (polled_io(dd)) goto iocerr;
			};

		}; /* end if !TVI else */
		busywait(2,dd);
            	if (!(inb(STATUS) & WD_ST_BSY) && (inb(STATUS) & WD_ST_ERROR)) 
		{
			printf("\nWN I/O Error: estat: %x  drstat %x stat: %x \n",
					 inb(TASKF+1),inb(TASKF+6),inb(STATUS));
			printf(" cyl: %d hd: %d sec: %d unit: %d fnc: %x addr: %lx\n",
	io->i_actcylinder,io->i_acthead,io->i_sector,dd->d_iopb.i_unit,io->i_funct,io->i_addr);
iocerr:
			u.u_error = EFAULT;
		};
		dd->d_state.s_state = stater; 
		dd->d_state.s_active &= ~IO_BUSY; 
		return;
       	};
  
       if (cmd == I1010_CHAR) {
BUG("CHAR");
                if (copyout(drtabp, cmdarg,
                        (char *)&(drtabp->dr_slice) - (char *)drtabp))
                        u.u_error = EFAULT;
                return;
        }

        if (cmd == I1010_FLOPPY) {

                u.u_rval1 = 0;
                return;
       
        }
 
        if (cmd == I1010_NTRACK) {
                
                /* Calculate # of tracks on section. */
                struct i1010slice        *slicep;        /* ptr to slice info */
                daddr_t                 t;              /* # of tracks */

BUG("NTRACK");
                slicep = drtabp->dr_slice + SLICE(dev);
                t = slicep->p_nsec / drtabp->dr_nsec;
		if (copyout(&t, cmdarg, sizeof(t)))
			u.u_error = EFAULT;
                return;
        }
        if (cmd != I1010_IOC_FMT) {
BUG("IOCTL BAD CMD");
                u.u_error = ENXIO;
                return;
        }

        /* Format a track. */
BUG("IOC-FMT");

         if(copyin(cmdarg, &ftk, sizeof(struct i1010ftk))) {
                 u.u_error = EFAULT;
                 return;
         }

	hdformat(dev,dd,bp,ftk.f_cyl,ftk.f_track,ftk.f_intl);
	return;
}
/****************************************************************************/

hdformat(dev,dd,bp,cyl,track,interlv)
	dev_t dev;
        register struct buf *bp;
        register struct i1010dev *dd;
	int cyl,track,interlv;
{

	int i,x,stater,timer;
        /* * Wait for buffer.  */

        x = splbio();
        while(bp->b_flags&B_BUSY) {
                bp->b_flags |= B_WANTED;
                sleep((caddr_t)bp, PRIBIO+1);
        }
        bp->b_flags = B_BUSY | B_FORMAT;
        bp->b_dev = dev;
        splx(x);
		tsbusy(dd);
		stater = dd->d_state.s_state;
		dd->d_state.s_state = READING_LABEL; /* throw away interrupt*/
		dd->d_iopb.i_unit = UNIT(dev);
		dd->d_iopb.i_sector = 15; /* gap 3 */
		dd->d_iopb.i_xfrcnt = 17;
       		dd->d_iopb.i_actcylinder = cyl;
       		dd->d_iopb.i_acthead = track;
       		dd->d_iopb.i_funct = WD_FORMAT_OP;
if(lwdb)
printf("\nFORMAT cy: %d hd: %d sec: %d unit: %d fnc: %x addr: %lx\n",
	dd->d_iopb.i_actcylinder,dd->d_iopb.i_acthead,dd->d_iopb.i_sector,dd->d_iopb.i_unit,dd->d_iopb.i_funct,dd->d_iopb.i_addr);
		if (dd->d_iopb.i_unit != ((inb(TASKF+6) & 0x10 )>>4))
			{ 
			recalibrate(dd,dd->d_iopb.i_unit);
			};

		table(interlv,&bblock[0]);
                  
		if (TVI)
		{
			outb(CONTROL,CPUC|IRQA|WGATE);
			for (i = 10; i ; i--) ;/*wait a bit****/
			outb(CONTROL,WGATE);
			hdwdma(physaddr(&bblock[0])); /* fill buffer */
			outb(CONTROL,CBRDY|WGATE);
		};
		polled_io(dd);

		if (!TVI)
			hdwdma(physaddr(&bblock[0])); /* fill buffer */

		timer = lbolt;
        	while(inb(STATUS) & WD_ST_BSY) {
          		if (lbolt - timer > 1000)
			{
				printf("FORMAT: BUSY TIMEDOUT\007\n");
				printf("STATUS: %x\n",inb(STATUS));
				reset(dd);
				return(1)      ;
			};
        	}
		if (TVI)
		{
			outb(CONTROL,IRQA|WGATE);
			for (i = 10; i ; i--) ;/*wait a bit****/
			outb(CONTROL,WGATE);
		};
        	bp->b_flags &= ~(B_BUSY|B_FORMAT);
		dd->d_state.s_state = stater; 
		dd->d_state.s_active &= ~IO_BUSY; 
		return;
}

/****************************************************************************/
/* generate a format pattern from interleave */
#define SPT 17
table (v,p)
int v;
unsigned char *p;
{
	int k,i,j;
	k = 0;
	j= SPT/v;
	for (i= SPT; i ; i--)
	{
		*p++=0;
		*p++ = ++k ;
		k += j ;
		k %= SPT;
	};
}
/*
	Dummy because its called from kernel
 */
hddump()
{

}

/*
 * generic print routine. called by the kernel.
 */
hdprint( dev, string )
int dev;
char *string;
{
}
