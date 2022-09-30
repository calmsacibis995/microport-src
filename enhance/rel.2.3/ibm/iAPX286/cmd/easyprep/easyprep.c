static char * sccsid = "@(#)easyprep.c	1.2";
/*
 * This program creates the input files needed during the execution of
 * easyinstall shellscript.
 * Compile with -D DEBUGGER to get testing and demo image.
 * Compile with -D RAWOUT to get version that shows output of utilities on
 *   screen.
 * Written by Bernie Riff 10/86.
 */

/* 
 * Modification history:
 *   M000 uport!bernie Thu Dec  4 20:02:23 PST 1986
 *     Removed EXTRA_PRINTS since they are no longer necessary for debugging.
 *     Removed unneeded fflush from query().
 *     Clarified prompting for bad track information.
 *   M001 uport!bernie Thu Dec 18 13:00:17 PST 1986
 *     Prevent use of cylinder 0 for UNIX partition.
 *     Add real screen-clearing between sections.
 *   M002 uport!bernie Thu Jan 8 18:00:10 PST 1987
 *     Include getype.c to centralize code for disk-parameter-getting functions.
 *     Remove functions already defined in getype.c .
 *     Added parameter to call of get_drive_params().
 *     Added code prior to get_drive_params() to access the information in the
 *       master boot block.
 *     Changed drt from a pointer to a drive table to the name of a drive table.
 *     Deleted drive type information from input files to format and fdisk.
 *     Check if system will find drive parameters with found_params() before 
 *       including answer to "Is this correct" in input files to format and 
 *       fdisk.
 *     Changed variable named DEBUG to DEBUGGER to remove conflict with
 *       getype.c .
 *     Test entered bad tracks to make sure they are within bounds of disk
 *       parameters.
 *     Recommend the use of fdisk rather than installit for further
 *       parititioning.
 *     Strengthen warnings when all files and partitions might be destroyed.
 *     Removed device openers and master boot block reads from found_params()
 *       and partdump().
 *     Clarified prompting for whether runtime system disks will be installed.
 *   M003 uport!bernie Mon May 18 15:49:38 PST 1987
 *     Because of need to issue write-protected boot floppy, must eliminate the
 *       writing of various files to the boot floppy by easyprep.
 *     Eliminate use of write_log().
 *     Accumulate input for format, fdisk, and divvy in buffers, and pull the
 *       execution of fdisk and divvy into easyprep.
 *     Close hard disk as soon as possible.
 *     Create variable named RAWOUT that when defined sends output of 
 *       utilities to the screen.
 *   M004 uport!bernie Mon May 25 18:42:11 PDT 1987
 *     Add prompting for installation of new additional disks.
 *   M005 uport!rex	Wed Aug 19 15:10:50 PDT 1987
 *	Make scan optional with command line flag -s; default is to scan.
 *	Make scan an option to operator with a question prompt after bad
 *	    track decision.
 *	Correct System V/AT recognition for new system indicator and
 *	    compatibility with old system indicator and DOS 3.3.
 *	Correct problem with a floating point expression using integer
 *	    arithmetic.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <malloc.h>
#include "sys/misc.h"
#include "sys/cmos.h"
#include "sys/wn.h"
#include "sys/wndefaults.h"
#include <sys/uadmin.h>
#include "sys/fdisk.h"

#define PARTOFF 0x1BE
#define FILENM path[unit]
#define BOOT_AND_UTIL_SIZE 1500
#define DISK_SIZE 1200
#define SWAP_SIZE 2000
#define WORK_SIZE 2000
#define OVERHEAD_SIZE 510
#define SECTORS_PER_TRACK 17

struct bad_spot
  {
    int cyl;
    int track;
    struct bad_spot *next_bad;
  };

dev_t fd;
char *path[] = { "/dev/rdsk/0s10" };
int unit=0;
unsigned char tabbuf[512];
unsigned int ncyl;
struct i1010drtab  drt;  /* M002 */
static int partit_info[4][4] = { 4, -1, -1, -1,
                                 3, -1, -1, -1,
                                 2, -1, -1, -1,
                                 1, -1, -1, -1 };
static int pieces[4][4] = { 1, -1, -1, -1,
                            2, -1, -1, -1,
                            3, -1, -1, -1,
                            4, -1, -1, -1 };
int empty_partit, ptable_built, gaptable_built, bad_tracker;
int start_cylinder, end_cylinder, clear_pt;
struct bad_spot *new_bad, *head_bad, *tail_bad;
int bbucket, tty, status, to_child[2];  /* M003 */
int scan, early_stop;

#ifdef DEBUGGER
int will_format, real_pt;
#endif

#include "../getype.c"

/****************************************************************************/

/*
 *  found_params() determines whether fdisk will find the hard disk parameters.
 */

int found_params()     /* M002 */
{
  struct i1010drtab  *drivetab;

/* if parameter table in master boot block is ok, use that */
  if (checkdp(tabbuf+DISKPARMS)) return (1);

/* if not, try reading ROM table */
  readcmos();
  type = unit ? cmos.disk & 0xF : cmos.disk >> 4 ;

  if (!getpar(unit,drivetab))   /* drive parms from ROM*/
    return (0);
  if ((type == 0) || (type > MAXTYPE)) 
    return (0);
  return (1); 
}

