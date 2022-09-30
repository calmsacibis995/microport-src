static char *uportid = "@(#)divvy.c	Microport Rev Id  2.3 5/29/87";
/*
	DIVVY divides up the physical disk partition(s) allocated
	to System 5 into logical devices /root,/swp,/user,/tmp
	which in turn map to preassigned minor device numbers
	on the hard disk.
*/

/*
 * Modification history
 *   M001  uport!bernie Thu Nov 20 19:10:59 PST 1986
 *         Added tolerance of unexpected responses to query() and getno().
 *   M002  uport!bernie Thu Mar 26 19:09:12 PST 1987
 *         Check that active partition is a UNIX partition.
 *         Add -d option for displaying contents of partition end record.
 *   M003  uport!bernie Fri Apr 3 18:54:56 PST 1987
 *         Add 0 and 1 options for working exclusively with drives 0 or 1
 *         respectively.
 *   M004  uport!bernie Fri Apr 3 18:55:06 PST 1987
 *         Add -u option for updating partition end record with changes
 *         in partition table only.
 *   M005  uport!bernie Mon Apr 13 13:11:50 PST 1987
 *         Show file system allocations, even if zero.
 *         Make information about /usr appear last.
 *   M006  uport!bernie Tue Apr 21 14:41:56 PST 1987
 *         Simplified reallocation algorithm.
 *         Require different minimum allocations for secondary drive 
 *         only if there is already an active UNIX partition on the
 *         primary drive.
 *   M007  uport!bernie Thu May 14 17:54:55 PST 1987
 *         Modified assignment of values to start field of the first four
 *         slices so that physically /tmp is the next to the last of the
 *         four slices and /usr is the last of the four.  This is in 
 *         conformity with the scheme use in wndefaults.h for the default
 *         slice table.
 *         Set reinitialization for chosen_unit.
 *   M008  uport!rex	Tue Sep  1 15:47:14 PDT 1987
 *         Fixed problem updating slice table if beginning of partition
 *         changed but PER was still found.
 */

#include "sys/divvy.h"
#include <ctype.h>
#define STARS "*********************************************************************"
#include "sys/cmos.h"
#define BITCH(s1,s2)  printf("\n%s\n  %s%s\n%s\n\007\007",STARS,s1,s2,STARS)
#define FILE path[unit]
#define UNCYL(pt)  (pt->ecyl | (((int)(pt->esect & 0xC0)) << 2))

long atol();
char *path[NUNITS] = { "/dev/rdsk/0s0","/dev/rdsk/1s0" };
char *names[MAXMAGIC] = 
{
	"/root",
	"/swap",
	"/usr",
	"/tmp"
};	
char *tokens[4];
int last;
struct per pers[NUNITS];	/* per-unit PER table */

/* M003   #define i1010minor pers[0].miner */
#define i1010minor pers[chosen_unit].miner
#define SIZE(i) pers[UNIT(i)].slice[SLICE(i)].p_nsec
#define START(i) pers[UNIT(i)].slice[SLICE(i)].p_fsec
#define SPC(i)  pers[UNIT(i)].drtab.dr_spc
struct partit *part[NUNITS];	/* per unit active partition pointers */
dev_t fd[NUNITS];

int     bps = 512;    /* bytes / sector */
unsigned int endcyl;

int *sig, otto, dumpPER, active_primary, change_alloc;  /* M002 */  /* M006 */
unsigned int chosen_unit;   /* M003 */
int offset;  /* M003 */
int update;   /* M004 */
byte *er,*bt,*bad, savehead, buf0[512], buf1[512];
byte *tabbuf[2] = { buf0,  buf1 } ;
long newsize[MAXMAGIC], oldsize[MAXMAGIC]; /* M006 */

/************************************************************************/
/*	START EXECUTION HERE !!!!                                       */
/************************************************************************/

