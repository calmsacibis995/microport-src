/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*
 * @(#)wn.conf.c	1.13
 *	iSBC 215 Specific Configuration file.
 *
 *	if you want to use 24 bit addressing then define compile
 *	with -DBIT24
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/iobuf.h>
#include <sys/wn.h>


/*
 * These tables specify the wake-up blocks, and per-controller data-structures
 * for each 218/218 controller.  The slice, device-characteristics
 * are defined.  See wn.h for descriptions of these structures.
 */

/*
 * CMI 5.25 wini 11 meg.
 * Cyls 0-300 are data areas,
 * cyls 301-303 are alternates,
 * cyl 304 contains bad track data,
 * cyl 305 is for diagnostics
 */

struct	i215slice Sw11[] = {
	0,	19584,		/* [0] all the disk (512 byte sectors) */
	16,	19248,		/* [1] all the useable area (512 byte) */
	12304,	6960,		/* [2] all but 6Mb of useable (512 byte) */
	16400,	2864,		/* [3] all but 8Mb of useable (512 byte) */
	0,	0,		/* [4] un used area */
	0,	0,		/* [5] un used area */
	0,	0,		/* [6] un used area */
	0,	11016		/* [7] all the disk (1024 byte sectors) */
};

/*
 * CMI 5.25 wini 16 meg.
 * Cyls 0-300 are data areas,
 * cyls 301-303 and 1st 2 tracks of cyl 304 are alternates,
 * cyl 304 last 4 tracks contain bad track data,
 * cyl 305 is for diagnostics
 */

struct	i215slice Sw16[] = {
	0,	29376,		/* [0] all the disk (512 byte sectors) */
	16,	28880,		/* [1] all the useable area (512 byte) */
	12304,	16592,		/* [2] all but 6Mb of useable (512 byte) */
	16400,	12496,		/* [3] all but 8Mb of useable (512 byte) */
	0,	0,		/* [4] un used area  */
	0,	0,		/* [5] un used area  */
	0,	0,		/* [6] un used area */
	0,	16524		/* [7] all the disk (1024 byte sectors) */
};

/*
 * Quantum 5.25 wini 40 meg.
 * Cyls 0-506 are data areas,
 * cyls 507-509 and 1st 4 tracks of cyl 510 are alternates,
 * cyl 510 last 4 tracks contains bad track data,
 * cyl 511 is for diagnostics
 */

struct	i215slice Sw40[] = {
	0,		65536,	/* [0] all the disk (512 byte sectors) */
	16,		64880,	/* [1] all the useable area (512 byte) */
	12304,		52592,	/* [2] all but 6Mb useable area (512 byte) */
	16400,		48496,	/* [3] all but 8Mb useable area (512 byte) */
	47120,		17776,	/* [4] all but 23Mb useable area (512 byte) */
	53264,		11632,	/* [5] all but 26Mb useable area (512 byte) */
	59408,		5488,	/* [6] all but 29Mb useable area (512 byte) */
	0,		36864,	/* [7] all the disk (1024 byte sectors) */
};

/*
 * 5 1/4 floppy 1 side 128 byte sectors
 */
struct i215slice Sf128s26[] = {
	0,	1040		/* [0] all the disk */
};

/*
 * 5 1/4 floppy 1 side 256 byte sectors
 */
struct i215slice Sf256s16[] = {
	0,	640,		/* [0] all the disk */
	32,	608		/* [1] the user area */
};

/*
 * 5 1/4 floppy 2 side 256 byte sectors
 */
struct i215slice Sf256d16[] = {
	0,	1280,		/* [0] all the disk */
	32,	1280		/* [1] the user area */
};

/*
 * 5 1/4 floppy 1 side 512 byte sectors
 */
struct i215slice Sf512s8[] = {
	0,	320,		/* [0] all the disk */
	16,	304		/* [1] the user area */
};

/*
 * 5 1/4 floppy 2 side 512 byte sectors
 */
struct i215slice Sf512d8[] = {
	0,	640,		/* [0] all the disk */
	16,	624		/* [1] the user area */
};

/*
 * 5 1/4 floppy 1 side 512 byte sectors
 */
struct i215slice Sf512s9[] = {
	0,	360,		/* [0] all the disk */
	18,	342		/* [1] the user area */
};

/*
 * 5 1/4 floppy 2 side 512 byte sectors
 */
struct i215slice Sf512d9[] = {
	0,	720,		/* [0] all the disk */
	18,	702		/* [1] the user area */
};

/*
 * 5 1/4 floppy 1 side 1024 byte sectors
 */
struct i215slice Sf1024s4[] = {
	0,	160,		/* [0] all the disk */
	4,	156		/* [1] the user area */
};

/*
 * 5 1/4 floppy 2 side 1024 byte sectors
 */
struct i215slice Sf1024d4[] = {
	0,	320,		/* [0] all the disk */
	4,	316		/* [1] the user area */
};

/*
 * 215 Board Device-Table Definitions (drtab's)
 *
 * Note: the descriptions that sets nalt == 0 are to allow
 *	 access to the bad-track data which is beyond the alternate
 *	 tracks.  The user must be careful not to overwrite this
 *	 data unintentionally (ie, via format).
 *
 * Note: the fields Sec/cyl, and Sec/blk are filled in by the
 *	 initialization code.
 */

struct	i215drtab i215drtab[] = {
/*Cyls, Fixed, Remov,  #Sec,  SecSiz,  Nalt, Sec/cyl,	Slices		entry Drive-Type */
  306,	4,	0,	16,	512,	0,	64,	Sw11,		/* [0] CMI 5.25" 11M */
  306,	4,	0,	9,	1024,	0,	36,	Sw11,		/* [1] CMI 5.25" 11M */
  306,	6,	0,	16,	512,	0,	96,	Sw16,		/* [2] CMI 5.25" 16M */
  306,	6,	0,	9,	1024,	0,	54,	Sw16,		/* [3] CMI 5.25" 16M */
  512,  8,	0,	16,	512,	0,	128,	Sw40,		/* [4] Quantum 5.25" 40M */
  512,  8,	0,	9,	1024,	0,	72,	Sw40,		/* [5] Quantum 5.25" 40M */
  40,	0,	1,	16,	128,   FLPY_FM,	16,	Sf128s26,	/* [6] 5.25" SS/SD 128 byte */
  40,	0,	1,	16,	256,   FLPY_MFM,16,	Sf256s16,	/* [7] 5.25" SS/DD 256 byte */
  40,	0,	2,	16,	256,   FLPY_MFM,32,	Sf256d16,	/* [8] 5.25" DS/DD 256 byte */
  40,	0,	1,	8,	512,   FLPY_MFM,8,	Sf512s8,	/* [9] 5.25" SS/DD 512 byte */
  40,	0,	2,	8,	512,   FLPY_MFM,16,	Sf512d8,	/* [10] 5.25" DS/DD 512 byte */
  40,	0,	1,	9,	512,   FLPY_MFM,9,	Sf512s9,	/* [11] 5.25" SS/DD 512 byte */
  40,	0,	2,	9,	512,   FLPY_MFM,18,	Sf512d9,	/* [12] 5.25" DS/DD 512 byte */
  40,	0,	1,	4,	1024,  FLPY_MFM,4,	Sf1024s4,	/* [13] 5.25" SS/DD 1024 byte */
  40,	0,	2,	4,	1024,  FLPY_MFM,8,	Sf1024d4,	/* [14] 5.25" DS/DD 1024 byte */
};

/*
 * This is a dummy definition for initializing
 * unopen and non existant units
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
	i215MINOR(0,0,3,7),			/* [0] 16M wini slice 7 */
	i215MINOR(0,0,5,7),			/* [1] 40M wini slice 7 */
	i215MINOR(0,0,1,7),			/* [2] 11M wini slice 7 */
	i215MINOR(0,0,0,1),			/* [3] 11M wini slice 1 */
	i215MINOR(0,0,0,3),			/* [4] 11M wini slice 3 */
	i215MINOR(0,0,2,0),			/* [5] 16M wini slice 0 */
	i215MINOR(0,0,2,1),			/* [6] 16M wini slice 1 */
	i215MINOR(0,0,2,3),			/* [7] 16M wini slice 3 */
	i215MINOR(0,0,2,5),			/* [8] 16M wini slice 5 */
	i215MINOR(0,0,4,0),			/* [9] 40M wini slice 0 */
	i215MINOR(0,0,4,1),			/* [10] 40M wini slice 1 */
	i215MINOR(0,0,4,3),			/* [11] 40M wini slice 3 */
	i215MINOR(0,0,4,4),			/* [12] 40M wini slice 4 */
	i215MINOR(0,0,4,5),			/* [13] 40M wini slice 5 */
	i215MINOR(0,0,4,6),			/* [14] 40M wini slice 6 */
	i215MINOR(0,4,6,0),			/* [15] 128 byte SS floppy */
	i215MINOR(0,4,8,0),			/* [16] 256 byte DS floppy */
	i215MINOR(0,4,8,1),			/* [17] 256 byte DS floppy */
	i215MINOR(0,4,7,0),			/* [18] 256 byte SS floppy */
	i215MINOR(0,4,7,1),			/* [19] 256 byte SS floppy */
	i215MINOR(0,4,10,0),			/* [20] 512 byte DS floppy */
	i215MINOR(0,4,10,1),			/* [21] 512 byte DS floppy */
	i215MINOR(0,4,9,0),			/* [22] 512 byte SS floppy */
	i215MINOR(0,4,9,1),			/* [23] 512 byte SS floppy */
	i215MINOR(0,4,12,0),			/* [24] 512 byte DS floppy */
	i215MINOR(0,4,12,1),			/* [25] 512 byte DS floppy */
	i215MINOR(0,4,11,0),			/* [26] 512 byte SS floppy */
	i215MINOR(0,4,11,1),			/* [27] 512 byte SS floppy */
	i215MINOR(0,4,14,0),			/* [28] 1024 byte DS floppy */
	i215MINOR(0,4,14,1),			/* [29] 1024 byte DS floppy */
	i215MINOR(0,4,13,0),			/* [30] 1024 byte SS floppy */
	i215MINOR(0,4,13,1),			/* [31] 1024 byte SS floppy */
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
	/* WUA,  Devcode 0,   1,     Int */
#ifdef BIT24
	0x01000L, DEVWINIG, DEV5FLPY, 0x25,
#else
	0x01000L, DEVWINI, DEV5FLPY, 0x25,
#endif
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