/****************************************************************************/

/*
 *  Copy partition table to buffer 
 */
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

/****************************************************************************/

/* partdump():  dumps the partition table in an array */

partdump(pp)
struct parttab *pp;
{
  int i;
  struct partition *pn;

  copytbuf();  /* copy partition table to buffer */

  for (i=0; i<4; i++) {
    pn = &pp->p[i];
    switch (pn->systind) {    /* System Indicator  */
      case 0x01:    /* DOS 12-bit FAT  */
    /*  partit_info[i][1] = 1;
        break; */
      case 0x04:    /* DOS 16-bit FAT  */
        partit_info[i][1] = 1;
        break;
      case 5:      /* real true blue unix  */
	if (pn->bootind != 0x80) {	/* M005: must be DOS 3.3 extension */
	    partit_info[i][1] = 5;
	    break;
	}				/* M005: else must be old System V/AT */
      case 0x52:    /* real true blue unix  */
        partit_info[i][1] = 0x52;
        break;
      case 0:
      default:
        partit_info[i][1] = 0;
        break;
    }

      /* beginning cylinder */
    partit_info[i][2] = ((pn->bis & 0xC0) << 2) | pn->bicyl; 
      /* ending cylinder */
    partit_info[i][3] = ((pn->sis & 0xC0) << 2) | pn->sicyl; 
  }
  ptable_built = 1;
}

/****************************************************************************/

/*
 * This function determines whether or not the current partition table is
 * empty.
 */

empty_table()
{
    int i;

    for ( i = 0; i < 4; ++i )
      if ( partit_info[i][3] )
        return (0);
    return (1);
}

/****************************************************************************/

/*
 * This function returns the number of an empty partition or returns 0 if
 * there is no empty partition.
 */

one_empty()
{
    int i;

    for ( i = 0; i < 4; ++i ) 
      if ( !partit_info[i][3] )
        {
          empty_partit = partit_info[i][0];
          return (empty_partit);
        }
    return (0);
}

/****************************************************************************/

/*
 * This function determines if there already is a UNIX partition on the disk.
 */

already_UNIX()
{
    int i;

    for ( i = 0; i < 4; ++i ) 
      if ( partit_info[i][1] == 0x52 )
        return (1);
    return (0);
}

/****************************************************************************/

/*
 * Sorts the rows of the 4x4 array A in non-decreasing order of the values
 * in column k.
 */

sort_array(A,k)
  int A[4][4], k;
{
    int i, j, h, lowrow, temp[4];

    for ( i = 0; i < 4; ++i )
      {
        lowrow = i;
        for ( j = i + 1; j < 4; ++j )
        if ( A[j][k] < A[lowrow][k] )
          lowrow = j;
        for ( h = 0; h < 4; ++h )
            /* Move lowest remaining row into ith position */
          {
            temp[h] = A[lowrow][h];
            A[lowrow][h] = A[i][h];
            A[i][h] = temp[h];
          }
      }
}

/****************************************************************************/

/*
 * Makes an array of contiguous gaps in the current partition table and
 * then sorts these pieces by size of gap.
 * Assumes there are at most 3 partitions occuppied and therefore at most
 * 4 contiguous gaps.
 */

make_pieces_array()
{
    int i, j;

    sort_array(partit_info,3);  /* Sort partit_info array by end cylinders */

      /* Locate lowest (by end cylinder) occuppied partition */
    i = 0;
    while ( partit_info[i][3] <= 0 ) /* DOS partition may BEGIN at 0 */
      ++i;
    
      /* Find possible low-end gap */
    j = 0;
    if ( partit_info[i][2] > 1 )  
      {
        pieces[j][1] = 1;  /* UNIX parition begins with cylinder > 0 */
        pieces[j][2] = partit_info[i][2] - 1;
        pieces[j][3] = (pieces[j][2] - pieces[j][1]) + 1;
        ++j;
      }

      /* Find possible middle gaps */
    while ( i < 3 )
      {
        if ( partit_info[i][3] + 2 <= partit_info[i + 1][2] )
          {
            pieces[j][1] = partit_info[i][3] + 1;
            pieces[j][2] = partit_info[i + 1][2] - 1;
            pieces[j][3] = (pieces[j][2] - pieces[j][1]) + 1;
            ++j;
          }
        ++i;
      }
  
      /* Find possible high-end gap */
    if ( partit_info[i][3] < (drt.dr_ncyl) - 1 )  /* M002 */
      {
        pieces[j][1] = partit_info[i][3] + 1;
        pieces[j][2] = (drt.dr_ncyl) - 1;   /* M002 */
        pieces[j][3] = (pieces[j][2] - pieces[j][1]) + 1;
      }

    sort_array(pieces,3);  /* Sort pieces array by partition size */

    gaptable_built = 1;
}

/****************************************************************************/

/* 
 * This function tests to make sure that the starting and ending cylinders
 * of the new partition are "reasonable".
 */

