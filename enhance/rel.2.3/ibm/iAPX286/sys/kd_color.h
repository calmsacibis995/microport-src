/* @(#)kd_color.h	1.6 Microport 10/1/87 */
/*
 * M001 uport!mike Mon Mar 23 14:34:31 PST 1987
 *	Added latest LCC changes (from rel 1.3.8 Beta 7)
 *
 */

#define	MAXESCDATA	32		/* up to 32 parameters */
#define	NUMSETCHARS	512		/* max setkey chars total */
#define	NUMSIGKEYS	32		/* max number of signal keys */
#define VIDRAMLEN 28000			/* default size of a plane */

/* these are wrong in sys/kd.h */
#define	R_CURSTART	10		/* cursor start address */
#define	R_CUREND	11		/* cursor end address   */

#ifdef MP386
#define	IOCTLARG	int
#define	IOCTLINT(x)	x
#define	IOCTLCPTR(x)	x
#define	SPLX(x)				/* 5.3/386 */
#define	SPL6(x)
#else	/* ! MP386 */
/* #define	ONGUARD	   {asm(" cli");} */
/* #define	UNGUARD	   {asm(" sti");} */
#define	IOCTLARG	union ioctl_arg
#define	IOCTLINT(x)	x.iarg
#define	IOCTLCPTR(x)	x.cparg
#define	SPLX(x)		splx(x)		/* 5.2/286 */
#define	SPL6(x)		x = spl6()
#endif	/* MP386 */

#define	TTYSPL() 	spl7()		/* timeout-clock-level 6 problem */

#define	MODEREG_BLINK	0x20		/* mode reg blink/bright bg bit */

#define	MOVEF(base,from,to,cnt)	kd_movwf (base,from,to,cnt,FLKRREG,MODEREG)
#define	MOVEB(base,from,to,cnt)	kd_movwb (base,from,to,cnt,FLKRREG,MODEREG)
#define	FILLW(fill,base,to,cnt)	kd_fillw (fill,base,to,cnt,FLKRREG,MODEREG)

/* get card code out of dev */
/* get unit number out of card and dev */
#define UNIT(x)	(x % NUMCONSOLES)

struct bit_plane {
    char data [VIDRAMLEN];
};
struct cursor_state {		/* *saved* cursor attributes */
    struct chcell fill;		/* fill data */
    long modeflags;		/* mode flags, (ie. insert mode) */
    short row;			/* current cursor row */
    short col;			/* current cursor column */
    short pos;			/* current cursor position */
    short size;			/* cursor size */
    char font;			/* allow ctrl chrs, high bank of font */
    char chattr;		/* current character attributes */
    char lastchar;		/* last character transmitted */
};

/* key data for NORM, SHIFT, ALT, ALTSHIFT */
struct keydata {
    unsigned short data [4];
};

struct colr_state {		/* virtual screen state */
    struct tty *tty_tp;			/* tty state struct */
    struct kda_state *kda;		/* adapter state */
    struct virtscreen *screen;		/* where the screen data is */
    struct virtscreen *vs;		/* off screen save area */
    char *fonttable;			/* this console's fonttable */

    short biosmode;			/* display mode this console */
    short fakepos;			/* fake cursor position (+1) */
    short flkrreg;			/* if flicker problem this is stat reg*/
    short kb_xlate;			/* K_XLATE, K_RAW, etc. */
    short lastactive;			/* page displayed before this one */
    short minrow, maxrow;		/* scrolling region pointers */
    short minscrcol, maxscrcol;
    short minscrrow, maxscrrow;		/* screen display pointers */
    short pels;				/* pixel elements per char */
    short pagelen;			/* page length */
    short unit;				/* unit number */

    char cntesc;			/* length of escape string */
    char colorreg;			/* color reg contents */
    char escflg;			/* in escape string */
    char graphlock;
    char graphwait;
    char inwrite;			/* in write system call? */
    char mapped;			/* Is this the current page? */
    char modegraph;			/* which graphic mode console in */
    char modereg;			/* mode reg contents */
    char numplanes;			/* number of graphic planes */
    char openned;			/* this console is openned */
    char parmpres;			/* parameter present */
    char vidlock;			/* semaphore */

    struct cursor_state cur;		/* current cursor state */
    struct cursor_state save;		/* saved cursor state */

    short escdata [MAXESCDATA];		/* escape string found */
    struct keydata *kb_data;		/* pointer to translation data */
    unsigned char *kb_control;		/* pointer to control chars */
    struct setkey sk;			/* keyboard setkey */

    int (*gfxoff)();			/* turn off graphics */
    int (*gfxsave)();			/* save graphics */
    int (*gfxrest)();			/* restore graphics */
    int (*gfxmove)();			/* move plane data */

    char *ctrlregs;			/* pointer to controller regs */
    unsigned char *kb_keymap;		/* keymap buffer */
    unsigned char *kb_nextkm;		/* first available slot */
    unsigned char *kb_lastkm;
    unsigned char kb_locd;		/* code limits */
    unsigned char kb_hicd;

