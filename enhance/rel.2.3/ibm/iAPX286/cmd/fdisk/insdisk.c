static char *uportid = "@(#)insdisk.c	Microport Rev Id  1.3.8C 12/2/86";

/****************************************************************************/
/*      insdisk  installs a fixed disk and sets up the required
 *      auto-configuration tables on the hard disk.
 *
 *      For each hard disk, an attempt is made to obtain the drive
 *      type from the CMOS RAM. The user is then asked to confirm the
 *	drive type if found. If an illegal type is found, or the
 *	user doesn't confirm, the user is queried for drive parameters.
 *
 *      The bad track table is then generated from user input.
 *
 *  NOTE:   insdisk assumes that fdisk has been successfully run to generate
 *	a partition table prior to calling insdisk.
 *
 *      Larry Weaver  Jan. 86
 *
 *  Modification History:
 *  M002:	uport!dwight Thu Apr 10 13:24:32 PST 1986
 *	Added return(0) to certain functions. Note that this version
 *	contains return dependancies based on "return;" statements!
 *	This needs to be reworked for explicit parameter return values.
 *  M003:   uport!bernie Thu Nov 20 16:39:31 PST 1986
 *      Added end of bad track list signal for use of easyinstall.
 *	Added prompting of user for disk parameters each time fdisk is 
 *	executed.
 *	Improved tolerance of unexpected responses to getno() and query().
 *  M004:	uport!rex	Wed Jun 17 02:36:34 PDT 1987
 *	Fixed next alternate track selection which previously was using
 *	duplicate alternate tracks.
 *  M005:	uport!rex	Tue Jul 14 17:43:36 PDT 1987
 *	Fix to read in master boot block code if Magic 0xAA55 not found.
 *
 */
#define	M004		/* for M004 fixes to be compiled in */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <malloc.h>
#include "sys/misc.h"
#include "sys/wn.h"
#include "sys/wndefaults.h"

#define DRVTYP 0x12	/* fixed disk type byte offset in CMOS RAM */
#define FILE path[unit]
extern char *path[];
/* this code assumes that the drive table for this drive begins at offset
   0 of the last sector in the partition, and that the 
       slice table starts on the next word boundary following the
   fixed-length drtab. Thus only the stlboot and bad-block pointers in
   the table beginning at offset 0x1bf are not redundant.
   
*/

#define PTOFFSET        0x1be   /* partition table offset */
#define SIGOFFSET       0x1fe   /* table signature offset */

#define DRTOFFSET 0     /* drive table offset */
#define SLTOFFSET DRTOFFSET+sizeof(struct i1010drtab)
#define BADOFFSET 0x1f6
#define BADSIZE 48
#define ALT_CYLS 6


byte *moveb ();

struct partit {
        byte boot_ind;
        byte bhead;
        byte bsect;
        byte bcyl;
        byte syst_ind;
      	byte ehead;
        byte esect;
        byte ecyl;
        long  rel_sect;
        long no_sects;
};
struct diskaddr
{
	byte head;
	byte sector;
	byte cyl;
};
/* global variables */
int     bps = 512;    /* bytes / sector */
extern int spt,spc, tpc,ncyl, unit, changes, reboot, otto;
extern unsigned char tabbuf[], ptab[];
dev_t fd;
unsigned int begincyl,endcyl;

int *sig;
byte *er,*bt,*bad;
unsigned int newer, altcyl,althead,badcyl, badtrk;
struct bad_track_map *bd, *bdt, *bdlimit;
struct partit *pt;
struct diskaddr *da;
struct i1010drtab drvtab;

