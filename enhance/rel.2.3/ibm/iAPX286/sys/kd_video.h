/* uportid = "@(#)kd_video.h	Microport Rev Id  1.3.3 6/18/86" */
/*
 * @(#)kd_video.h	1.3
 *
 * M000 lance 2-20-86
 * 		actually, not the first change.  
 *		removed COL_MODER define (not needed)
 *		added board variable bits 
 * M001 lance 4-28-86
 *		Added defines for EGA/PGA/HERC support.
 */

/* general defines */
/* defines for the video code */
#define BWMODE 		0x07
#define CRT_MASK 	0x30
#define BW_FLAG  	0x30
#define COL_FLAG  	0x20			/* M001 */
#define BW_CRTR  	0x3B4
#define COL_CRTR 	0x3D4

#ifdef	MP386
#define EGA_VRBASE (union screenel *) 0xC00A0000L
#define BW_VRBASE  (union screenel *) 0xC00B0000L
#define COL_VRBASE (union screenel *) 0xC00B8000L
#else
#define EGA_VRBASE (union screenel *)  gstokv(VIDEO2SEL)
#define BW_VRBASE  (union screenel *)  gstokv(VIDEOSEL)
#define COL_VRBASE (union screenel *) (((unsigned long) gstokv(VIDEOSEL)) \
				      | (unsigned long) 0x8000L)
#endif

#define NUMPAGES 4
#define DEFCON 0

/* defines for board variables     M000 */
#define	MONOBOARD	0x20			/* M000 */
#define	CGABOARD	0x40			/* M001 */
#define	EGABOARD	0x80			/* M001 */
#define	HERCBOARD	0x100			/* M001 */
#define	PGABOARD	0x200			/* M001 */
#define	BOARD(x)	(x & 0xfc0)		/* M001 */

#define TABLN 8

struct chcell {			/* the structure which defines a chracter */
	/* cell consists of a character byte and */
	/* an attribute byte */
	unsigned char ch;
	unsigned char at;
};

union screenel {		/* the screenel structure , depending */
	/* on mode, is either 16-bit character */
	/* structures, or 16-bit graphic cells */
	struct chcell chc;
	unsigned short grc;
};

#define	NUM_SCREEN_COLUMNS	80
#define	NUM_SCREEN_ROWS		25
#define NUM_SCREEN_CHARS	NUM_SCREEN_COLUMNS * NUM_SCREEN_ROWS

struct virtscreen {		/* the virtual screen defintions */
	union screenel virts [ ( NUM_SCREEN_CHARS ) ];
};

/* === */
