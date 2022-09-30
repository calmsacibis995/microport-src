/* sccsid  = "@(#)hddefaults.h	1.2  1/8/88" */
/* uportid = "@(#)hddefaults.h	Microport Rev Id  1.3.3 12/17/86" */
/* Modified to comport with INTEL - ISC Minor device conventions
/* These slice tables partition each hard disk drive. They are 
 * initialized by FDISK from the partition table on the drive.
 * They may be modified from their default values by DIVVY.
 * They are read in and checked by get_slice_table
 * in hd.subs.c during hard-disk initialization.
 */
#define USR	24000L  /* this is minimum, set to remaining space */
#define TMP	0000L
#define	ROOT	20000L
#define SWAP	6000L

struct	i1010slice slicer[16] = {
	0,	USR+ROOT+SWAP+TMP,	/* [0]  entire disk	*/
	0,		ROOT,		/* [1]  /root	*/
	ROOT,		SWAP,		/* [2]	/swap	*/
	ROOT+SWAP+TMP,	USR,		/* [3]	/usr	*/
	ROOT+SWAP,	TMP,		/* [4]  /tmp	*/
	0,		0,		/* [5 -15]	reserved	*/
	0,		0,		/* for vtoc partitions 5 -15	*/
	0,		0,
	0,		0,
	0,		0,
	0,		0,		
	0,		0,	
	0,		0,	
	0,		0,	
	0,		0,	
	0,		0,	

};
/* slice table for Drive 1 */
struct	i1010slice slice1[16] = {
	0,	USR+ROOT+SWAP+TMP,	/* [0]  entire disk	*/
	0,		ROOT,		/* [1]  /root	*/
	ROOT,		SWAP,		/* [2]	/swap	*/
	ROOT+SWAP+TMP,	USR,		/* [3]	/usr	*/
	ROOT+SWAP,	TMP,		/* [4]  /tmp	*/
	0,		0,		/* [5 -15]	reserved	*/
	0,		0,		/* for vtoc partitions 5 -15	*/
	0,		0,
	0,		0,
	0,		0,
	0,		0,		
	0,		0,	
	0,		0,	
	0,		0,	
	0,		0,	
	0,		0,	

};


/*
 * i1010minor
 *
 * This structure has been added to widen the minor number information.
 *
 *	i1010MINOR(board#,unit#,drtab#,slice#)
 * 	[minor] device
 * This table configures the board number, slice number, drtab number
 * and unit number.
 */
unsigned i1010minor[40] = {
	i1010MINOR(0,0,0,0),			/* [0] unit 0 slice 0 */
	i1010MINOR(0,0,0,1),			/* [1] unit 0 slice 1 */
	i1010MINOR(0,0,0,2),			/* [2] unit 0 slice 2 */
	i1010MINOR(0,0,0,3),			/* [3] unit 0 slice 3 */
	i1010MINOR(0,0,0,4),			/* [4] unit 0 slice 4 */
	i1010MINOR(0,0,0,5),			/* [5] unit 0 slice 5 */
	i1010MINOR(0,0,0,6),			/* [6] unit 0 slice 6 */
	i1010MINOR(0,0,0,7),			/* [7] unit 0 slice 7 */
	i1010MINOR(0,0,0,8),			/* [8] unit 0 slice 8 */
	i1010MINOR(0,0,0,9),			/* [9] unit 0 slice 9 */
	i1010MINOR(0,0,0,10),			/* [10]unit 0 slice 10*/
	i1010MINOR(0,0,0,11),			/* [11]unit 0 slice 11*/
#ifdef	MP386
	i1010MINOR(0,0,0,0),			/* [12-15] reserved */
	i1010MINOR(0,0,0,0),			/* [12-15] reserved */
	i1010MINOR(0,0,0,0),			/* [12-15] reserved */
	i1010MINOR(0,0,0,0),			/* [12-15] reserved */
	i1010MINOR(0,1,1,0),			/* [16]unit 1 slice 0 */
	i1010MINOR(0,1,1,1),			/* [17] unit1slice 1 */
	i1010MINOR(0,1,1,2),			/* [18] unit1slice 2 */
	i1010MINOR(0,1,1,3),			/* [19] unit1slice 3 */
	i1010MINOR(0,1,1,4),			/* [20] unit1slice 4 */
	i1010MINOR(0,1,1,5),			/* [21] unit1slice 5 */
	i1010MINOR(0,1,1,6),			/* [22] unit1slice 6 */
	i1010MINOR(0,1,1,7),			/* [23] unit1slice 7 */
	i1010MINOR(0,1,1,8),			/* [24] unit1slice 8 */
	i1010MINOR(0,1,1,9),			/* [25] unit1slice 9 */
	i1010MINOR(0,1,1,10),			/* [26]unit1slice 10*/
	i1010MINOR(0,1,1,11),			/* [27]unit1slice 11*/
#else	/* ! MP386 */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,0,0,0),			/* [12-19] reserved */
	i1010MINOR(0,1,1,0),			/* [20]unit 1 slice 0 */
	i1010MINOR(0,1,1,1),			/* [21] unit1slice 1 */
	i1010MINOR(0,1,1,2),			/* [22] unit1slice 2 */
	i1010MINOR(0,1,1,3),			/* [23] unit1slice 3 */
	i1010MINOR(0,1,1,4),			/* [24] unit1slice 4 */
	i1010MINOR(0,1,1,5),			/* [25] unit1slice 5 */
	i1010MINOR(0,1,1,6),			/* [26] unit1slice 6 */
	i1010MINOR(0,1,1,7),			/* [27] unit1slice 7 */
	i1010MINOR(0,1,1,8),			/* [28] unit1slice 8 */
	i1010MINOR(0,1,1,9),			/* [29] unit1slice 9 */
	i1010MINOR(0,1,1,10),			/* [30]unit1slice 10*/
	i1010MINOR(0,1,1,11),			/* [31]unit1slice 11*/
#endif	/* ! MP386 */
};