insdisk()
{
#ifdef M004
	struct bad_track_map *tbt;
#endif

	if (ckactpar())
	{
		printf("No active System5 partition!\n");
		printf(" Partition table must be initialized before bad-track table!\n");
		return(1);
	}
	initbad();   /*initialize buffer*/

	endcyl = pt->ecyl | (((int)(pt->esect & 0xC0)) << 2);
	begincyl = pt->bcyl | (((int)(pt->bsect & 0xC0)) << 2);
	
        /*      Compute the first alternate cylinder    */
	altcyl = (endcyl - (ALTTRACKS/tpc)) + 1;
	althead = 0;

	newer = query("Is a complete new bad track table to be written");
        if (newer)      /* initialize tables */
        {
                for (   ; bd < bdlimit ; bd++)
                        bd->bad_cylinder = 0xFFFF ;
        }
        else
        {
	/* this code assumes that bad track table is always on
	  next-to last sector in partition */
 		read_sector(fd,bad,endcyl,pt->ehead,(pt->esect&0x1f)-1);

                /* find the first alternate cylinder left */
                for (tbt = bd = bdt; bd < bdlimit; bd++)
		{
#ifdef M004
			if (bd->new_cylinder > altcyl)
			{
				tbt = bd;
				altcyl = bd->new_cylinder;
				althead = bd->new_track;
			}
			else if (bd->new_cylinder == altcyl &&
				 bd->new_track > althead)
			{
				tbt = bd;
				althead = bd->new_track;
			}
#endif
                        if (bd->bad_cylinder == 0xFFFF)
                        {
                                bd--;
                                break;
                        };
		}
      if (bd >= bdlimit) BOMB ("Bad Track Table Full");
#ifdef M004
		bd = tbt;	/* someone else may be looking at 'bd'	*/
#else
                altcyl = bd->new_cylinder;
                althead = bd->new_track + 1;
#endif
                if (++althead >= tpc)                             
                {
                        altcyl++;
                        althead = 0;
                };
                if ((altcyl == endcyl) && ( althead == tpc-1))
			 BOMB ("No alternates left");
        };
/*
 *
 *  Get bad track entries from user 
 *
 */

	if (query("Do you wish to scan the disk for bad tracks")) /* M000 */
 		scan_disk();					  /* M000 */
 	if (query("Do you wish to type in bad tracks")) {	  /* M000 */
		printf("\nEnter bad tracks, terminate with CTRL-D\n");
		while (1)
		{
	     		/* get pointers to bad track */
	        	if (!get_bad_track (&badcyl,&badtrk) ) break;  
			if (add_bad_trk(badcyl, badtrk)) return(1);		/* M000 */
	
	
		}
	}							/* M000 */
	ckbadalts(); /* check for bad alternate trax */
        printf("Bad Track Update Complete\n");
	changes++;			/* must now reboot	*/
	return(0);
}

/****************************************************************************/


/*  Initialize unit (drive) */

init_unit()
{
#ifdef BOOTDRVTBL			/* M003 */
	int	set_params = 0;
#endif BOOTDRVTBL			/* M003 */
	if ((fd = open(FILE, O_RDWR)) == -1) {
		fprintf(stderr, "Can't open %s, errno=%d\n", FILE, errno);
		exit(errno);
	}

	read(fd,tabbuf,512) ;	/* dummy read to force wnsweep */

	if (read_sector(fd,tabbuf,0,0,1 )) {
		fprintf(stderr, "Can't read %s, errno=%d\n", FILE, errno);
		exit(errno);
	}
	if ( *((int *) &tabbuf[0x1FE]) != 0xAA55 )	/* M005 */
		++otto;			/* so ckactpar() will read in code */
	copytbuf();  /* copy partition table from buffer */
	if ( get_drive_params(&drvtab))	/* get drive table */ {
		changes++;
#ifdef BOOTDRVTBL			/* M003 */
		if (! hd_root())
			set_params++;
#endif BOOTDRVTBL			/* M003 */
	}
	spt  = drvtab.dr_nsec;
	tpc  = drvtab.dr_nfhead;
	ncyl = drvtab.dr_ncyl;
	spc  = spt * tpc;
#ifdef BOOTDRVTBL			/* M003 */
	if (set_params) {
		set_params = drvtab.dr_precomp;
		raw_io(fd, (char *) 0, set_params, tpc, spt, WD_SET_PARM_OP);
		raw_io(fd, (char *) 0, 0, 0, 0, WD_RECAL_OP);
	}
#endif BOOTDRVTBL			/* M003 */
        bad = (byte *) malloc(bps);

        bd = bdt = (struct bad_track_map *) ( bad );
        bdlimit = bdt + (bps /sizeof (struct bad_track_map )) ;
        er = (byte *) malloc(bps);
	/* unkludge cylinder */
	if(!ckactpar())
	{
		read_PER(); /* read PER if an active SYS5 part*/
		endcyl = pt->ecyl | (((int)(pt->esect & 0xC0)) << 2);
		if (BTvalid()) read_BT();
		else {
			int	i = 0;
			byte	*p = er;
			while (i++ < bps)	/* make sure PER is clean */
				*p++ = '\0';
			initbad();
		}
	};
}


/****************************************************************************/
BTvalid()
{
        sig  = (int *)(er + SIGOFFSET);
	return (*sig == 0xAA55);	/*validate badtrack tables*/
}
/****************************************************************************/
validateBT()
{
        sig  = (int *)(er + SIGOFFSET);
	*sig = 0xAA55;	/*validate drive  and badtrack tables*/
}
/****************************************************************************/
/*
 *   Set up for badtrack mapping 
 */

