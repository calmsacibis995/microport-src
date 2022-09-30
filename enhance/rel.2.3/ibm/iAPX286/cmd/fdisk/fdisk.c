static char *uportid = "@(#)fdisk.c	Microport Rev Id  2.3 5/28/87";

/*
 * fdisk.c:	Read and Write the Fixed Disk Partition Table.
 *		1/15/86 uport!dwight
 *
 *		Copyright 1986 by Microport Systems. All Rights Reserved.
 * Modification History:
 * M000		uport!dwight	Sun Mar 23 20:43:53 PST 1986
 * 	Added reboot switch, if changes have been made.
 * M001		uport!dwight	Wed Jun  4 15:41:25 PDT 1986
 *	Fixed dos-reverse order in changing the active partition.
 * M002		uport!dwight	Mon Jul  7 12:02:35 PDT 1986
 *	Prevented user from starting a partition on cylinder 0.
 * M003		uport!rex	Thu Jan 29 11:06:27 PST 1987
 *	Prevented user from ending a partition beyond the last cylinder.
 * M004   uport!bernie Mon Apr 13 12:39:16 PST 1987
 *	Improve io for menu routine so as to improve interface with getno()
 *	and query() in getype.c
 *	Correct prompting of menu routine when user enters invalid option.
 *	Corrected pointer error in displaying helpmenu.
 * M005   uport!bernie Mon Apr 20 13:14:44 PST 1987
 *	Allow user to look at help information before continuing.
 * M006		uport!rex	Tue Feb 24 1987
 *	Reboot when critical changes have been made.
 *	Ask for update if "-s" command line option was given.
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <malloc.h>
#include <sys/uadmin.h>
#include "sys/fdisk.h"
#include "sys/cmos.h"
#include "sys/wn.h"
#include "sys/io_op.h"			/* M006 */

#define	FILE	path[unit]		/* where partition table lives	*/
#define OUTFILE	FILE			/* where update gets written out*/
#define	BLKSIZ	512
#define	PARTOFF	0x1BE			/* start of the partition table */
#define MAXUNIT 1			/* maximum disk unit */
#define	VALID_PTAB	0xAA55		/* indicates vaild part. table	*/
#define	NOEXIT(s)	((*(s) != 'x') && (*(s) != 'X'))
#define	EXIT(s)		((*(s) == 'x') || (*(s) == 'X'))

static char *boottype = "U";		/* Active, Non-active, Unknown	*/
static char *active = "A", *nonactive = "N";

static char *systype = "non-DOS";	/* type of system indicator	*/
static char *dos12bit = "DOS";		/* DOS 12-bit FAT		*/
static char *dos16bit = "DOS";		/* DOS 16-bit FAT		*/
static char *unix5_2  = "System5";
static char *other    = "unknown";
static char *memname  = "/dev/mem";	/* M006 */
char *path[] = { "/dev/rdsk/0s0" , "/dev/rdsk/1s0" };
int unit=0;
int otto = 0,booter = 0, changes = 0, reboot = 0;
char rbuf[80];				/* M000: Any routine which makes
					 * mod's must inc this reply buffer
					 */
unsigned char tabbuf[512];
unsigned int spt, tpc, ncyl, type, spc;

char create_part(), change_part(), delete_part(), display_part(), rawpart();
char new_unit();
extern char fdisk_end(), help(), insdisk();
char (*menutbl[])() = { create_part, change_part, delete_part,
			display_part, insdisk, fdisk_end, help, rawpart,
				new_unit };

extern struct i1010drtab drvtab;
extern struct partition *pt;
extern dev_t fd;
extern int optind;

main(argc,argv)
int argc; char **argv;
{
	char c;
	int  i;
	unsigned int m, menu();

/* check for auto initialize */
	while ((c = getopt(argc,argv, "is"))!= EOF)
		switch (c)
		{
		case 'i':	 otto = 1;

		case 's':	booter = 1;
				++changes;		/* M006 */
		};
	if (optind < argc)
	{
		printf ("\n%s\n",argv[optind]);
		unit = atoi(argv[optind]);
	};
	init_unit();
	while ((m=menu()) != EOF) {
		(*menutbl[m])();
	}
}