    char kb_buff[16];			/* temp for generating return data */
    char *bit_planes [16];		/* graphic bit plane save data */
    char *gfx_buff;			/* pointer to graphics output buffer */
    long gfx_buffsize;			/* size of graphics buffer */
    struct biosinitdata *grafregs;	/* graphic mode regs */
};

struct kdc_state {		/* console states */
    short numcons;		/* number of virtuals consoles enabled */
    short fill;
    struct colr_state *cstp [NUMCONSOLES]; /* console state pointers */
};

union dev_address {
    char *mem;			/* memory mapped aaddress */
    int io;			/* io mapped address */
};

struct kda_state {		/* adapter state */
    long debug;			/* global debug flag, SHOULD BE FIRST! */
    long screensize;		/* number of characters on the real screen */
    union dev_address ad_6845;	/* base address of CRT controller */
    union screenel *baseram;	/* pointer to real video memory */

    struct biosinitdata *moderegs;	/* ega mode regs */
    char *fonttable;		/* saved fonttable */
    char *curfont;		/* current loaded font */

    short actpage;		/* which page is being displayed */
    short board;		/* type of board installed */
    short biosmode;		/* current adapter mode */
    short cursor;		/* current cursor shape */
    short defmode;		/* default screen mode */
    short egamem;		/* amount of ega memory */
    short modeindex;		/* display mode index */
    short flkr;			/* flicker problem is present */
    short kb_state;		/* current kb shift states */
    short kb_status;		/* kb I/O control status */
    short kb_table;		/* kb shift table in use */
    short sigdata;		/* user specified signal data */
    short sigactive;		/* console active when signal key pressed */
    short signext;		/* next signal key entry */

    char asyinit;		/* async debug i/o has been initialized */
    char colrreg;		/* data in color reg */
    char ecd;			/* enhanced color display */
    char ega;			/* board is an ega */
    char hercules;		/* Is this mono board a herc? */
    char init;			/* keyboard/display has been initialized */
    char kb_leds;		/* led light states */
    char lastchar;		/* last character received from keyboard */
    char modeattr;		/* ega attribute currently set */
    char modereg;		/* data in mode reg */
    char nextpage;		/* screen swap is scheduled */
    char numplanes;		/* number of bitplanes */
    char unixunit;		/* saved unix unit # */
    char video_off;		/* flag indicating video is off */

    struct setsigkey sigmap [NUMSIGKEYS];	/* pointer to signal map */
};

/* cstp->openned flags */
#define	KDOPEN_ALPHA	1
#define	KDOPEN_GRAPHIC	2
#define	KDOPEN_INITFLAG	4

#define	CURPOS		cstp->cur.pos
#define	CURSORSIZE	cstp->cur.size
#define CROW 		cstp->cur.row
#define CCOL 		cstp->cur.col
#define MODEFLAGS 	cstp->cur.modeflags
#define FONTMODE 	cstp->cur.font
#define	FILL		cstp->cur.fill
#define ATRBTE 		cstp->cur.chattr
#define	LASTCHAR	cstp->cur.lastchar

#define	MINSCRROW	0
#define	MINSCRCOL	0
#define	MAXSCRROW	cstp->maxscrrow
#define	MAXSCRCOL	cstp->maxscrcol

#define	MINROW		cstp->minrow
#define MAXROW		cstp->maxrow
#define	MINCOL		0
#define MAXCOL		MAXSCRCOL
#define NUMCOLS		(MAXCOL + 1)
#define NUMROWS		(MAXROW + 1)

#define	FLKRREG		cstp->flkrreg
#define	FAKEPOS		cstp->fakepos
#define ESCFLG 		cstp->escflg
#define CNTESC 		cstp->cntesc
#define ESCDATA 	cstp->escdata
#define	PARMPRES	cstp->parmpres
#define VIDLOCK 	cstp->vidlock

#define	MAPIT_ALPHA	1
#define	MAPIT_GRAPHIC	2
#define	MAPPED_ANY	cstp->mapped
#define	MAPPED		(cstp->mapped & MAPIT_ALPHA)
#define	MAPPED_GRAPHIC	(cstp->mapped & MAPIT_GRAPHIC)

#define	MODEREG		cstp->modereg
#define	COLRREG		cstp->colorreg
#define	INWRITE		cstp->inwrite
#define	SCREEN		cstp->screen->virts

#define ALL_ATTRIB_OFF 0
#define NORM_ATTRIB_ON 7
#define UNDL_ATTRIB_ON 1
#define INTEN_ATTRIB_ON 0x8
#define BLINK_ATTRIB_ON 0x80
#define RVID_ATTRIB_ON 0x70
#define REGOSC 0x30

