static char *uportid = "@(#)initPER.c	Microport Rev Id  1.3.4 6/28/86";

/* Copyright @ 1986 by Microport Systems Inc. All Rights Reserved.	*/

/*
 *  Modification history:
 *    M001 uport!bernie Fri Mar 27 12:54:49 PST 1987
 *         Corrected initialization of partition table pointer.
 *         Corrected initialization of slice for DOS partition.
 *    M002 uport!bernie Thu Apr 23 17:14:13 PST 1987
 *         Made default allocations dependent upon the size of the
 *         active partition, unit partition is on, and whether or
 *         not primary drive already has active UNIX partition.
 */

#include "sys/divvy.h"
#include "sys/wndefaults.h"
extern struct partit *part[];
extern struct per pers[];
extern byte *tabbuf[],*moveb();
extern int *sig, active_primary;   /* M002 */
/***************************************************************************/
/*  initialize partition end record with default values */
init_PER(unit)
{
	struct partit *pt;
	struct per *pert;
	struct partit *pp,*dp;
	byte *to;
	long xx, yy;  /* M002 */
	int i,spt;

	pt = part[unit]; /* M001 */
	pert = &pers[unit];
	spt = pert->drtab.dr_nsec;

/*   build /root[0], /swap[1], /usr[2], and /tmp[3] 
	by updating the defaults with the base of the active partition */

	if ( unit == 0  ||  active_primary == 0 )  /* M002 */
/* If initializing primary drive or else initializing secondary drive */
/* with no active UNIX parition already on the primary drive ... */
	{
	    if (( xx = pt->no_sects -
	      (ROOTMIN + SWAPMIN + TMPMIN + USRMIN + (ALTTRACKS*spt))) < 0 )
			/* M002 */
	    {
		  printf(
"System5 Partition on unit %d is %ld blocks smaller than needed.\n\007\007",
		      unit,-xx);
		  exit(1);
	    }
	    if ( (yy = pt->no_sects - (ALTTRACKS*spt)) >= BIGPARTIT ) /*M002*/
	    {
		slicer[0].p_nsec = ROOT;
		slicer[1].p_nsec = SWAP;
		slicer[3].p_nsec = TMP;
					/* all remaining space to /usr */
		slicer[2].p_nsec = yy - (ROOT + SWAP + TMP);
	    }
	    else if ( yy >= (ROOT1 + SWAP1 + TMP1 + USRMIN) )  /* M002 */
	    {
		slicer[0].p_nsec = ROOT1;
		slicer[1].p_nsec = SWAP1;
		slicer[3].p_nsec = TMP1;
			/* all remaining space to /usr */
		slicer[2].p_nsec = yy - (ROOT1 + SWAP1 + TMP1);
	    }
	    else   /* M002 */
	    {
		slicer[0].p_nsec = ROOTMIN;
		slicer[1].p_nsec = SWAPMIN;
		slicer[3].p_nsec = TMPMIN;
		slicer[2].p_nsec = USRMIN + xx;
		    /* all remaining space to /usr */
	    }
        }
	else
		/* initializing secondary drive with active UNIX partition */ 
		/* already on primary */
	{
	    if (( xx = pt->no_sects -
	  (ROOTMIN1 + SWAPMIN1 + TMPMIN1 + USRMIN1 + (ALTTRACKS*spt))) < 0 )
			/* M002 */
	    {
	        printf(
"System5 Partition on unit %d is %ld blocks smaller than needed.\n\007\007",
			    unit,-xx);
	        exit(1);
	    }
	    else   /* M002 */
	    {
		slicer[0].p_nsec = ROOTMIN1;
		slicer[1].p_nsec = SWAPMIN1;
		slicer[3].p_nsec = TMPMIN1;
		slicer[2].p_nsec = USRMIN1 + xx;
		/* all remaining space to /usr */
	    }
	}

		/* M002 */
	slicer[0].p_fsec = pt->rel_sect; 
	slicer[1].p_fsec = slicer[0].p_fsec + slicer[0].p_nsec;
	slicer[3].p_fsec = slicer[1].p_fsec + slicer[1].p_nsec;
	slicer[2].p_fsec = slicer[3].p_fsec + slicer[3].p_nsec;

    /* slice 5 is DOS  */
	pp = (struct partit *) (tabbuf[unit] + PTOFFSET);
	dp = 0;
	for (i = 0 ; i< 4; i++) 
	{  /* M001 */
		if ( pp->syst_ind == 0x01  ||  pp->syst_ind == 0x04 ) dp = pp;
		pp++;
	}
	if (dp)
	{
	slicer[5].p_fsec = dp->rel_sect;
	slicer[5].p_nsec = dp->no_sects;
	}
	else
	{
	slicer[5].p_fsec = 0;
	slicer[5].p_nsec = 0;
	}

/* slices 6 - 9 echo the partitions */
	dp = (struct partit *) (tabbuf[unit]+PTOFFSET);
	for (i = 4 ; i> 0; i--) 
	{
		slicer[5+i].p_fsec = dp->rel_sect;
		slicer[5+i].p_nsec = dp++->no_sects;
	};
/* slice 10 is whole disk */
	slicer[10].p_fsec = 0;
	slicer[10].p_nsec = pert->drtab.dr_spc * pert->drtab.dr_ncyl;

/* slice 11 is the last track of the active partition */
	slicer[11].p_fsec = pt->rel_sect + pt->no_sects - spt;
	slicer[11].p_nsec = spt;
	to = (byte *)pert->slice;
	to = moveb(slicer,to,sizeof(slicer));
	/* also default minor device table*/
	to = moveb(i1010minor,to,sizeof(i1010minor));
	*(sig -1) = 0xAA55;	/*validate slice and minor tables */
	return(0);
}
/****************************************************************************/