/* new_unit changes the disk drive affected by fdisk */
char
new_unit()
{
	fdisk_new(); /* close */
	close(fd);  /*previous unit*/
	unit++;
	if (unit > MAXUNIT) unit = 0;

	init_unit();

}
/* rawpart: print out the actual partition table			*/
char
rawpart()
{
	int p;
	struct parttab *pp;

	pp = &ptab;
	for (p=0; p < 4; p++ ) {
		printf("partition #%d:\n", p+1);
		printf("\tboot ind: [%x, %x, %x, %x], ", 
			pp->p[p].bootind, pp->p[p].bih,
			pp->p[p].bis, pp->p[p].bicyl);
		printf("\tsystem ind: [%x, %x, %x, %x]\n", 
			pp->p[p].systind, pp->p[p].sih,
			pp->p[p].sis, pp->p[p].sicyl);
		printf("\t[rel=%lx] ", 
			pp->p[p].rel.sector);
		printf("\t[nsec=%lx]\n", 
			pp->p[p].nsec.partsize);
	}
	printf("--- partition signature: %x\n", pp->sig.signature);

}

/* partprint():	print out the partition table in a DOS-fdisk fashion	*/
partprint(pp)
struct parttab *pp;
{
	int i;
	unsigned int size, start;
	struct partition *pn;
	unsigned char beginsec, endsec;
	unsigned int begincyl, endcyl;

	printf("Partition\tStatus\tType\t\tStart\tEnd\tSize\tBlocks\n");
	for (i=0; i<4; i++) {
		pn = &pp->p[i]; 
		printf("   %d\t\t", 4-i);
		if (pn->bootind == 0x80) {
			boottype = active;
		} else if (pn->bootind == 0x00) {
			boottype = nonactive;
		}
		printf("%s\t", boottype);

		switch (pn->systind) {		/* System Indicator	*/
			case 0x01:		/* DOS 12-bit FAT	*/
				systype = dos12bit;
				break;
			case 0x04:		/* DOS 16-bit FAT	*/
				systype = dos16bit;
				break;
			case 5:			/* real true blue unix	*/
				systype = unix5_2;
				break;
			case 0x52:		/* real true blue unix	*/
				systype = unix5_2;
				break;
			case 0:
			default:
				systype = other;
				break;
		}
		if (strlen(systype) < 8)
			printf("%s\t\t", systype);
		 else printf("%s\t",systype);

		begincyl = ((pn->bis & 0xC0) << 2) | pn->bicyl;
		beginsec = pn->bis & 0x3F;
		endcyl = ((pn->sis & 0xC0) << 2) | pn->sicyl;
		endsec = pn->sis & 0x3F;
		if (pn->systind) size = (endcyl - begincyl) + 1;
		else size = 0;
		printf("%d\t%d\t%d\t%ld\n", begincyl, endcyl, size,(long)size*spc);
	}
}

/* menu(): Print out the high level menu of options		*/
unsigned int
menu()
{
	int p;
	char *s;

	for (p=0; *mainmenu[p] != '0'; p++) {
	    if (p == 9)
		printf ("%s [ %d ]\n\n\n",mainmenu[p],unit);
	    else
		printf("%s", mainmenu[p]);
	}
	
	for (s = rbuf; (s=fgets(s, 80, stdin)) != NULL;)  /* M004 */
	    switch (*s) 
	    {
		    case '1':		/* Change Partition	*/
		    case '2':		/* Activate Partition	*/
		    case '3':		/* Delete Partition	*/
		    case '4':		/* Print Paritions	*/
		    case '5':		/* Bad Track mapping	*/
			    return((unsigned int) *s - '0' - 1);
			    break;
		    case '6':		/* change units */
			    return(8);
		    case 'x':			/* exit fdisk	*/
		    case 'X':
			    return(5);
		    case '?':			/* help menu	*/
		    case 'h':
			    return(6);
		    case 'r':			/* raw ptab	*/
			    return(7);
		    case EOF:
			    return(EOF);
		    default:
		    printf("\n\n  Invalid Option!! -- type ? for help\n\n\n");
		    for (p=0; *mainmenu[p] != '0'; p++)   /* M004 */
			if (p == 9 ) 
			    printf ("%s [ %d ]\n\n\n",mainmenu[p],unit);
			else
			    printf("%s", mainmenu[p]);
	    }
}

