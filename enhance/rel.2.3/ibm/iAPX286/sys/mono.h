/* uportid = "@(#)mono.h	Microport Rev Id  1.3.3 6/18/86" */
/* defines for the video code */
#define BWMODE 0x07
#define CRT_MASK 0x30
#define BW_FLAG  0x30
#define BW_CRTR  0x3B4
#define COL_CRTR 0x3D4
#define BW_VRBASE (union screenel * )  gstokv(VIDEOSEL)
#define COL_VRBASE (union screenel *) (gstokv(VIDEOSEL) + (unsigned long) 0x8000)
#define TBL_LN 16

#define REGOSC 0x30
#define SPOSC 0x3F

#define NUMLINES 24
#define MAXLINE 23

#define NUMCOLS 80
#define MAXCOL 79

#define TABLN 8

extern struct seg_desc gdt[];	/* segment descriptor for video */


struct curs_types {		/* the structure which hold the start and end */
	int strtlc;		/* line numbers for the current cursor */
	int endlc;
};

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
	unsigned int grc;
};

struct scrlscrn {		/* the scroll screen structure */
	union screenel scrls[ ( NUMCOLS * NUMLINES ) - NUMCOLS ];
};

struct blnkln {			/* the line blanking structure */
	union screenel blnk[ NUMCOLS ];
};