valid_partit()
{
    int i;

    if ( start_cylinder <= 0 )
      return (0);
    if ( end_cylinder <= 0 )
      return (0);
    if ( start_cylinder >= end_cylinder )
      return (0);
    if ( end_cylinder >= drt.dr_ncyl )  /* M002 */
      return (0);

    if ( !clear_pt )
      for ( i = 0; i < 4; ++i )
        {
            /* If new partition overlaps an existing partition, return 0 */
          if ( end_cylinder == partit_info[i][3] )
            return (0);
          if ( end_cylinder < partit_info[i][3] &&
	      end_cylinder >= partit_info[i][2] )
            return (0);
          if ( end_cylinder > partit_info[i][3] &&
	      start_cylinder <= partit_info[i][3] )
            return (0);

            /* If new parition number is already used, return 0 */
          if ( empty_partit == partit_info[i][0]  &&  partit_info[i][3] != 0 )
            return (0);
        }

    return (1);
}

/****************************************************************************/

main (ac, av)
    int		ac;
    char	**av;
{
    long int   needed_Kbs, extra_Kbs, total_Kbs, UNIX_Kbs;
    int   total_cylinders, heads, valid_bt;  /* M002 */
    int   percent_UNIX, UNIX_cylinders, i, j, temp;
    char  buf[81];
    FILE  *outfile, *fopen();

    bad_tracker = 0;
    clear_pt = 0;
    ptable_built = 0;
    gaptable_built = 0;
    empty_partit = 4; /* Arbitrary and may be changed if empty() is executed */
    percent_UNIX = 0;

    scan = 1;			/* M005: start */
    early_stop = 0;
    while (--ac)
      {
	if (strcmp(*(++av), "-noscan") == 0)
	  {
	    scan = 0;
	    continue;
	  }
	if (strcmp(*av, "-stop") == 0)
	  {
	    early_stop = 1;
	    continue;
	  }
      }				/* M005: end */
	
#ifdef DEBUGGER
    printf("[H[J");      /* M001 */
    will_format = 1;
    real_pt = 1;

    printf("\nWelcome to the demonstration ");
    printf("and testing version of easyinstall. \n");
    printf("The interface of the standard version is identical to what you \n");
    printf("will see except for the first few screens of queries. \n\n");
    printf("\nYou have the option of turning ");
    printf("off access to the format utility. \n");
    if ( query("Do you wish to do this?") )
      will_format = 0;

    printf("[H[J");      /* M001 */
    printf("\nYou have the option of either ");
    printf("accessing the real partition table \n");
    printf("or creating your own trial partition table.  If you choose not \n");
    printf("to use the real paritition table, easyinstall will expect to \n");
    printf("find your trial partition table information in a file called\n");
    printf("ptable that is located in either /etc or your home directory \n");
    printf("(whichever is appropriate in this case).  The file, ptable, \n");
    printf("should consist of four entries; each entry should consist \n");
    printf("of a partition number, an operating system number (none=0, \n");
    printf("DOS=1, UNIX=5), a beginning cylinder number, and and ending\n");
    printf("cylinder number (all in the indicated order). \n");
    if ( !query("Are you going to want to use the real partition table?") )
      {
        real_pt = 0;
        if ( !query("Have you created ptable?") )
          {
            printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
            exit(1);
          }
      }

    printf("[H[J");      /* M001 */
    printf("\nYou have the option of either having the installation \n");
    printf("include a bad track scan or not.  Note that the user does \n");
    printf("not have this option. \n");
    if ( !query("Are you going to want to do a bad track scan?") )
      scan = 0;

    printf("[H[J");      /* M001 */
    printf("\nYou have the option of either running only the preliminary \n");
    printf("portion of easyinstall (creating the input files for fdisk \n");
    printf("and divvy) or a full installation. \n");
    if ( !query("Do you wish to do a full installation?") )
      early_stop = 1;

    printf("[H[J");      /* M001 */
    printf("\nWe now begin the normal easyinstall interface. \n");
    if ( !query("Do you wish to continue?") )
      {
        printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
        exit(1);
      }

#endif

    printf("[H[J");      /* M001 */
    printf("\nWelcome to easyinstall.  This program is intended for the \n");
    printf("inexperienced user who wishes to do a basic installation \n");
    printf("of System V/AT by creating an active UNIX partition with \n");
    printf("all or a portion of his/her single hard disk. \n");
    printf("\nThe entire installation process will involve seven steps:\n");
    printf("    1) specifying your disk's parameters and bad tracks,\n");
    printf("    2) if desired, formatting your hard disk,\n");
    printf("    3) creating the desired active UNIX partition,\n");
    printf("    4) scanning your hard disk for bad tracks,\n");
    printf("    5) making the necessary file systems,\n");
    printf("    6) copying files from your boot floppy disk to\n");
    printf("       your hard disk, and\n");
    printf("    7) installing the remaining System V/AT floppies.\n");
    printf("\nIf you do not understand what all this means, DO NOT BE \n");
    printf("ALARMED; most of the process will be carried out for you \n");
    printf("automatically.  You will, however, need to know various \n");
    printf("parameters for your specific hard disk.  These usually \n");
    printf("can be found in your disk owner's manual. \n\n");
    if ( !query("Do you wish to begin easyinstall?") )
      {
        printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
        exit(1);
      }

    printf("[H[J");      /* M001 */
    if ((fd = open(FILENM, O_RDWR)) == -1)   /* M002 */
      {
	fprintf(stderr, "Can't open %s, errno=%d\n", FILENM, errno);
	exit(errno);
      }
    read(fd,tabbuf,512) ;	/* dummy read to force wnsweep */  /* M002 */
    if (read_sector(fd,tabbuf,0,0,1 ))   /* M002 */
      {
	fprintf(stderr, "Can't read %s, errno=%d\n", FILENM, errno);
	exit(errno);
      }
    close(fd);   /* M003 */
    get_drive_params(&drt);    /* M002 */

    total_cylinders = drt.dr_ncyl;  /* M002 */
    heads = drt.dr_nfhead;   /* M002 */

    printf("[H[J");      /* M001 */
    printf("\nFor various reasons, many hard disks have small regions \n");
    printf("on them, called bad tracks, that your computer will not \n");
    printf("be able to use.  Your system needs to be made aware of \n");
    printf("their locations, so that it will be able to avoid them.\n");
    printf("This will be done in two ways: \n");
    printf("    1) you will first be asked to enter the locations \n");
    printf("       (one at a time) of any bad tracks discovered by \n");
    printf("       the disk's manufacturer (these are usually listed \n");
    printf("       on a label attached directly to the hard disk); \n");
    printf("       and \n");
    printf("    2) later, as part of this installation procedure,\n");
    printf("       your disk should also be scanned for additional\n");
    printf("       bad tracks.\n\n");
    if ( query("Do you wish to enter any bad track information?") )  /* M000 */
      {
        bad_tracker = 1;     /* Set bad track flag */
        new_bad = (struct bad_spot *) malloc(sizeof (struct bad_spot));
        head_bad = new_bad;
        tail_bad = new_bad;
	valid_bt = 0;  /* M002 */
	while ( !valid_bt )  /* M002 */
	  {
            new_bad->cyl = getno("Enter CYLINDER number of the bad track:");
	    if ( new_bad->cyl > total_cylinders - 1 || new_bad->cyl < 0 )
		printf(" The value you entered is out of the valid range.\n");
	    else
		valid_bt = 1;
	  }
	valid_bt = 0;    /* M002 */
	while ( !valid_bt )   /* M002 */
	  {
	    new_bad->track =
		    getno("Enter HEAD (or TRACK) number of the bad track:");
	    if ( new_bad->track > heads - 1 || new_bad->track < 0 )
		printf(" The value you entered is out of the valid range.\n");
	    else
		valid_bt = 1;
	  }
        new_bad->next_bad = 0;
        printf("\n");
        while (query("Do you wish to enter any additional bad track information?"))
          {
            new_bad = (struct bad_spot *) malloc(sizeof (struct bad_spot));
	    valid_bt = 0;  /* M002 */
	    while ( !valid_bt )   /* M002 */
	      {
		new_bad->cyl = getno("Enter CYLINDER number of the bad track:");
		if ( new_bad->cyl > total_cylinders - 1 || new_bad->cyl < 0 )
		  printf(" The value you entered is out of the valid range.\n");
		else
		  valid_bt = 1;
	      }
	    valid_bt = 0;   /* M002 */
	    while ( !valid_bt )  /* M002 */
	      {
                new_bad->track =
			getno("Enter HEAD (or TRACK) number of the bad track:");
		if ( new_bad->track > heads - 1 || new_bad->track < 0 )
		  printf(" The value you entered is out of the valid range.\n");
		else
		  valid_bt = 1;
	      }
            new_bad->next_bad = 0;
            tail_bad->next_bad = new_bad;
            tail_bad = new_bad;
            printf("\n");
          }
      }

#ifndef	DEBUGGER
    if (scan)							/* M005 */
      if ( !query("Are you going to want to do the bad track scan?") )
	scan = 0;
#endif	/* ! DEBUGGER */

    printf("[H[J");      /* M001 */
    printf("\nYour hard disk may now be formatted.\n");
    printf("DO NOT format your hard disk unless you KNOW you need to do so.\n");
    if ( query("Do you wish to format your hard disk?") )
      {
        printf("[H[J");      /* M001 */
        printf("\nAllow hard disk to warm up ");
        printf("for half an hour BEFORE FORMATTING!\n");
        printf("Formating will ");
        printf("DESTROY ALL FILES AND EXISTING PARTITIONS \n"); /* M002 */
	printf("currently on your hard disk.  If you have not already done \n");
        printf("so, you may wish to backup the files on your hard disk \n");
	printf("before continuing.\n");
        if ( query("Are you ready to begin the formatting of your hard disk?") )
          {
            clear_pt = 1;  /* set flag to clear partition table */
            printf("[H[J");      /* M001 */
            printf("\nYour hard disk is now being formatted.\n");
            printf("This process may take 15 or 20 minutes.\n\n\n");

#ifdef DEBUGGER 
            if ( will_format )
              {					/* M003 */
		  pipe(to_child);

                if ( fork() == 0 )
		  {
			      /* hook child's input to parent's output */
		    close(0);
		    dup(to_child[0]);

#ifndef RAWOUT
			      /* hook child's output to bit bucket */
		    bbucket = open("/dev/null",O_WRONLY);
  	            close(1);
		    dup(bbucket);  
		    close(bbucket);
#endif
			      /* close unnecessary pipe descriptors */
		    close(to_child[0]);
		    close(to_child[1]);
    
		    execlp("/etc/format", "format", "/dev/rdsk/0s10", (char *) 0);
		  }
              
                  /* hook parent's output to child's input */
		close(1);
		dup(to_child[1]);
	
			/* close unnecessary pipe descriptors */
		close(to_child[0]);
		close(to_child[1]);

							    /* M003 */
                if ( found_params() )   /* M002 */
		  printf("n\n");                    /* drive type not correct */
                                       /* specify number of cylinders on disk */
                printf("%d\n", drt.dr_ncyl); /* M002 */
                                          /* specify number of heads/cylinder */
                printf("%d\n", drt.dr_nfhead);  /* M002 */
                printf("%d\n", drt.dr_lzone);         /* specify landing zone */
                                            /* specify write pre-compensation */
                printf("%d\n", drt.dr_precomp);  /* M002 */
                printf("y\n");              /* new specifications are correct */
		wait(&status);

		tty = open("/dev/tty", O_WRONLY);
		if ( tty == -1 )
		{
		  fprintf(stderr, "can't open /dev/tty\n");
		  exit(1);
		}
                  /* reconnect parent's output to terminal */
                close(1);
		dup(tty);
		close(tty);

		if ( status != 0 )
		  {
		    printf("\n\n\nformat failed.");
		    printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
		    exit(1);
		  }
                system("dd if=/etc/master.bblock of=/dev/rdsk/0s10 bs=1w 1> /dev/null 2>&1");
              }
#else
							/* M003 */
	    pipe(to_child);

            if ( fork() == 0 )
	      {
			  /* hook child's input to parent's output */
		close(0);
		dup(to_child[0]);

#ifndef RAWOUT
			  /* hook child's output to bit bucket */
		bbucket = open("/dev/null",O_WRONLY);
		close(1);
		dup(bbucket);  
		close(bbucket);
#endif
            
			  /* close unnecessary pipe descriptors */
		close(to_child[0]);
		close(to_child[1]);
              
		execlp("/etc/format", "format", "/dev/rdsk/0s10", (char *) 0);
	      }
              
              /* hook parent's output to child's input */
	    close(1);
	    dup(to_child[1]);
              
	      /* close unnecessary pipe descriptors */
	    close(to_child[0]);
	    close(to_child[1]);

							/* M003 */
            if ( found_params() )   /* M002 */
	      printf("n\n");                        /* drive type not correct */
				       /* specify number of cylinders on disk */
            printf("%d\n", drt.dr_ncyl); /* M002 */
					  /* specify number of heads/cylinder */
            printf("%d\n", drt.dr_nfhead);  /* M002 */
            printf("%d\n", drt.dr_lzone);             /* specify landing zone */
                                            /* specify write pre-compensation */
            printf("%d\n", drt.dr_precomp);  /* M002 */
            printf("y\n");                  /* new specifications are correct */
	    wait(&status);

	    tty = open("/dev/tty", O_WRONLY);
	    if ( tty == -1 )
	      {
		fprintf(stderr, "can't open /dev/tty\n");
		exit(1);
	      }
              /* reconnect parent's output to terminal */
            close(1);
	    dup(tty);
	    close(tty);

	    if ( status != 0 )
	      {
		printf("\n\n\nformat failed.");
		printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
		exit(1);
	      }

            system("dd if=/etc/master.bblock of=/dev/rdsk/0s10 bs=1w 1> /dev/null 2>&1");
#endif
            printf("[H[J");      /* M001 */
            printf("\nThe formatting of ");
            printf("your hard disk is now completed.\n\n\n\n");
          }
        else
          {
            printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
            exit(1);
          }
      }   

    if ( clear_pt )
      printf("\n\n");
    else
      printf("[H[J");      /* M001 */
    printf("\nYour responses to the following questions will ONLY be used to");
    printf("\ndetermine the amount of space you need on your hard disk.");
    printf("\nNote that the runtime.1, runtime.2, and utility.1 disks"); /* M002 M004 */
    printf("\nMUST be installed.\n\n\n"); /* M002 M004 */

    if (query("Do you also plan to install the runtime.3 disk?")) /*M002 M004*/
      needed_Kbs = BOOT_AND_UTIL_SIZE + (3 * DISK_SIZE) + SWAP_SIZE 
		   + WORK_SIZE + OVERHEAD_SIZE; 
    else
      needed_Kbs = BOOT_AND_UTIL_SIZE + (2 * DISK_SIZE) + SWAP_SIZE 
		   + WORK_SIZE + OVERHEAD_SIZE;
    printf("\n");
		  /* M004 */
    if ( query("Do you also plan to install the System Vision disk?") ) 
      needed_Kbs = needed_Kbs + DISK_SIZE;
    printf("\n");
		  /* M004 */
    if ( query("Do you also plan to install the Link kit disk?") ) 
      needed_Kbs = needed_Kbs + DISK_SIZE;
    printf("\n");

    if (query("Do you plan to install ALL software development system disks?") )
      needed_Kbs = needed_Kbs + (4 * DISK_SIZE);
    else
      {
        printf("\n");
        if ( query(
	  "Do you plan to install progdev.1, progdev.2 and progdev.3 disks?"))
          needed_Kbs = needed_Kbs + (3 * DISK_SIZE);
      }
    printf("\n");

    if ( query("Do you plan to install ALL text preparation system disks?") )
      needed_Kbs = needed_Kbs + (3 * DISK_SIZE);
    else
      {
        printf("\n");
        if ( query("Do you plan to install text.1 disk?") )
          needed_Kbs = needed_Kbs + DISK_SIZE;
      }

      /* cylinder 0 not used */
    total_Kbs = ((long int) total_cylinders - 1) * heads * SECTORS_PER_TRACK /2;
    extra_Kbs = total_Kbs - needed_Kbs;

    if ( extra_Kbs < 0 )
      {
        printf("\n\n\nThere is not enough space ");
        printf("for our product to be installed.\n");
        printf("%ld kilobytes will be required for this ", needed_Kbs);
        printf("installation of \n");
        printf("System V/AT.  ");
        printf("Your entire hard disk only has %ld kilobytes",total_Kbs);
        printf("\nof space that UNIX can use.");
        printf("\n\n\n\neasyinstall has been aborted at this point.\n\n\n");
        exit(1);
      }

    if ( !clear_pt )
      {
#ifdef DEBUGGER
        if ( real_pt )
          {
            printf("\n\n");
            partdump(&ptab);
          }
        else
          {
            outfile = fopen("ptable", "r");
            for ( i = 0; i < 4; ++i )
              for ( j = 0; j < 4; ++j )
                {
                  fscanf(outfile,"%d", &temp);
                  partit_info[i][j] = temp;
                }
            fclose(outfile);
            ptable_built = 1;
          }
#else
        printf("\n\n");
        partdump(&ptab);
#endif

        if ( empty_table() )
          {
            printf("[H[J");      /* M001 */
            printf("\nYour hard disk appears to be empty.\n");
            printf("If this is not true and you choose to continue, ALL \n");
				      /* M002 */
            printf("FILES AND EXISTING PARTITIONS currently on your hard \n"); 
	    printf("disk will be DESTROYED!\n");
            if ( query("Do you wish to continue?") )
              clear_pt = 1;  /* set flag to clear partition table */
            else 
              {
                printf(
		  "\n\n\neasyinstall has been aborted at this point.\n\n\n");
                exit(1);
              }
          }
      }

    if ( !clear_pt  &&  !one_empty() )
      {
        printf("[H[J");      /* M001 */
        printf("\nYour hard disk already has ");
        printf("the maximum number of partitions on it.\n");
        printf("If you continue, "); 
        printf("ALL FILES AND EXISTING PARTITIONS currently on \n"); /* M002 */
	printf("your hard disk will be DESTROYED!\n");
        if ( query("Do you wish to continue?") )
          clear_pt = 1;  /* set flag to clear partition table */
        else 
          {
            printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
            exit(1);
          }
      }

    if ( !clear_pt && already_UNIX() )
      {
        printf("[H[J");      /* M001 */
        printf("\nYour hard disk already has a UNIX partition on it.\n");
        printf("If you continue, ");
        printf("ALL FILES AND EXISTING PARTITIONS \n"); /* M002*/
	printf("currently on your hard disk will be DESTROYED!\n");
        if ( query("Do you wish to continue?") )
          clear_pt = 1;  /* set flag to clear partition table */
        else 
          {
            printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
            exit(1);
          }
      }
    
    if ( !clear_pt )
      {
        make_pieces_array();
          /* prompt on the basis of the largest gap found */
        total_Kbs = ((long int) pieces[3][3]) * heads * SECTORS_PER_TRACK /2;
        extra_Kbs = total_Kbs - needed_Kbs;
      }

    if ( extra_Kbs < 0 )
      {
        printf("[H[J");      /* M001 */
        printf("\nThere is not enough space ");
        printf("for our product to be installed.\n");
        printf("%ld kilobytes will be required for this ", needed_Kbs);
        printf("installation \n");
        printf("of System V/AT.  ");
        printf("The largest empty space on your hard disk \n");
        printf("only has %ld kilobytes that UNIX can use.\n\n",total_Kbs);
        printf("If you continue, ALL FILES AND EXISTING PARTITIONS \n");
	printf ("currently on your hard disk will be DESTROYED!\n");
        if ( query("Do you wish to continue?") )
          {
            clear_pt = 1;  /* set flag to clear partition table */
              /* reevaluate on basis of entire disk; cylinder 0 not used */
            total_Kbs = ((long int) total_cylinders - 1) 
			* heads * SECTORS_PER_TRACK /2;
            extra_Kbs = total_Kbs - needed_Kbs;
          }
        else 
          {
            printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
            exit(1);
          }
      }

    use_whole_disk:
    if ( extra_Kbs >= 0 )
      {
        printf("[H[J");      /* M001 */
        printf("\nIt is possible to create an active UNIX ");
        printf("partition of up \n");
        printf("to %ld kilobytes.  ", total_Kbs);
        printf("This would include %ld kilobytes \n", needed_Kbs - 2000);
        printf("for the basic installation of our product, 2000 kilobytes \n");
        printf("minimum working space, and UP TO %ld kilobytes of \n", extra_Kbs);
        printf("ADDITIONAL potential working space.\n");
        if ( !clear_pt  
	    &&  !query("Would this give you enough ADDITIONAL working space?") )
          {
            printf(
	      "\nIt is possible to create a larger UNIX parition, if you \n");
            printf("are willing to allow ");
            printf("ALL FILES AND EXISTING PARTITIONS \n"); /* M002 */
	    printf("currently on your hard disk to be DESTROYED. \n");
            if ( query(
	      "Do you wish your hard disk to be CLEARED of ALL CURRENT FILES?"))
              {
                clear_pt = 1;
                total_Kbs = ((long int) total_cylinders - 1) * heads 
			    * SECTORS_PER_TRACK /2;
                extra_Kbs = total_Kbs - needed_Kbs;
                goto use_whole_disk;  /* Try again with whole disk */
              }
            else
              {
                if ( query("Do you wish to abort easyinstall?") )
                  {
                    printf("\n\n\n\neasyinstall has ");
                    printf("been aborted at this point.\n\n\n");
                    exit(1);
                  }
              } 
          }
        if ( extra_Kbs > 0 )
          {
            printf("\nWhat PERCENTAGE of this ");
            printf("ADDITIONAL potential working space \n");
            printf("of %ld kilobytes do you wish to have included in the \n", 
								extra_Kbs);
            printf("UNIX partition you are creating?");
            do
              {
                percent_UNIX = getno("Specify an integer between 0 and 100:");
                if ( ( percent_UNIX < 0 )  ||  ( percent_UNIX > 100 ) ) {
                  printf("\n The integer you specified ");
                  printf("was NOT between 0 and 100.");
                }
              }
            while ( percent_UNIX < 0  ||  percent_UNIX > 100 );
          }
	}

      /* compute and round up UNIX_Kbs */
    if (percent_UNIX == 100)				/* M005: start */
	UNIX_Kbs = needed_Kbs + extra_Kbs;
    else
	UNIX_Kbs = needed_Kbs + (long) 
		(((extra_Kbs * percent_UNIX) + 1) / 100); /* M005: end */
    if ( clear_pt )
	/* start UNIX partition at the high end of hard disk */
      {
        UNIX_cylinders = ((2 * UNIX_Kbs) / ( SECTORS_PER_TRACK * heads )) + 1;
        if ( UNIX_cylinders > total_cylinders )
          UNIX_cylinders = total_cylinders;
        start_cylinder = total_cylinders - UNIX_cylinders;
            /* UNIX partition must start at cylinder > 0 */
        if ( start_cylinder == 0 )       /* M001 */ 
          start_cylinder = 1;
        end_cylinder = total_cylinders - 1; /* numbering starts with 0 */
      }
    else
      {
        UNIX_cylinders = ((2 * UNIX_Kbs) / ( SECTORS_PER_TRACK * heads )) + 1;
        if ( UNIX_cylinders > pieces[3][3] )  /* if larger than largest gap */
          UNIX_cylinders = pieces[3][3];
            /* find smallest gap that will work */
        for ( i = 0; UNIX_cylinders > pieces[i][3]; ++i )
          {
          };
            /* start UNIX partition at the high end of this gap */
        start_cylinder = pieces[i][2] - UNIX_cylinders + 1;
        end_cylinder = pieces[i][2]; 
      }
   

    printf("\n\n\n\nAn active UNIX partition of a total of %ld ", UNIX_Kbs);
    printf("kilobytes \n");
    printf("(%d cylinders) will now be created.\n\n", UNIX_cylinders);
    if ( clear_pt  &&  ( total_cylinders - UNIX_cylinders > 0 ) )
      {
        printf("The remaining %ld kilobytes ", total_Kbs - UNIX_Kbs);
        printf("(%d cylinders) of your \n", total_cylinders - UNIX_cylinders);
        printf("hard disk are unused but may be partitioned later by \n"); 
        printf("either using our fdisk command or the DOS \n");  /* M002 */
        printf("installation procedure.\n\n");
      }

    if ( !valid_partit() )
      {
        printf("The new partition is defective.");
        printf("\n\n\n\neasyinstall has been aborted at this point.\n\n\n");
        exit(1);
      }

    if ( early_stop )
      {
        printf("\n\n\nPreliminary easyinstall is complete.\n\n");
        exit(1);
      }
			
			/* M003 */
    if (scan)		/* M005 */
      {
	printf("\n\n\nAfter your UNIX partition has been created, a scan for bad\n");
	printf("tracks on your hard disk will automatically begin.  This\n");
	printf("entire process should take 20 - 90 minutes, depending on\n");
	printf("the size of your hard disk.\n\n");
      }
			/* M003 */
    pipe(to_child);

    if ( fork() == 0 )
	  {
		   /* hook child's input to parent's output */
		  close(0);
		  dup(to_child[0]);

#ifndef RAWOUT
		   /* hook child's output to bit bucket */
	    bbucket = open("/dev/null",O_WRONLY);
  	  close(1);
		  dup(bbucket);  
		  close(bbucket);
#endif
              
		   /* close unnecessary pipe descriptors */
		  close(to_child[0]);
		  close(to_child[1]);
    
	    execlp("/etc/fdisk", "fdisk", "-s", (char *) 0);
	  }
              
      /* hook parent's output to child's input */
	  close(1);
	  dup(to_child[1]);
  
      /* close unnecessary pipe descriptors */
    close(to_child[0]);
    close(to_child[1]);

			/* M002 */  /* M003 */
    if ( found_params() ) printf("n\n");            /* drive type not correct */
		                                   /* specify number of cylinders on disk */
    printf("%d\n", drt.dr_ncyl);    
                                          /* specify number of heads/cylinder */
    printf("%d\n", drt.dr_nfhead);  /* M002 */
    printf("%d\n", drt.dr_lzone);                     /* specify landing zone */
                                            /* specify write pre-compensation */
    printf("%d\n", drt.dr_precomp);  /* M002 */
    printf("y\n");                          /* new specifications are correct */
    if ( clear_pt )                /* choose to delete all exiting partitions */
      {
        printf("3\n");                   
        printf("y\n");                          
        printf("4\n");
        printf("y\n");
        printf("3\n");
        printf("y\n");
        printf("2\n");
        printf("y\n");
        printf("1\n");
        printf("n\n");
      }
    printf("1\n");                            /* choose to create a partition */
    printf("%d\n", empty_partit);                /* create in empty partition */
    printf("%d\n", start_cylinder);              /* specify starting cylinder */
    printf("%d\n", end_cylinder);                  /* specify ending cylinder */
    printf("5\n");                                /* make it a UNIX partition */
    printf("x\n");                                /* stop creating partitions */
    printf("2\n");                            /* choose to activate partition */
    printf("%d\n", empty_partit);               /* make UNIX partition active */
    printf("x\n");                              /* stop activating partitions */
    printf("5\n");                    /* choose to scan and assign bad tracks */
    printf("y\n");                     /* choose to write new bad track table */

    if ( scan )							      /* M005 */
      printf("y\n");                                            /* begin scan */
    else
      printf("n\n");                                           /* do not scan */

    if ( !bad_tracker )
      printf("n\n");                      /* choose not to type in bad tracks */
    else
      {
        printf("y\n");                        /* choose to type in bad tracks */
        new_bad = head_bad;
        do
          {
            printf("%d,%d\n", new_bad->cyl, new_bad->track);
            new_bad = new_bad->next_bad;
          }
        while ( new_bad != 0 );
        printf("-1,-1\n");                  /* equivalent to ctrl-D for fdisk */
      }
    printf("x\n");                                    /* return to UNIX level */
    printf("y\n");                                         /* install changes */
    wait(&status);

    tty = open("/dev/tty", O_WRONLY);
    if ( tty == -1 )
    {
      fprintf(stderr, "can't open /dev/tty\n");
      exit(1);
    }
      /* reconnect parent's output to terminal */
    close(1);
    dup(tty);
    close(tty);

		if ( status != 0 )
    {
      printf("\n\n\nfdisk failed.");
      printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
      exit(1);
    }

			/* M003 */
    printf("[H[J");
    printf("\nAn appropriate UNIX partition has been built, and the\n");
		printf("scanning of your hard disk for bad tracks is finished.\n\n");

      /* M003 */
    printf("\n\n\nThe necessary file systems will be created next.\n");
    printf("This should take no more than a few minutes.\n\n");

			/* M003 */
	  pipe(to_child);

    if ( fork() == 0 )
	  {
		   /* hook child's input to parent's output */
		  close(0);
		  dup(to_child[0]);

#ifndef RAWOUT
		   /* hook child's output to bit bucket */
	    bbucket = open("/dev/null",O_WRONLY);
  	  close(1);
		  dup(bbucket);  
		  close(bbucket);
#endif
              
		   /* close unnecessary pipe descriptors */
		  close(to_child[0]);
		  close(to_child[1]);
    
	    execlp("/etc/divvy", "divvy", (char *) 0);
	  }
              
      /* hook parent's output to child's input */
	  close(1);
	  dup(to_child[1]);
  
      /* close unnecessary pipe descriptors */
    close(to_child[0]);
    close(to_child[1]);

			/* M003 */
    printf("n\n");                               /* accept default allocation */
    printf("y\n");                                             /* update disk */
    printf("y\n");                                 /* proceed with making 0s0 */
    printf("y\n");                                 /* proceed with making 0s2 */
    wait(&status);

    tty = open("/dev/tty", O_WRONLY);
    if ( tty == -1 )
    {
      fprintf(stderr, "can't open /dev/tty\n");
      exit(1);
    }
      /* reconnect parent's output to terminal */
    close(1);
    dup(tty);
    close(tty);

		if ( status != 0 )
    {
      printf("\n\n\ndivvy failed.");
      printf("\n\n\neasyinstall has been aborted at this point.\n\n\n");
      exit(1);
    }

			/* M003 */
    printf("[H[J");
    exit(0);
}