/* menu #1: create a partition (not limited to DOS).			*/
char
create_part()
{
	unsigned int p, start, end, tsec, begincyl, endcyl, size;
	unsigned int tb, te, ts;
	unsigned char t;
	char *s;
	struct partition *pp;
#ifdef BOOTDRVTBL
	extern char bsdptupdate;
#endif BOOTDRVTBL

	s = rbuf;
startpart:
	do {
		for (p=0; *menu1[p] != '0'; p++) {
			printf("%s", menu1[p]);
		}
		partprint(&ptab);
		printf("\n\nPress 'x' to return to FDISK options\n\n");
		printf("Which partition would you like to create [1-4]?...: ");
	  if ( (s=fgets(s, 80, stdin)) == NULL ) return;  /* M004 */
		if (EXIT(s)) {
			return;
		}
		p = atoi(s);
		if ((p < 1) || (p > 4)) {
			printf
	("\n\n  Invalid partition # -- enter a number between 1 and 4.\n\n\n");
		}
	} while ((p < 1) || (p > 4));

	pp = &ptab.p[4-p];
	if (pp->systind != 0) {
		printf("\n\n  Partition #%d is currently set up. If you\n", p);
		printf("  really want to change it, you must first delete\n");
		printf("  the partition via main menu option #3.\n\n\n");
		goto startpart;
	}

startcyl:
	printf("Enter Starting Cylinder Value: ");
	if ( (s=fgets(s, 80, stdin)) == NULL ) return;  /* M004 */
	if (EXIT(s)) return;
	if ((start = atoi(s)) < 0 || (start > ncyl)) {
		printf("Invalid starting cylinder #\n");
		goto startcyl;
	}
	if (start == 0) {			/* Start M002 */
		printf("Cannot start a partition on cylinder 0\n");
		goto startcyl;
	}					/* End M002 */
	if ((te = occupied(start,start ))) {
		printf("Invalid starting cylinder #\n");
		printf("Already in partition #%d\n", te);
		goto startcyl;
	}

endingcyl:
	printf("Enter Ending Cylinder Value: ");
	if ( (s=fgets(s, 80, stdin)) == NULL ) return;  /* M004 */
	if (EXIT(s)) return;
	if ((end = atoi(s)) < 0 || end > ncyl - 1) {	/* M003 */
		printf("Invalid ending cylinder #\n");
		goto endingcyl;
	}

	if (end < start) {
		printf("The end cylinder must be greater than\n");
		printf("the starting cylinder!\n");
		goto startcyl;
	}

		if ((te = occupied(start,end ))) {
			printf("Invalid ending cylinder #\n");
			printf("Overlaps partition #%d\n", te);
			goto endingcyl;
		}
	pp = &ptab.p[4-p];
osloop:
	printf("What operating system (1=DOS, 5=UNIX)?: ");
	if ( (s=fgets(s, 80, stdin)) == NULL ) return;  /* M004 */
	if (EXIT(s)) return;
	switch (*s) {
		case '1':			/* DOS 12-bit FAT	*/
		case '4':			/* DOS 16-bit FAT	*/
			pp->systind = (unsigned char) atoi(s);
			break;
		case '5':			/* UNIX V		*/
			pp->systind = (unsigned char) 0x52;
#ifdef BOOTDRVTBL
			bsdptupdate = 2;	/* request boot sector write */
#endif BOOTDRVTBL
			break;
		default:
			printf("Invalid entry\n");
			goto osloop;
	}

	pp->rel.sector = (long)(start * spc);
	pp->nsec.partsize =(long)( ((end - start) +1) * spc);
	pp->bih = 0;
	tb = start & 0x300;
	t = 1;
	if (start == 0) {
		t = 2;				/* avoid master boot	*/
	}
	pp->bis = (unsigned char) ((tb >> 2) | t);
	pp->bicyl = (unsigned char) (start & 0xFF);
	pp->sih = (unsigned char) tpc - 1;
	te = end & 0x0300;
	pp->sis = (unsigned char) ((te >> 2) | spt);
	pp->sicyl = (unsigned char) (end & 0xFF);

	changes++;				/* virgin no longer	*/

	goto startpart;

}

