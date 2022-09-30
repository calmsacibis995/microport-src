static char *uportid = "@(#)showbad.c	Microport Rev Id  1.3.2 6/10/86";

/****************************************************************************/
/*
shobad prints the bad track table for the requested unit (default 0)
        Larry Weaver  April 86
*/

/****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <malloc.h>
#include "sys/misc.h"
#include "sys/wn.h"
#include "sys/cmos.h"
#define DRVTYP 0x12	/* fixed disk type byte offset in CMOS RAM */
#define FILE path[unit]
char *path[] = { "/dev/rdsk/0s0" , "/dev/rdsk/1s0" };

unsigned char tabbuf[512];
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



struct partition {
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
int spt, unit, changes, reboot;
dev_t fd;
unsigned int endcyl;

int i,*sig;
byte *er,*bt,*bad ;
unsigned int newer, altcyl,althead,badcyl, badtrk;
struct bad_track_map *bd, *bdt, *bdlimit;
struct partition *pt;
struct diskaddr *da;
struct i1010drtab drvtab;

main(argc,argv)
int argc; char *argv[];
{
	if (argc == 1) 
	{
		unit = 0;
		shobad() ;
	}
	 else
		for (i=1 ; i < argc ; i++)
		{
		 	unit = (atoi(argv[i]));
			shobad();
		};
}

shobad()
{

	initialize();
	printheader();
        for (bd = bdt; bd < bdlimit; bd++)
        	if (bd->bad_cylinder == 0xFFFF)
                         break;
		else
		{
			printbad(bd);
		};
}

/**********************************************************************/
printheader()
{
	printf("\t\tBad Track Table - Unit %d \n",unit);
	printf("    Bad Cylinder    Bad Head\t Alt. Cylinder      Alt. Head\n\n");
}
/**********************************************************************/

printbad()
{
	printf ("\t%d\t\t%d\t\t%d\t\t%d\n",bd->bad_cylinder,bd->bad_track,
			bd->new_cylinder,bd->new_track);
}

/**********************************************************************/

/*  Initialize unit (drive) */

initialize()
{
	if ((fd = open(FILE, O_RDWR)) == -1) {
		fprintf(stderr, "Can't open %s, errno=%d\n", FILE, errno);
		exit(errno);
	}

	if (read_sector(fd, tabbuf, 0, 0, 1) ) {
		fprintf(stderr, "Can't read %s, errno=%d\n", FILE, errno);
		exit(errno);
	}


	bt = &tabbuf[0];	/* boot block pointer */
        pt = (struct partition *) (bt+PTOFFSET); /* partition table pointer */
        for (i= 1; i<=4 ; i++)
{
                if (pt++->boot_ind == 0x80) break;
};

        if ( i>4 ) BOMB("No Active Partition") 
	else pt--;	/* backup to active partition */

        er = (byte *) malloc(bps);
        bad = (byte *) malloc(bps);
/*      read end-of-partition record -if any - to get drtabs,
	 slice table, and pointer to first bad-track sector  */

	/* unkludge cylinder */
		endcyl = pt->ecyl | (((int)(pt->esect & 0xC0)) << 2);
	/* end sector is always 1,use sectors/track to get last sector */
	/* this code assumes that bad track table is always on
	  next-to last sector in partition */
	/* partition endrecord -includes drvtab   */
	spt = pt->esect & 0x3f;
        	read_sector (fd,er,endcyl,pt->ehead,spt);
	/* bad-track-table  */
 		read_sector(fd,bad,endcyl,pt->ehead,spt-1);

        bd = bdt = (struct bad_track_map *) ( bad );
        bdlimit = bdt + (bps /sizeof (struct bad_track_map )) ;
        sig  = (int *)(er + SIGOFFSET);
/* if no PER signature, bomb off */
        if (*sig != 0xAA55)BOMB("No bad track table");
}

/****************************************************************************/
/* read a sector by disk address (cyl,head,sector)     */
read_sector(dev,buf,cyl,head,sector)
dev_t dev;
byte *buf,head,sector;
int cyl;
{

	int i;
	struct i1010iopb *io, iopb;
	io = &iopb;
	io->i_addr = (long) buf;
	io->i_actcylinder = cyl;
	io->i_acthead = head;
	io->i_sector = sector;
	io->i_funct = WD_READ_OP;
	i = ioctl(dev,I1010_RAWIO,io);
	if (i) printf("ioctl failed, error no: %d \n",errno);	
	return(i);
}
/**************************************************************************/
/* query user and return 1 if yes, 0 if n*/

query(string)
char *string;
{
	char tstr[20], *tp, *gets();				/* M000 */

	printf("\n%s:",string);
	while(1) {
        	tp = gets(tstr);
		if (tp==NULL) return(0);
		switch (*tp) {
        		case 'Y':
        		case 'y':
        			return(1);
        		case 'N':
        		case 'n':
        			return(0);
  		};
 	}
}

/****************************************************************************/
