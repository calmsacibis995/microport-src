/* uportid = "@(#)divvy.h	Microport Rev Id  1.3.3 6/18/86" */

/*
 * Modification history
 *   M000  uport!bernie Mon Apr 20 18:46:36 PST 1987
 *     Specify define for smallest partition size (in blocks) for which default 
 *     allocations from wndefaults.h rather than divvy.h is used.
 *     Specify default configuration for "small drive".
 *     Specify defines for minimum file system allocations.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <malloc.h>
#include "sys/misc.h"
#include "sys/wn.h"

#define SLICES 16
#define MINORS 21	/* entries in minor device table */
#define MAXMAGIC 4	/* /root,/swap,/usr, and /tmp */
#define NUNITS 2	/* # of disk drives */

/* Smallest size partition (in blocks) for which default allocations */
/* from wndefaults.h are used. */
/* BIGPARTIT == ROOT + SWAP + TMP + 20000L */
#define BIGPARTIT 46000L  /* M000 */

/* Default allocations (in blocks) for partition smaller than BIGPARTIT */
#define ROOT1 12000L  /* M000 */
#define SWAP1 4000L  /* M000 */
#define TMP1 0000L   /* M000 */

/* Minimum acceptable file system allocations for primary drive in all */
/* cases and secondary drive if there is no active UNIX partition on the */
/* primary drive. */
#define ROOTMIN 12000L  /* M000 */
#define SWAPMIN 4000L  /* M000 */
#define USRMIN 0000L   /* M000 */
#define TMPMIN 0000L   /* M000 */

/* Minimum acceptable file system allocations for secondary drive if there */
/* is already an active UNIX partition on the primary drive. */
#define ROOTMIN1 0000L  /* M000 */
#define SWAPMIN1 0000L  /* M000 */
#define USRMIN1 0000L   /* M000 */
#define TMPMIN1 0000L   /* M000 */

/* this code assumes that the last sector in the active partition
on each drive contains the following structure:
*/
struct per
{
	struct i1010drtab drtab;	/*drive table*/
	struct i1010slice slice[SLICES];	/* slice table */
	unsigned miner[MINORS];	/* minor devices - drive 0 only */
	byte dummy[ 512 - (sizeof(struct i1010drtab)
		 + (sizeof(struct i1010slice) * SLICES) + (MINORS*2)) ];
};

#define PTOFFSET        0x1be   /* partition table offset */
#define SIGOFFSET       0x1fe   /* table signature offset */

#define DRTOFFSET 0     /* drive table offset */
#define SLTOFFSET DRTOFFSET+sizeof(struct i1010drtab)

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