main(argc,argv)
int argc;
char **argv;
{
	char c;
	int i, numeric_arg;  /* M003 */

  numeric_arg = -1;   /* M003 */
	chosen_unit = 0;
	otto = 0;
	dumpPER = 0;  /* M002 */
	update = 0;  /* M004 */

	for (i = 1; i < argc; i++)  /* M003 */
	{
		if ( isdigit(*argv[i]) )
		{
	 	  numeric_arg = atoi(argv[i]);
		  if ( numeric_arg >= 0  && numeric_arg < NUNITS )
			  chosen_unit = numeric_arg;
	  }
		else
		{
		  c = *(argv[i] + 1);
		  switch (c)   /* M002 */
		  {
			  case 'a':    /*	check for -a (otto-initialize) option	*/
		      otto = 1;
				  break;
			  case 'd': /* only display contents of partition end records   M002 */
		      dumpPER = 1;
				  break;
			  case 'u':   /* M004 */
		      update = 1;
				  break;
      }
	  }
	}

	/* Since the minor device table contains data about both drives, it   */
	/* is necessary to use an offset to direct the macros to the part of  */
	/* the table that refers to the drive in question.     M003           */
	if ( chosen_unit == 0 )
		offset = 0;
	else   /* chosen_unit must be 1 */
		offset = 20;

  /* read the PERs for chosen_units */
  active_primary = 0;
	if ( chosen_unit == 0 )  /* M003 */
	{
	  if (init_unit(0)) exit(1);
	}
	else   /* chosen_unit must be 1 */  /* M003 */
	{
	  readcmos();
	  if ( cmos.disk & 0xF) 
		  if ( (active_primary = -chkprimary()) < 0  ||  init_unit(1) ) 
				/* if cannot find partition table for primary drive
					 or cannot initialize secondary drive, ... */
        /* M003 */  /* M006 */ 
				exit(1);
	}

  if ( dumpPER )   /* M002 */
	{
	  close (fd[chosen_unit]);
		exit(0);
	}

	change_alloc = 0;  /* M006 */
  if ( !update )   /* M004 */
	  while ( menu() );

	if ( write_PER(chosen_unit) )	/*write all PERS back to disk */  /* M003 */
	{
	  /*  reinitialize to update handler tables in core */
		printf(" the hard disk will now be reinitialized\n");
		sync(); sync();
		while (i= ioctl(fd[chosen_unit],I1010_REINIT,0))  /* M007 */
		{
		  printf("Disk still busy. Reboot!\n!");
		  sleep(2);
		};
	  close (fd[chosen_unit]);  /* M007 */
	};

  if ( !update )   /* M004 */
	{
	  /* compute mkfs for /root (0),/usr(2),/tmp(3)	*/
	  make_mkfs(0);
	  make_mkfs(2);
	  make_mkfs(3);
	}

	exit(0);
}

/****************************************************************************/
/*  print menu and accept changes	*/

