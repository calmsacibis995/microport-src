static char *uportid = "@(#)hd.conf.c	Microport Rev Id 2.2.  4/20/87";
/*
 * @(#)hd.conf.c	1.11
 *	iSBC 1010 Specific Configuration file.
 *
 *	if you want to use 24 bit addressing then define compile
 *	with -DBIT24
 *
 * Modification history:
 *
 *	1/7/87 lew
 *	names changed to hd-- and 286 - 386 made conditional on MP386
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/iobuf.h>
#include <sys/hd.h>
#include <sys/hddefaults.h>

/*
 *   See hd.h for descriptions of these structures.
 */

/*
 * 1010 Board Device-Table Definitions (drtab's)
 *
 *
 * Note: the fields Sec/cyl, and Sec/blk are filled in by the
 *	 initialization code.
 */


#define MAXTYPE 17
/* drive type table */
#define DRTYPE( cy,hd,pc,lz,ns ) cy,hd,0,pc,0,0,0,0,0,lz,ns,0,512,0,ns*hd,0,

/*
 * This is a dummy definition for initializing
 * unopen and non existant units
 */
struct i1010drtab i1010drdmy[] = {
1024,	10,	0,	512,	0,	0,	0,	0,	/* [0] */ 
0,	1024,	17,	0,	512,	0,	17*10,	slicer,
1024,	10,	0,	512,	0,	0,	0,	0,	/* [1] */ 
0,	1024,	17,	0,	512,	0,	17*10,	slice1,
};

/*
 * These definitions are overlaid by get_drivetable in the hard
 * disk initialization code. All drive parameters are overlaid,
 * but the slice-table pointers remain as shown here.
 *Cyls,	Fixed,	Null0,	Precomp, Null1,	Control, Null2,	Null3,
 *Null4,Lzone, Sec/trk,	Null5,	SecSiz,	Nalt,	Sec/cyl, Slices,
 */

struct	i1010drtab i1010drtab[] = {
1024,	10,	0,	512,	0,	0,	0,	0,	/* [0] */ 
0,	1024,	17,	0,	512,	0,	17*10,	slicer,
1024,	10,	0,	512,	0,	0,	0,	0,	/* [1] */ 
0,	1024,	17,	0,	512,	0,	17*10,	slice1,
};

/*
 * Bad- Track Table , 1 per drive
 * This table is overlaid by read_bad_track_table at initialization, using
 * a table from the last track of the active partition on the drive
 */

struct	bad_tracks i1010bad_track[2] = {
	{
	0xFFFF,0x07,0xFFFF,0x07,
	},
	{
	0xFFFF,0x07,0xFFFF,0x07,
	}
};
/* the partition table from the master boot block is saved here
*/
unsigned char partitab[66] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	};

/*
 * This buffer is used to read in data from the drive during initialization
 */
/* track cache buffer */
unsigned char hdcache[17][512];
struct incache inc;
/*
 * 1010 Board configuration.
 *
 * This configures ONE 1010 controller board
 *
 * Devcode indicates what kind of wini/floppies are there.
 * This should be:
 *	DEVWINI		For 1010/A/B
 *	DEVWINIG	For 1010/G
 */

struct	i1010cfg	 i1010cfg[] = {
	/* WUA,  Devcode 0,   1,     Int */
	0x01F0, DEVWINI, DEVWINI, WNINTLVL,
	0x0170, DEVWINI, DEVWINI, WNINTLVL,
};

/*
 * The following are static initialization variables
 * which are based on the configuration. These variables
 * MUST NOT CHANGE because the i1010 device driver makes
 * most of the calculations based on these variables.
 */

#define	NUM1010	((sizeof (i1010cfg)) / (sizeof (struct i1010cfg)))

/*
 * maximum minor number posible.
 */
short	i1010maxmin = ((sizeof (i1010minor)) / (sizeof (unsigned)));

int	N1010 =	NUM1010;			/* number of configured boards */
struct	iobuf	i1010tab[NUM1010];	/* buffer headers per board */
struct	buf	i1010rbuf[NUM1010];	/* raw buffer headers per board */
struct	i1010dev	*i1010bdd[NUM1010];	/* board-idx -> "dev" map */
struct	i1010dev	i1010dev[NUM1010];	/* per-board device-data-structures */