initbad()
{
   /* initialize bad-track pointer and table buffer */
	da = (struct diskaddr *)(er + BADOFFSET);
	da->head = pt->ehead;
	da->cyl = endcyl & 0xFF;
	da->sector = ((endcyl >> 2) & 0xC0) | ((spt-1) & 0x1f);
        bad = (byte *) malloc(bps);

        bd = bdt = (struct bad_track_map *) ( bad );
        bdlimit = bdt + (bps /sizeof (struct bad_track_map )) ;
        for (   ; bd < bdlimit ; bd++)
                        bd->bad_cylinder = 0xFFFF ;
	validateBT();

return(0);
}

/****************************************************************************/
ckactpar()
{
	int i,j,valid;
	struct partit *save;
	
	bt = &ptab[0];	/* partition table pointer */
        pt = (struct partit *) (bt); /* partition table pointer */
	valid = 0;
	if (otto)	/* if initialize switch */
	{
		init_mbblk(); /* initialize master boot block */
		otto = 0;
		return(1);
	};
        for (i= 1; i<=4 ; i++)
	{
		if ( pt->ehead > tpc 
			|| (pt->ecyl | ((pt->esect & 0xC0)<< 2)) > ncyl)
		{
			printf("\n\007Invalid partition table! Clearing !\n\007");
			init_mbblk();	/* initialize master boot block */
			return(1);
		};
                if (pt++->boot_ind == 0x80)
		{
			valid = 1;
			save = pt;
		}
	}

        if (!valid ) return(1);			/* no active partition yet */
	else	     pt = --save;		/* backup to active partition */
	if (pt->syst_ind != 5 &&
	    pt->syst_ind != 0x52) return(1); 	/* active part not System5 */
	endcyl = pt->ecyl | (((int)(pt->esect & 0xC0)) << 2);
	return(0);
}
/****************************************************************************/
init_mbblk() /* initialize master boot block */
{
	int	j, fd;
	if ((fd = open("/etc/master.bblock", O_RDWR)) == -1) {
		fprintf(stderr, "Can't open %s, errno=%d\n", FILE, errno);
		exit(errno);
	}

	read(fd,tabbuf,512) ;
#ifndef BOOTDRVTBL			/* M005 */
	getdp(tabbuf+  DISKPARMS, &drvtab);
#endif
	copytbuf();  /* copy partition table from buffer */
	bt = &ptab[0];	/* partition table pointer */
	for ( j = 0 ; j < (sizeof(struct partit)*4)+2 ;j++) *bt++ = 0;
	changes++;
	bt = &ptab[0];	/* partition table pointer */
	close(fd);
}
/****************************************************************************/
/*      read end-of-partition record -if any - to get drtabs,
	 slice table, and pointer to first bad-track sector  */
read_PER()
{
        read_sector (fd,er,endcyl,pt->ehead,pt->esect&0x1f);
}

/****************************************************************************/
	/* read bad track table from disk */
read_BT()
{
        read_sector (fd,bad,endcyl,pt->ehead, (pt->esect&0x1f)-1);
}
/****************************************************************************/
/****************************************************************************/
	/* write updated bad track table back to disk */
write_BT()
{
	if (bad)
        write_sector (fd,bad,endcyl,pt->ehead, (pt->esect&0x1f)-1);
}
/****************************************************************************/
	/* write updated partition end record back to disk */
write_PER()
{
	if (er)
	{
		/* move drive table to buffer */
		moveb ((byte *) &drvtab, er ,sizeof(struct i1010drtab));
        	write_sector (fd,er,endcyl,pt->ehead,pt->esect & 0x1f );
	};
}

/****************************************************************************/
/*
        ripple badtrack tle up to leave room for new entry
*/
rippleup(bd,bdlimit)
struct bad_track_map *bd, *bdlimit;
{

        if ((--bdlimit)->bad_cylinder != 0xFFFF) BOMB("Bad Track Table Full");
        for ( ; bd < bdlimit; bdlimit--)
        {
                bdlimit->bad_cylinder = (bdlimit-1)->bad_cylinder;
                bdlimit->bad_track = (bdlimit-1)->bad_track;
                bdlimit->new_cylinder = (bdlimit-1)->new_cylinder;
                bdlimit->new_track = (bdlimit-1)->new_track;
        };
        

	return(0);				/* M002 */
}

#include "sys/cmos.h"

#include "../getype.c"

#ifdef BOOTDRVTBL			/* M003 */
/*
 *  so update the drive parameter entry in the
 *  active unix partition boot sector
 */