menu()   /* M006 */
{
	char bitcher[80],string[80],*p,*gets();
	int i, nunit;
	long xx, yy, min;

	printf("\n\n\n\tDEVICE\tUNIT\tBLOCKS\n\n");

	for (i = offset; i < (offset + MAXMAGIC); i++)
	{
		newsize[i - offset] = SIZE(i);
		if ( !change_alloc )
		  oldsize[i - offset] = SIZE(i);
	  if( *names[i - offset]  &&  i != offset + 2 )  /* M005 */
	    printf("\t%s\t%d\t%ld\n\n",names[i - offset],UNIT(i),SIZE(i));
	};
    /* M005 */
	printf("\t%s\t%d\t%ld\n\n",names[2],UNIT(offset + 2), SIZE(offset + 2));
	printf("  NOTE that all unassigned blocks are automatically allocated\n");
	printf("  to /usr and that its allocation cannot be DIRECTLY modified.\n\n");
		
	if (otto || (!query("Do you wish to change any allocation?")))
	{
	  /* get number of blocks to be assigned to /root, /swap, and /tmp */
	  xx = 0;
	  for ( i = 0; i < MAXMAGIC ; i++ )
	    if( i != 2 )
		    xx += SIZE(i + offset);

	  yy = ( part[chosen_unit]->no_sects - ((ALTTRACKS) 
					 * pers[chosen_unit].drtab.dr_nsec)) - xx;  /* M003 */
	  if ( chosen_unit == 0 || active_primary )   
		  /* M006 */ 
		  /* If working on primary drive or else no */
		  /* active UNIX partition on primary drive ... */
	  {
	    if ( yy < USRMIN )
      { 
			  min = USRMIN;
		    printf("\n%s\n", STARS);
		    printf("  The requested allocations would result in a /usr file \n");
		    printf("  system of %ld blocks.  /usr requires at least %ld blocks.\n", 
							  yy, min);
		    printf("  Please try again.\n");
		    printf("%s\n\007\007", STARS);

				/* reinitialize SIZES with last set of fully validated allocations */
	      for (i = offset; i < (offset + MAXMAGIC); i++)
		      SIZE(i) = oldsize[i - offset];

					/* M007 */
	      START(0 + offset) = part[chosen_unit]->rel_sect;
	      START(1 + offset) = START(0 + offset) + SIZE(0 + offset);
	      START(3 + offset) = START(1 + offset) + SIZE(1 + offset);
	      START(2 + offset) = START(3 + offset) + SIZE(3 + offset);

		    return(1);
	    }
	  }
	  else
	    /* M006 */
	    /* Else working on secondary drive and there is */ 
	    /* an active UNIX paritition on the primary drive */
	  {
	    if ( yy < USRMIN1 )
      { 
			  min = USRMIN1;
		    printf("\n%s\n", STARS);
		    printf("  The requested allocations would result in a /usr file \n");
		    printf("  system of %ld blocks.  /usr requires at least %ld blocks.\n", 
							  yy, min);
		    printf("  Please try again.\n");
		    printf("%s\n\007\007", STARS);

				/* reinitialize SIZES with last set of fully validated allocations */
	      for (i = offset; i < (offset + MAXMAGIC); i++)
		      SIZE(i) = oldsize[i - offset];

					/* M007 */
	      START(0 + offset) = part[chosen_unit]->rel_sect;
	      START(1 + offset) = START(0 + offset) + SIZE(0 + offset);
	      START(3 + offset) = START(1 + offset) + SIZE(1 + offset);
	      START(2 + offset) = START(3 + offset) + SIZE(3 + offset);

		    return(1);
	    }
	  }
		return (0);
	}
	else
		change_alloc = 1;

reget:
	printf("    Enter Device,Unit,Size in 512-byte blocks:");
	p = gets(string);
	if (p=NULL) 
		return (1);
	if ( parse(string,tokens,", " ) != 3 ) 
	{
		printf("\n  You did not enter exactly three items. \n");
		goto reget;
	}
	for ( i = 0 ; i < MAXMAGIC ; i++ )
	{
		if ( strcmp(names[i],tokens[0]) == 0 )
		{
			if ( (nunit=atoi(tokens[1])) != chosen_unit )  /* M003 */
			{
		    printf("\n%s\n", STARS);
				if ( chosen_unit == 0 )
				{
					printf("  You are currently allocating file system space on only your \n");
					printf("  primary drive (unit 0).  If you have already allocated file \n");
					printf("  system space on your primary drive and instead wish to do \n");
					printf("  this on your secondary drive (unit 1), delete out of this \n");
					printf("  program, and execute \"divvy 1\".\n");
				}
				else
				{
					printf("  You are currently allocating file system space on only your \n");
					printf("  secondary drive (unit 1).  If you instead wish to do this on \n");
					printf("  your primary drive (unit 0), delete out of this program, \n");
					printf("  and execute \"divvy\".\n");
				}
		    printf("%s\n\007\007", STARS);
			  return(1);
			};

			if ( chosen_unit == 0  ||  active_primary == 0 )  
				/* M006 */ 
				/* If working on primary drive or else no */
				/* active UNIX partition on primary drive ... */
			{
			  if ( i == 0  &&  atol(tokens[2]) < ROOTMIN )
			  {
					min = ROOTMIN;
				  sprintf(bitcher, " must be allocated at least %ld blocks", min);
				  BITCH(names[i], bitcher);
				  return(1);
			  };
			  if ( i == 1  &&  atol(tokens[2]) < SWAPMIN )
			  {
					min = SWAPMIN;
				  sprintf(bitcher, " must be allocated at least %ld blocks", min);
				  BITCH(names[i], bitcher);
				  return(1);
			  };
			  if ( i == 3  &&  atol(tokens[2]) < TMPMIN )
			  {
					min = TMPMIN;
				  sprintf(bitcher, " must be allocated at least %ld blocks", min);
				  BITCH(names[i], bitcher);
				  return(1);
			  };
			}
			else  
			/* M006 */
			/* Else working on secondary drive and there is */ 
			/* an active UNIX paritition on the primary drive */
			{
			  if ( i == 0  &&  atol(tokens[2]) < 0 )
			  {
					min = ROOTMIN1;
				  sprintf(bitcher, " must be allocated at least %ld blocks", min);
				  BITCH(names[i], bitcher);
				  return(1);
			  };
			  if ( i == 1  &&  atol(tokens[2]) < 0 )
			  {
					min = SWAPMIN1;
				  sprintf(bitcher, " must be allocated at least %ld blocks", min);
				  BITCH(names[i], bitcher);
				  return(1);
			  };
			  if ( i == 3  &&  atol(tokens[2]) < 0 )
			  {
					min = TMPMIN1;
				  sprintf(bitcher, " must be allocated at least %ld blocks", min);
				  BITCH(names[i], bitcher);
				  return(1);
			  };
			}

			if ( i == 2 )
			{
				BITCH(names[i],"\'s allocation cannot be DIRECTLY modified");
				return(1);
			};

			newsize[i] = atol(tokens[2]);
			break;
		};
	};

	if (i == MAXMAGIC) 
	{
		BITCH(tokens[0]," is not a valid device name");
		return(1);
	}

	/* get number of blocks to be assigned to /root, /swap, and /tmp */
	xx = 0;
	for ( i = 0; i < MAXMAGIC ; i++ )
	  if( i != 2 )
		  xx += newsize[i];

	yy = ( part[chosen_unit]->no_sects - ((ALTTRACKS) 
				 * pers[chosen_unit].drtab.dr_nsec)) - xx;  /* M003 */
	newsize[2] = yy;
  for ( i = 0; i < MAXMAGIC ; i++ )
    SIZE(i + offset) = newsize[i];

			/* M007 */
  START(0 + offset) = part[chosen_unit]->rel_sect;
  START(1 + offset) = START(0 + offset) + SIZE(0 + offset);
  START(3 + offset) = START(1 + offset) + SIZE(1 + offset);
  START(2 + offset) = START(3 + offset) + SIZE(3 + offset);

				/* update minor device table entry */
		/*
			pers[0].miner[i] &=i1010MINOR(3,0,0x3f,0xf);
			pers[0].miner[i] |=i1010MINOR(0,nunit,0,0);
		*/

	return(1);
}