/* menu #2: change the active partition.				*/
char
change_part()
{
	int p,a;
	char *s;

	for (;;) {
		do {
			for (p=0; *menu2[p] != '0'; p++) {
				printf("%s", menu2[p]);
			}
			partprint(&ptab);
			printf("\n\n");
			printf("Type 'x' to return to FDISK Options\n\n");
			printf("\nEnter the number of the partition you\n");
			printf("want to make active (1-4)............: ");
			s = rbuf;
	    if ( (s=fgets(s, 80, stdin)) == NULL ) return;  /* M004 */
			if (EXIT(s))
				return;
			a = atoi(s);
			if ((a < 1) || (a > 4)) {
				printf("\n\n");
			  printf("  Invalid partition # -- enter a number between 1 and 4.\n");
				printf("\n\n");
			}
		} while ((a < 1) || (a > 4));

		a = 4 - a;			/* M001: dos-reverse order */
		for (p=0; p<4; p++) {
			if (p == a)		/* M001 */
				{
				ptab.p[p].bootind = 0x80;
				pt = &ptab.p[p];
				}
			else
				ptab.p[p].bootind = 0x00;
		}
		
	if (!(ckactpar())) 
		{
			int	btwasvalid;
			/* remember if badtrax are valid */
			btwasvalid = BTvalid();
			read_PER();
			if (btwasvalid) validateBT();
			else if (BTvalid()) read_BT();
				else initbad();
		};
		changes++;			/* critical changes	*/
		reboot++;			/* M006 must now reboot	*/
	}
}

/* menu #3: delete a partition (not limited to DOS).			*/
char
delete_part()
{
	int p;
	char *s;
	struct partition *dp;

	for (;;) {
repeat:
		do {
			for (p=0; *menu3[p] != '0'; p++) {
				printf("%s", menu3[p]);
			}
			partprint(&ptab);
			printf("\n\nType 'x' to return to FDISK options\n");
			printf("\nWARNING! -- \n");
			printf("All partition data will be DESTROYED\n");
			printf("and all bad track data will be LOST,\n");
			printf("if active partition is deleted!\n");
			printf("Do you want to continue? (y or n): ");
			s = rbuf;
			*s = 'n';
	    if ( (s=fgets(s, 80, stdin)) == NULL ) return;  /* M004 */
			if ((*s == 'n') || (*s == 'N'))
				return;
			if (EXIT(s))
				return;
			if ((*s != 'y') && (*s != 'Y'))
				printf("\n\n  Invalid input.\n\n\n");
		} while ((*s != 'y') && (*s != 'Y'));
		printf("Which partition do you wish to delete?: ");
	  if ( (s=fgets(s, 80, stdin)) == NULL ) return;  /* M004 */
		if (((p = atoi(s)) < 1) || (p > 4)) {
			printf("\n\n");
			printf("  Invalid partition # -- enter a number between 1 and 4.\n");
			printf("\n\n");
			goto repeat;
		}
		printf("Deleting Partition #%d\n", p);
		dp = &ptab.p[4-p];
		if ( dp->bootind == 0x80 )
			reboot++;		/* M006 */
		dp->bootind = 0;
		dp->bih = 0;
		dp->bis = 0;
		dp->bicyl = 0;
		dp->systind = 0;
		dp->sih = 0;
		dp->sis = 0;
		dp->sicyl = 0;
		dp->rel.sector = 0;
		dp->nsec.partsize = 0;
		changes++;			/* must now reboot	*/
	}
}