write_PBS()
{
	unsigned char  beginsec;
	unsigned int   begincyl;
	extern unsigned char partbootsec[];
	extern char bsdptupdate;
	extern int  dptdbg;

	if (bsdptupdate == 0) return;
	if (bsdptupdate == 2)			/* need the boot code */
	{
		int	bsfd;
		if ((bsfd=open("/etc/boot.hd", 0)) < 0)
		{
			fprintf(stderr,"Can't open /etc/boot.hd\n");
			fprintf(stderr,"Fixed disk System5 will not boot\n");
			return;
		}
		if (dptdbg)
			fprintf(stderr, "Reading /etc/boot.hd\n");
		read(bsfd, partbootsec, 512);
		close(bsfd);
		dptopbs(&partbootsec[BOOTDRTBLOFF], &drvtab);
	}

	begincyl = ((pt->bsect & 0xC0) << 2) | pt->bcyl;
	beginsec =   pt->bsect & 0x3F;
	if (dptdbg)
		fprintf(stderr,"Update BSDPT at C %u, H %d, S %d\n",
		    begincyl, pt->bhead & 0xff, pt->bsect & 0x1f);
	if (dptdbg < 2)
		write_sector(fd,partbootsec,begincyl,pt->bhead,pt->bsect&0x1f);
}
#endif BOOTDRVTBL			/* M003 */

/****************************************************************************/
/*   read bad track data from terminal or file  */
get_bad_track(cyl,trk)
int *cyl;
byte *trk;
{
	char string[20],*p, *gets();
        int i;

        do
      {
                printf("\nEnter bad track as: cylinder,track:  ");
                p = gets(string);
		if (p==NULL) return(0);
                i = sscanf (string,"%d,%d",cyl, trk);
		    /* M003 - end of list signal for easyinstall */
		if ( ( *cyl == -1 )  &&  ( *trk == -1 ) )  
		  return(0);
		if (i != 2) printf("Syntax Error\07\07");
        }
        while (i!=2);
        return(1);
}


add_bad_trk(badcyl, badtrk)				/* start M000	*/
int 	badcyl;
byte 	badtrk;
{

		for (bd = bdt ; bd < bdlimit; bd++)
		{
				if  ((bd->bad_cylinder == badcyl)
				 && (bd->bad_track == badtrk))
					return(0); /*ignore duplicates M002 */

				if ((bd->bad_cylinder > badcyl) 
				 || ((bd->bad_cylinder == badcyl)
				 && (bd->bad_track > badtrk))) {
					if (rippleup (bd,bdlimit))return(1);  /* make room  */
					bd->bad_cylinder = badcyl;
					bd->bad_track = badtrk;
					bd->new_cylinder = altcyl;
					bd->new_track = althead;
					break;
				};
		};

			if (++ althead >= tpc)                          
			{
				altcyl++;
				althead = 0;
			};
			if (altcyl > endcyl) BOMB ("No alternates left");
	return(0);					/* M002 */
}
int on=1, off= 0;
scan_disk()
{
	int	cyl, err;
	byte trk, sec, *buf; 

	buf = (byte *) malloc(bps);
	for(cyl = begincyl; cyl <= endcyl; cyl++)
		for(trk=0; trk < tpc; trk++) {
			printf("Scan cyl %3d trk %2d\r", cyl, trk);
			fflush(stdout);
			err = 0;
			for(sec = 1; sec <= spt; sec++) {
				err = read_sector(fd, buf, cyl, trk, sec); /* read it */
				if (err)
					break;
				}
			if (err) {
				printf("Scan cyl %3d trk %2d sec %d: added to the bad track table\n", 
					cyl, trk, sec);
				if (add_bad_trk(cyl, trk)) return(1);
				}
			}
	printf("\n");
}								/* End M000 */



ondb()
{
int *flag,f=1;
flag = &f;
ioctl(fd,I1010_SETDB,flag);
}
/* check badtrack table for bad alternates */
ckbadalts()
{
	struct bad_track_map *b, *a;
	int i;

	do
	{
	    i=0;
	    for (b=bdt ;( b< bdlimit) && ( b->bad_cylinder != 0xFFFF) ; b++)
		for (a=bdt ; (a< bdlimit) && (a->bad_cylinder != 0xFFFF) ; a++)
			while ((b->new_cylinder == a->bad_cylinder) &&
				(b->new_track == a->bad_track))
			{
				b->new_cylinder = a->new_cylinder;
				b->new_track = a->new_track;
				i++;
			};
	} while ( i );
}