/****************************************************************************/

/*  compute and do a mkfs for the given device */

make_mkfs(dev)
{
	char cmd[40];
	long blox,inodes;
	blox = SIZE(dev + offset);  /* M003 */
	if (blox > 0)
	{
		inodes = blox/10;
		sprintf(cmd,"mkfs /dev/dsk/%ds%d %ld:%ld  2  %d\n",
			UNIT(dev + offset),dev,blox,inodes,SPC(dev + offset));  /* M003 */
		printf("\nReady to make the following file system:\n\t");
		printf(cmd);
		if (otto || query("\nShall I proceed?")) 
		{
#ifdef DEBUG
		  return;
#endif
			system (cmd);
		};
	};	
}

/****************************************************************************/

/* Parse a string into delimited substrings */

parse(string,tokens,delimiters)
char *string,*tokens[],*delimiters;
{
	int i;
	char *p,*s;
	s = string;
	i = 0;
	while ( p != NULL )
	{
		p = strtok(s,delimiters);
		tokens[i++] = p;
		s = NULL;
	};
	return (--i);
}
/****************************************************************************/


/*  Initialize unit (drive) */

init_unit(unit)
  int unit;
{
	int i;
	struct partit *pt;
	struct partit *pp,*dp;   /* M004 */

	if ((fd[unit] = open(FILE,2)) == -1) 
	{
		printf("Can't open %s, errno=%d\n", FILE, errno);
		return(errno);
	}

	read(fd[unit],tabbuf[unit],512) ; /* dummy read to force wnsweep */

	if (read_sector(fd[unit], tabbuf[unit], 0,0,1) )
	{
		printf("Can't read %s, errno=%d\n", FILE, errno);
		return(errno);
	}
			 /* partition table pointer */
  pt = (struct partit *) (tabbuf[unit] + PTOFFSET); 
  for (i= 1; i<=4 ; i++)
  {
    if (pt++->boot_ind == 0x80) 
			break;
  };

  if ( i>4 )  /* no active partition yet */
	{
		printf("No active partition!\n");
		printf(" Partition table must be initialized before DIVVY !\n");
		return(1);
	}
	else 
		pt--;	/* backup to active partition */

	if ( pt->syst_ind != 5 &&	/* M002 */
	     pt->syst_ind != 0x52 )
	{
	  close (fd[unit]);
		return (2);
	}
	part[unit] = pt;	/* partition table pointer for drive */

  er = (byte *) &pers[unit];
  /* read end-of-partition record -if any - to get drtabs,
	   slice table, and pointer to first bad-track sector  */

	/* unkludge cylinder */
	endcyl = UNCYL( pt);
	/* partition endrecord -includes drvtab   */
  read_sector (fd[unit],er,endcyl,pt->ehead,pt->esect&0x1f);

  sig  = (int *)(er + SIGOFFSET);
  if (*sig != 0xAA55)
	{
		printf ("\nInvalid Partition End Record on unit %d !!\007\n",unit);
		exit(1);
	};

  if ( dumpPER )  /* M002 */
  {
/*  printf("[H[J");  */
    printf("\n\n CONTENTS OF PARTITION END RECORD FOR UNIT #%d\n", unit);
  
		printf("\n                 Drive Table");
		printf("\n                 ----- -----");
    printf("\n         Number of cylinders:  %d", pers[unit].drtab.dr_ncyl);
    printf("\n         Number of heads/cylinder:  %d", 
						pers[unit].drtab.dr_nfhead);
    printf("\n         Landing zone:  %d", pers[unit].drtab.dr_lzone);
    printf("\n         Write precomp:  %d", pers[unit].drtab.dr_precomp);
    printf("\n         Sectors/track:  %d", pers[unit].drtab.dr_nsec);
    printf("\n         Sector size:  %d", pers[unit].drtab.dr_secsiz);
    printf("\n         Number of alternate cylinders:  %d", 
						pers[unit].drtab.dr_nalt);
    printf("\n         Actual sectors/cylinder:  %d", pers[unit].drtab.dr_spc);
    printf("\n         DOS disk control byte:  %d", 
						pers[unit].drtab.dr_control);
    printf("\n         DOS compatible null 0:  %d", pers[unit].drtab.dr_null0);
    printf("\n         DOS compatible null 1:  %d", pers[unit].drtab.dr_null1);
    printf("\n         DOS compatible null 2:  %d", pers[unit].drtab.dr_null2);
    printf("\n         DOS compatible null 3:  %d", pers[unit].drtab.dr_null3);
    printf("\n         DOS compatible null 4:  %d", pers[unit].drtab.dr_null4);
    printf("\n         DOS compatible null 5:  %d", pers[unit].drtab.dr_null5);
    printf("\n         Slice table pointer:  %d\n", pers[unit].drtab.dr_slice);
     
		printf("\n                 Slice Table");
		printf("\n                 ----- -----");
    printf("\nSlice 0 ROOT -- first sector:  %ld, number of sectors:  %ld", 
      pers[unit].slice[0].p_fsec, pers[unit].slice[0].p_nsec);
    printf("\nSlice 1 SWAP -- first sector:  %ld, number of sectors:  %ld",
      pers[unit].slice[1].p_fsec, pers[unit].slice[1].p_nsec);
    printf("\nSlice 2 USR -- first sector:  %ld, number of sectors:  %ld",
      pers[unit].slice[2].p_fsec, pers[unit].slice[2].p_nsec);
    printf("\nSlice 3 TMP -- first sector:  %ld, number of sectors:  %ld",
      pers[unit].slice[3].p_fsec, pers[unit].slice[3].p_nsec);
    printf("\nSlice 4 Reserved -- first sector:  %ld, number of sectors:  %ld",
      pers[unit].slice[4].p_fsec, pers[unit].slice[4].p_nsec);
    printf
			("\nSlice 5 DOS partition -- first sector: %ld, number of sectors: %ld",
      pers[unit].slice[5].p_fsec, pers[unit].slice[5].p_nsec);
    printf
	    ("\nSlice 6 UNIX partition #1 -- first sector: %ld, number of sectors: %ld",
      pers[unit].slice[6].p_fsec, pers[unit].slice[6].p_nsec);
    printf
	    ("\nSlice 7 UNIX partition #2 -- first sector: %ld, number of sectors: %ld",
      pers[unit].slice[7].p_fsec, pers[unit].slice[7].p_nsec);
    printf
	    ("\nSlice 8 UNIX partition #3 -- first sector: %ld, number of sectors: %ld",
      pers[unit].slice[8].p_fsec, pers[unit].slice[8].p_nsec);
    printf
	    ("\nSlice 9 UNIX partition #4 -- first sector: %ld, number of sectors: %ld",
      pers[unit].slice[9].p_fsec, pers[unit].slice[9].p_nsec);
    printf("\nSlice 10 Entire disk -- first sector: %ld, number of sectors: %ld",
      pers[unit].slice[10].p_fsec, pers[unit].slice[10].p_nsec);
    printf
	    ("\nSlice 11 Last track active pt -- first sector: %ld, %s: %ld",
      pers[unit].slice[11].p_fsec, "number of sectors", 
		  pers[unit].slice[11].p_nsec);
    printf("\n");
  
		printf("\n             Minor Device Table");
		printf("\n             ----- ------ -----");
		printf("\n  Note that the Winchester driver ONLY uses the information");
		printf("\n  stored in the minor device table of the partition end");
		printf("\n  record of the primary drive (unit 0).\n");
    for (i = 0;i < 12; ++i)
      printf("\n    i1010minor[%d] (unit 0, slice %d):  %d", 
							i, i, pers[unit].miner[i]);
    for (i = 12; i < 20; ++i)
      printf("\n    i1010minor[%d] (reserved):  %d", i, pers[unit].miner[i]);
    for (i = 20;i < 32; ++i)
      printf("\n    i1010minor[%d] (unit 1, slice %d):  %d", 
						 i, (i - 20), pers[unit].miner[i]);
	  putchar('\n');
	  return(0);
  }
  else   /* M002 */
  {
    if ( update )   /* M004 */
	  {
        /* update slice 5 (DOS)  */
	    pp = (struct partit *) (tabbuf[unit] + PTOFFSET);
	    dp = 0;
	    for (i = 0 ; i< 4; i++) 
	    {
		    if ( pp->syst_ind == 0x01  ||  pp->syst_ind == 0x04 ) 
					dp = pp;
		    pp++;
	    }
	    if (dp)
	    {
	      pers[unit].slice[5].p_fsec = dp->rel_sect;
	      pers[unit].slice[5].p_nsec = dp->no_sects;
	    }
	    else
	    {
	      pers[unit].slice[5].p_fsec = 0;
	      pers[unit].slice[5].p_nsec = 0;
	    }
  
        /* update slices 6 - 9 (the partitions) */
	    dp = (struct partit *) (tabbuf[unit]+PTOFFSET);  
	    for (i = 4 ; i> 0; i--) 
	    {
		    pers[unit].slice[5+i].p_fsec = dp->rel_sect;
		    pers[unit].slice[5+i].p_nsec = dp++->no_sects;
	    }
		  return(0);
	  }

		for (i=9; i > 5; i--)		/* Start M008 */
			if (pers[unit].slice[i].p_fsec == pers[unit].slice[0].p_fsec)
			{
				if (pers[unit].slice[i].p_nsec != pt->no_sects ||
						pers[unit].slice[i].p_fsec != pt->rel_sect ||
						pers[unit].slice[0].p_fsec != pt->rel_sect)
					*(sig-1) = 0;			/* Force PER init */
				break;
			}											/* End M008 */
  
#ifndef DEBUG
    if((*(sig-1) != 0xAA55) || otto )
#endif
	    init_PER(unit);
  }
  
  return(0);
}