/* menu #4: display partition information.				*/
char
display_part()
{
	int p;
	char *s;

	do {
		for (p=0; *menu4[p] != '0'; p++) {
			printf("%s", menu4[p]);
		}
		partprint(&ptab);
		printf("\n\n\n\n\n");
		printf("\nPress 'x' to return to FDISK options:");
	  s = rbuf;  /* M004 */
	  s = fgets(s, 80, stdin);  /* M004 */
	} while (NOEXIT(s));  /* M004 */
}

fdisk_new()
{
	int  i, valid,bfd;
	char *s;
#ifdef BOOTDRVTBL
	extern int	dptdbg;
#endif BOOTDRVTBL

	s = rbuf;
	valid = 0;

	if (changes) {			/* update universe	*/
		changes = 0; /* clear out change indicator */
		printf("- Do you want your changes installed on the previous unit ?\n");
		printf("-                 (y or n)?: ");

		while ( fgets(s, 80, stdin) != NULL )  /* M004 */
		{
		   switch (*s) {
			case 'y':
			case 'Y':
				for (i=0; i <=4; i++) {
					if (ptab.p[i].bootind == 0x80) {
						valid++;
						pt = &ptab.p[i];
					}
				}
				if (valid == 0) {
					printf("No Active Partition!\n");
					printf("You must activate a partition");
					printf(" before updating the");
					printf(" partition table.\n");
					return;
				}
				printf("Updating disk unit %d ", unit);
				ptab.sig.signature = VALID_PTAB;

				if (booter) 
				{
					if ((bfd = open("/etc/master.bblock", O_RDWR)) == -1) {
						fprintf(stderr, "Can't open %s, errno=%d\n", FILE, errno);
						exit(errno);
					}

					read(bfd,tabbuf,512) ;
				};
				readtbuf();

#ifdef BOOTDRVTBL
				if (dptdbg < 2)
#endif BOOTDRVTBL
				if (write_sector(fd, tabbuf,0,0,1) ) {
		 		  fprintf(stderr, "Can't write %s, errno=%d\n", 
					OUTFILE, errno);
					exit(errno);
				}
				if (pt->systind == 5 ||
				    pt->systind == 0x52 )
				{
#ifdef BOOTDRVTBL
				    if (dptdbg < 2)
				    {
#endif BOOTDRVTBL
					write_BT();  /*write out bad tracks */
					write_PER(); /* and partition end */
#ifdef BOOTDRVTBL
				    }
					write_PBS(); /* and boot sector   */
#endif BOOTDRVTBL
				};
#ifdef BOOTDRVTBL
			    if (dptdbg > 1)
				reboot = 0;
			    else
#endif BOOTDRVTBL
				reboot++;	/* force reboot */
				return;

			case 'x':
			case 'X':
			case 'n':
			case 'N':
				return;
		
	   		}

		printf("\007\n\n-                 (y or n)?  ");
		}
	}
	else {
		printf("\n");
		return;			/* no update	*/
	}

}

char
fdisk_end()
{
	int i;
	printf("\nLeaving FDISK ");

	fdisk_new();

	if (reboot)				/* update universe	*/
	{
		sync(); sync();
		if (hd_root())			/* M006 */
			do_reboot();		/* M006 */

		printf(" the hard disk will now be reinitialized\n");
		while (i= ioctl(fd,I1010_REINIT,0))
			printf("Disk still busy. Reboot!\n!");
	}
	exit(0);
}

char
help()
{
	int p;
	char *s;
	do   /* M005 */
	{
	    for (p=0; *helpmenu[p] != '0'; p++) 
	    {
		  printf("%s", helpmenu[p]);  /* M004 */
	    }

	    printf("\n\n\n");
	    printf("\nPress 'x' to return to FDISK options:");
	    s = rbuf;  /* M004 */
	    s = fgets(s, 80, stdin);  /* M004 */
	} 
	while (NOEXIT(s));  /* M004 */
}

