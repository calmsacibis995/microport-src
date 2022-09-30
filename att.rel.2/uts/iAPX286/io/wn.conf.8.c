/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
 * @(#)wn.conf.8.c	1.5
 *	iSBC 215 Specific Configuration file.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/iobuf.h>
#include <sys/wn.h>


/*
 * These tables specify the wake-up blocks, and per-controller data-structures
 * for each 218/218 controller.  The slice, device-characteristics
 * are defined.  See i215.h for descriptions of these structures.
 */

/*
 * Note: MUST have 215/218 boards contiguous major numbers AND have
 *	 same 1st index in bdevsw[] & cdevsw[].
 */

int	i215fmaj = 0;				/* 1st {b,c}devsw index */

/*
 * Priam 3450 Partitions.
 * Note: Partition[0] is the entire disk (except for the diagnostic cylinder).
 *	 This includes the alternate-track cylinders and the bad-track data.
 *	 Partitions[1-3] only address cylinders 0-509.  510-519 are alternate
 *	 track cylinders.  Cylinders 520-523 contain the bad-track data.
 *	 Cylinder 524 is for diagnostics.
 */

struct	i215slice Piw0[] = {
	   0,	     524*5*12,		/* [0] whole disk (+alts & bad-track)*/
	  12,		 6000,		/* [1] "root":  6000K */
	6012,		 1188,		/* [2] "swap":  1188K */
	7200,           23400           /* [3] rest:   23400K */
};

/*
 * Pertec D8020 Partitions.
 * Note: Partition[0] is the entire disk.
 *	 Alternate-tracking not supported automatically for the Pertec.
 */

struct	i215slice Ppw0[] = {
	   0,	    466*3*12,		/* [0] whole disk */
	  12,		6000,		/* [1] "root":  6000K */
	6012,		1188,		/* [2] "swap":  1188K */
	7200,		9360		/* [3] rest:    9360K */
};

/*
 * SS/DD Floppy, 1024-byte sector Partitions.  Note: track 0 unused.
 */

struct	i215slice Pf0[] = {
	8,		608		/* [0] rest:  608K    */
};

/*
 * DS/DD Floppy, 1024-byte sector Partitions.  Note: track 0 unused.
 */

struct	i215slice Pdf0[] = {
	8,		1224		/* [0] rest:  1224K   */
};

/*
 * SS/SD Floppy, 128-byte sector Partitions.
 */

struct	i215slice Psf0[] = {
	   0,		  26,		/* [0] track 0: 26 sectors: 3.25K */
	  26,		1976		/* [1] rest:  1976 sectors: 247K  */
};

/*
 * SS/DD Floppy, 256-byte sector Partitions.  Note: track 0 unused.
 */

struct	i215slice Pxf0[] = {
	  26,		1976		/* [0] rest:  1976 sectors: 494K  */
};

/*
 * DS/DD Floppy, 256-byte sector Partitions.  Note: track 0 unused.
 */

struct	i215slice Pdxf0[] = {
	  26,		3978		/* [0] rest:  3978 sectors: 994.5K */
};

/*
 * 215 Board Device-Table Definitions (drtab's)
 * Note: the Priam description sets nalt == 0; this is to allow
 *	 access to the bad-track data which is beyond the alternate
 *	 tracks.  The user must be careful not to overwrite this
 *	 data unintentionally (ie, via format).
 */

struct	i215drtab i215drtab[] = {
/*Cyls, Fixed, Remov,  #Sec,  SecSiz,  Nalt, Sec/cyl, Partitions  entry Drive-Type */
  524,	5,	0,	12,	1024,	0,	60,	Piw0,	/* [0] Priam 3450   */
  466,	3,	0,	12,	1024,	6,	36,	Ppw0,	/* [1] Pertec D8020 */
  77,	0,	1,	8,	1024,  FLPY_MFM,8,	Pf0,	/* [2] SSDD 1024 byte */
  77,	0,	2,	8,	1024,  FLPY_MFM,16,	Pdf0,	/* [3] DSDD 1024 byte */
  77,	0,	1,	26,	128,   FLPY_FM,	26,	Psf0,	/* [4] SSSD 128 byte */
  77,	0,	1,	26,	256,   FLPY_MFM,26,	Pxf0,	/* [5] SSDD 256 byte */
  77,	0,	2,	26,	256,   FLPY_MFM,52,	Pdxf0	/* [6] DSDD 256 byte */
};

/*
 * This is a dummy definition for initializing unopen units
 */
struct	i215drtab i215drdmy[] = {
   0,	0,	0,	0,	0,	0,	0,	(struct i215slice *)0
};

/*
 * i215minor
 *
 * This structure has been added to widen the minor number information.
 *
 * This table configures the board number, slice number, drtab number
 * and unit number.
 */
unsigned i215minor[] = {

/*	i215MINOR(board#,unit#,drtab#,slice#) */
						/* [minor] device */
	i215MINOR(0,0,0,0),			/* [0] priam track 0 */
	i215MINOR(0,0,0,1),			/* [1] priam root part */
	i215MINOR(0,0,0,2),			/* [2] priam swap part */
	i215MINOR(0,0,0,3),			/* [3] priam usr  part */
	i215MINOR(0,4,2,0),			/* [4] f0 floppy */
	i215MINOR(0,4,3,0),			/* [5] df0 floppy */
	i215MINOR(0,4,4,0),			/* [6] sft0 floppy */
	i215MINOR(0,4,4,1),			/* [7] sf0 floppy */
	i215MINOR(0,4,5,0),			/* [8] xf0 floppy */
	i215MINOR(0,4,6,0),			/* [9] dxf0 floppy */
	i215MINOR(0,0,1,0),			/* [10]	pw0t0 wini(215a/b) */
	i215MINOR(0,0,1,1),			/* [11]	pw0a wini(215a/b) */
	i215MINOR(0,0,1,2),			/* [12]	pw0b wini(215a/b) */
	i215MINOR(0,0,1,3) 			/* [13]	pw0c wini(215a/b) */
};


/*
 * 215 Board configuration.
 *
 * This configures ONE 215/218 controller.
 *
 * Devcode indicates what kind of wini/floppies are there.
 * This should be:
 *	DEVWINI		For 215/A/B
 *	DEVWINIG	For 215/G
 *	DEV8FLPY	For 8" floppies on a 218
 *	DEV5FLPY	For 5.25" floppies on a 218
 */

struct	i215cfg	 i215cfg[] = {
/* WUA,	Devcode 0,   1,     Int */
0x01000L,DEVWINI, DEV8FLPY, 0x25
};

/*
 * The following are static initialization variables
 * which are based on the configuration. These variables
 * MUST NOT CHANGE because the i215 device driver makes
 * most of the calculations based on these variables.
 */

#define	NUM215	((sizeof (i215cfg)) / (sizeof (struct i215cfg)))

/*
 * maximum minor number posible.
 */
short	i215maxmin = ((sizeof (i215minor)) / (sizeof (unsigned)));

int	N215 =	NUM215;			/* number of configured boards */
struct	iobuf	i215tab[NUM215];	/* buffer headers per board */
struct	buf	i215rbuf[NUM215];	/* raw buffer headers per board */
struct	i215dev	*i215bdd[NUM215];	/* board-idx -> "dev" map */
struct	i215dev	i215dev[NUM215];	/* per-board device-data-structures */