/****************************************************************************/

     /* M006 check if primary drive already has active UNIX parition */

chkprimary()
{
	int i, unit;
	struct partit *pt;
	struct partit *pp,*dp;   /* M004 */

  unit = 0;

	if ((fd[unit] = open(FILE,2)) == -1) 
	{
		printf("Can't open %s, errno=%d\n", FILE, errno);
		return(errno);
	}

	read(fd[unit],tabbuf[unit],512) ;	/* dummy read to force wnsweep */

	if (read_sector(fd[unit], tabbuf[unit], 0,0,1) )
	{
		printf("Can't read %s, errno=%d\n", FILE, errno);
		return(errno);
	}

  close (fd[unit]);

		/* partition table pointer */
  pt = (struct partit *) (tabbuf[unit] + PTOFFSET); 
  for ( i= 1; i <= 4 ; i++ )
    if (pt++->boot_ind == 0x80) 
			 break;

  if ( i > 4 )  /* no active partition yet */
		return (0);
	else 
		pt--;	/* backup to active partition */

	if ( pt->syst_ind != 5 &&    /* M002 */
	     pt->syst_ind != 0x52 )
		return (0);

	return (-1);
}

/****************************************************************************/

	/* write updated partition end record back to disk */
write_PER(unit)  /* M003 */
int unit;  /* M003 */
{
	int i, wrote;
	struct	partit *pt;
	wrote = 0;
  pt = part[unit];
  printf("\nReady to write unit #%d - ", unit);   /* M003 */
  if (otto || query("Shall I update the disk?"))
  {
		wrote++;
   	write_sector (fd[unit],&pers[unit],
			UNCYL(pt),pt->ehead,pt->esect&0x1f );
	};
	return (wrote);
}