copytbuf()
{
	int i;
	unsigned char *tp;

	tp = tabbuf + PARTOFF;

	for (i=0; i < 4; i++) {
		ptab.p[i].bootind = *tp++;
		ptab.p[i].bih = *tp++;
		ptab.p[i].bis = *tp++;
		ptab.p[i].bicyl = *tp++;
		ptab.p[i].systind = *tp++;
		ptab.p[i].sih = *tp++;
		ptab.p[i].sis = *tp++;
		ptab.p[i].sicyl = *tp++;
		ptab.p[i].rel.s[0] = *tp++;
		ptab.p[i].rel.s[1] = *tp++;
		ptab.p[i].rel.s[2] = *tp++;
		ptab.p[i].rel.s[3] = *tp++;
		ptab.p[i].nsec.n[0] = *tp++;
		ptab.p[i].nsec.n[1] = *tp++;
		ptab.p[i].nsec.n[2] = *tp++;
		ptab.p[i].nsec.n[3] = *tp++;
	}
	ptab.sig.s[0] = *tp++;
	ptab.sig.s[1] = *tp;
}

readtbuf()
{
	int i;
	unsigned char *tp;

	tp = tabbuf + PARTOFF;

	for (i=0; i < 4; i++) {
		*tp++ = ptab.p[i].bootind;
		*tp++ = ptab.p[i].bih;
		*tp++ = ptab.p[i].bis;
		*tp++ = ptab.p[i].bicyl;
		*tp++ = ptab.p[i].systind;
		*tp++ = ptab.p[i].sih;
		*tp++ = ptab.p[i].sis;
		*tp++ = ptab.p[i].sicyl;
		*tp++ = ptab.p[i].rel.s[0];
		*tp++ = ptab.p[i].rel.s[1];
		*tp++ = ptab.p[i].rel.s[2];
		*tp++ = ptab.p[i].rel.s[3];
		*tp++ = ptab.p[i].nsec.n[0];
		*tp++ = ptab.p[i].nsec.n[1];
		*tp++ = ptab.p[i].nsec.n[2];
		*tp++ = ptab.p[i].nsec.n[3]; 
	}
	*tp++ = ptab.sig.s[0];
	*tp++ = ptab.sig.s[1];
}

/*
  Check partition limits to be sure they are vacant
*/
occupied(start,end)
{
	struct partition *pp;
	int begincyl, endcyl, i;
	pp = &ptab.p[0];
	for (i=4 ; i>0 ; i-- )
	{
		begincyl = ((pp->bis & 0xC0) << 2) | pp->bicyl;
		endcyl = ((pp->sis & 0xC0) << 2) | pp->sicyl;
		pp++;
		if (!endcyl) continue;
		if ((end < begincyl) || (start > endcyl)) continue;
		return(i); /* not vacant */
	}
	return(0); /* vacant */
}

/* M006: Check to see if rooted on hard disk */
hd_root()
{
	extern char *memname;
	int	mfd;
	struct sys_info sis;
	if ((mfd=open(memname, O_RDONLY)) < 0) {
		fprintf(stderr, "Cannot read from kernel memory\n");
		return(0);
	}
	if (ioctl(mfd, IOCIOP_SYSINFO, &sis) == -1) {
		fprintf(stderr, "Fdisk: _SYSINFO failed: %d\n", errno);
		close(mfd);
		return(0);
	}
	close(mfd);
	if ((sis.rootdev & 0xfff) == 0)
		return(1);
	return(0);
}

/* M006: Reboot via mmioctl(); same as CTRL-ALT-DEL. */
do_reboot()
{
	extern char *memname;
	int	mfd;
	if ((mfd=open(memname, O_RDONLY)) < 0) {
		fprintf(stderr, "Cannot read from kernel memory\n");
		return;
	}
	if (ioctl(mfd, IOCIOP_REBOOT, 0) == -1)
		fprintf(stderr, "Fdisk: _REBOOT failed: %d\n", errno);
	else {
		printf(
	"\nSystem will now reboot due to the active partition change.\n");
		sleep(5);		/* give time for 80286 reset */
	}
	close(mfd);
}