/* MODEFLAGS Bits */
#define	CRM	0x01	/* Control Representation Mode			*/
#define	IRM	0x02	/* Insertion/Replacement Mode			*/
#define	VEM	0x04	/* Vertical Editing Mode (not implemented)	*/
#define	HEM	0x08	/* Horizontal Editing Mode (not implemented)	*/
#define	FEAM	0x10	/* Format Effector Action Mode			*/
#define	LNM	0x20	/* Line Feed/New Line Mode			*/
#define	OM	0x100	/* Origin Mode					*/
#define	WM	0x200	/* Wrap Mode					*/

#define	CRTERM	0x2000000  /* Terminate responses with newline		*/
#define	FAKECUR	0x4000000  /* Fake cursor is in effect			*/
#define	BSUND	0x8000000  /* BackSpace UNDerline sequence in effect	*/

#define	CRM_MODE	(MODEFLAGS & CRM)
#define	IRM_MODE	(MODEFLAGS & IRM)
#define	VEM_MODE	(MODEFLAGS & VEM)
#define	HEM_MODE	(MODEFLAGS & HEM)
#define	FEAM_MODE	(MODEFLAGS & FEAM)
#define	LNM_MODE	(MODEFLAGS & LNM)
#define	ORIGIN_MODE	(MODEFLAGS & OM)
#define	WRAP_MODE	(MODEFLAGS & WM)
#define	CR_TERM		(MODEFLAGS & CRTERM)
#define	FAKE_CURSOR	(MODEFLAGS & FAKECUR)
#define	BSUND_MODE	(MODEFLAGS & BSUND)

/* figure the pos of a row and col in terms of integer offset */
#define POSIT(prow, pcol) (((prow) * NUMCOLS) + pcol)

/* screen shape definitions */
typedef struct gfx_mode {
    short mode_attr;
    char xbits;		/* number pixels per byte			*/
    char modeindex;	/* mode table index				*/
    char alphamode;	/* is this mode an alpha mode ?			*/
    short colors;	/* number of colors supported			*/
    unsigned long rambase; /* base of video ram				*/
    short charx;	/* number of character dots (horizontal)	*/
    short chary;	/* number of character dots (vertical)		*/
    short pages;	/* number of pages of memory			*/
    short x;		/* number of pixels in x			*/
    short y;		/* number of pixels in y			*/
} Gfx_Mode;

/* I/O Addresses */

#define	CRT_CTRL	(kda.ad_6845.io)
#define	CRT_DATA	(kda.ad_6845.io + 1)
#define	CRT_MODE	(kda.ad_6845.io + 4)
#define	CRT_COLOR	(kda.ad_6845.io + 5)
#define	CRT_STAT	(kda.ad_6845.io + 6)

#define	CRT_ATRB	0x3C0
#define	CRT_MISC	0x3C2
#define	CRT_SEQ		0x3C4
#define	CRT_POS2	0x3CA
#define	CRT_POS1	0x3CC
#define	CRT_GRAPH	0x3CE

#define	KEYBD_OUT	0x60
#define	KEYBD_IDAT	0x60
#define KEYBD_STAT	0x64
#define KEYBD_ICMD	0x64

/* send data to the 6845 crt controller */
#define	WRITECONT(reg, val)	kd_outb21 (CRT_CTRL, reg, val) /* byte */
#define	COLROUT6845(reg, val)	kd_outb23 (CRT_CTRL, reg, val) /* short */
#define	WRITEMODE(val)		kd_outb   (CRT_MODE, val)
#define	WRITECOLOR(val)		kd_outb   (CRT_COLOR, val)
#define	READSTATUS		kd_inb    (CRT_STAT)
#define	WRITESTATUS(val)	kd_outb   (CRT_STAT, val)

#define FAKEOFF {int x=spl7();						\
		if (vid_depth++ == 0)					\
		    kd_fakecursor(cstp, 0);				\
		splx(x);						\
		}
#define FAKEON { int x=spl7(); 						\
		 if (--vid_depth == 0)					\
			kd_fakecursor(cstp, 1);				\
		 splx(x);						\
	       }

#ifdef	DEBUG
#define DBG(l, p)	if (kda.debug & (1<<l)) { p; }
#else
#define DBG(l, p)
#endif

#define	DBG0(p)		DBG(0, p)		
#define	DBG1(p)		DBG(1, p)		
#define	DBG2(p)		DBG(2, p)		
#define	DBG3(p)		DBG(3, p)		
#define	DBG4(p)		DBG(4, p)		
#define	DBG5(p)		DBG(5, p)		
#define	DBG6(p)		DBG(6, p)		
#define	DBG7(p)		DBG(7, p)		
#define	DBG8(p)		DBG(8, p)		
#define	DBG9(p)		DBG(9, p)		
#define	DBG10(p)	DBG(10, p)		
#define	DBG11(p)	DBG(11, p)		
#define	DBG12(p)	DBG(12, p)		
#define	DBG13(p)	DBG(13, p)		
#define	DBG14(p)	DBG(14, p)		
#define	DBG15(p)	DBG(15, p)		

/* example:
 * DBG1 (printf("flopen: devstat[%x]=%x \n", unit, devstat[unit]));
 */

/* === */