/****************************************************************************/

/* 
 * Prompt for a number and return it 
 */

getno(string)    /* M001 */
  char *string;
  {
	  int i,j;
	  char buf[81];

		while (1)
		{
      printf("\n %s ",string);
	    if ( fgets(buf, 81, stdin) != NULL );      /* M001 */
			{
        i = sscanf(buf, "%d", &j);
        if (i == 1) 
	        return(j);
			} 
			printf("\n Please reply with an integer.");
		}
  }

/****************************************************************************/

/*
 *  moveb  (from,to,length) moves a structure 
 *
*/
 byte *moveb (from, to, length )
 byte *from,*to;
 int length;
 {
	 for ( ;length;length -- ) 
		 *to++ = *from++ ;
	 return (to);
}

/****************************************************************************/

/* read or write a sector by disk address (cyl,head,sector)     */
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
	return(i);
}
/**************************************************************************/

write_sector(dev,buf,cyl,head,sector)
dev_t dev;
byte *buf,head,sector;
int cyl;
{
	int i;
	struct i1010iopb *io, iopb;

#ifdef DEBUG
  return(0);
#endif
	io = &iopb;
	io->i_addr = (long) buf;
	io->i_actcylinder = cyl;
	io->i_acthead = head;
	io->i_sector = sector;
	io->i_funct = WD_WRITE_OP;
	i = ioctl(dev,I1010_RAWIO,io);
/*	if (i) printf("ioctl failed, error no: %d \n",errno);
*/
}

/**************************************************************************/

/* query user and return 1 if yes, 0 if n*/

query(string)   /* M001 */
char *string;
{
	char tstr[81], *tp;				/* M000 */

	while(1) 
	{
		printf("\n%s (y or n): ",string);
    tp = fgets(tstr, 81, stdin);  /* M001 */
		if (tp==NULL) 
			return(0);
		switch (*tp) 
			{
     		case 'Y':
     		case 'y':
     			return(1);
     		case 'N':
     		case 'n':
     			return(0);
		    default: /* M001 */
				  printf("\nPlease reply with yes or no.");
				  break;
  		};
 	}
}

/****************************************************************************/

/* read what's currently in cmos. */

readcmos()
{
	unsigned int port, data, cksum, i;
	extern	errno;
	int	cmosfd;

	if ((cmosfd = open(CMOSDEV, 0)) == -1) 
	{	/* M000 */
		perror("Opening cmos device");		/* M000 */
		exit(1);
	}
	if ( !read(cmosfd, &cmos, sizeof(struct cmos))) 
	{
		perror("Reading cmos device");		/* M000 */
		exit(1);
	}
	close(cmosfd);
}
