#ifndef lint
static char *uportid = "@(#)kd_color.c	3.0 Microport Rev 2.3 10/1/87";
#endif
/*
 * uport!mike	Wed Sep 16 10:35:55 PDT 1987
 *		removed kernel interface routines (open,close) to kd1.c
 */

/*
 * Table of ifdefs
 *
 MP386		- compile code relating to the 5.3/386 driver
 MERGE386	- compile code relating to MERGE/386 changes/additions.
 ATMERGE	- compile code relating to MERGE/286 changes/additions.
 DEBUGGER	- removes the kernel putchar if the debugger is present in
		- the kernel.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/conf.h"

#ifndef MP386
#include "sys/dir.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/ppi.h"
#include "sys/mmu.h"
#else
#include "sys/fs/s5dir.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/pump.h"
#include "sys/cio_defs.h"
#include "sys/vtoc.h"
#include "sys/open.h"
#include "sys/immu.h"
#endif

#include "sys/user.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/clock.h"
#include "sys/ioctl.h"
#include "sys/kd_video.h"
#include "sys/seg.h"
#include <sys/sysmacros.h>

#include "sys/kd.h"
#include "sys/kd_info.h"
#include "sys/setkey.h"
#include "sys/kd_color.h"
#include "sys/kd_mode.h"

#ifdef	ATMERGE
#include "sys/realmode.h"
#endif

/* patchable parameters */
int fillattr = NORM_ATTRIB_ON;		/* default fill attribute */
int fillchar = ' ';			/* default fill character */
int kdgrfmod = 0x10;			/* default graphics mode */
int kdcons   = -1;			/* output to current active screen */
int kdmaxrow = 23;			/* default scrolling region 0-23 */

/* externally patched parameters */
extern int kernputc;			/* kernel putchar device */
extern int colrsnow;			/* flicker problem present */

/* Static state structures */
struct virtscreen kd_vs0;		/* console 0 screen save data */
struct colr_state kd_cstp0;		/* console 0 virt screen state */
struct kda_state  kda = { 0 };		/* adapter state structs */
struct kdc_state  kdc = { 0 };		/* console state pointers */

#ifdef	ATMERGE
extern struct sw_data usw_data;
int startscr = 0;
int fakecur = 1;			/* patchable */
#else
int fakecur = 0;			/* patchable */
#endif
int faketype = 0;

/* keep track of cursor recursion state */
int vid_depth = 1;			
short fakepos = 0;

/*
 * Address constants for discovering BIOS secrets
 */
#define	IARG(list, index)	(*(int *) (&list [index]))
#define	LOMEMW(index)		IARG (lomem, index)
#define	BIOS_VIDEO_INT		0x40
#define	BIOS_EQUIP_FLAG		0x410
#define	BIOS_CRT_MODE		0x449
#define	BIOS_CURSOR_POSN	0x450
#define	BIOS_CURSOR_MODE	0x460
#define	BIOS_ADDR_6845		0x463
#define	BIOS_CRT_MODE_SET	0x465
#define	BIOS_INFO		0x487
#define	BIOS_INFO_3		0x488

/*
 * kdcinit
 *     Initialize the system console (unit 0)
 */
kdcinit ()
{
    register struct colr_state *cstp;
    unsigned char *lomem, *io_mapbuf();

    if (kda.init)	/* avoid duplicate initialization */
	return;

#ifdef	MP386
    bclr (&kda, sizeof (kda));
#endif
    cstp = kdc.cstp [0] = &kd_cstp0;	/* system console (0) */
    bclr (cstp, sizeof (*cstp));	/* clear state block  */

    /* Init the video parms as BIOS has determined things */
    lomem = io_mapbuf (0);		/* map start of memory */

    /* The following statement checks to see if the BIOS video interrupt
     * has been remapped to the video rom. If so, we assume we have an
     * EGA.
     */
    kda.ega         = (lomem [BIOS_VIDEO_INT + 3] & 0xF0) == 0xC0;
    kda.ad_6845.io  = LOMEMW (BIOS_ADDR_6845);
    kda.biosmode    = kda.defmode = lomem [BIOS_CRT_MODE];
    kda.nextpage    = -1;		/* disable screen change */
    kda.cursor      = -1;		/* indicate the cursor is off */

    if ((lomem [BIOS_EQUIP_FLAG] & 0x30) == 0x30) {
	kda.board      = MONOBOARD;		/* BIOS found mono board */
	kda.baseram    = BW_VRBASE;
	kda.screensize = 2048 * 2;
    }
    else {
	kda.board      = CGABOARD;
	kda.screensize = 8192 * 2;
	kda.ecd        = (lomem [BIOS_INFO_3] & 0x0F) == 0x09;
	kda.egamem     =  lomem [BIOS_INFO] & 0x60;
#ifndef ATMERGE
	kda.baseram = COL_VRBASE;
#else
	gdt [VIDEOSEL].sd_lowbase |= 0x8000;
	kda.baseram = BW_VRBASE;
#endif
    }
    cstp->vs	     = &kd_vs0;		/* ptr to save area */
    cstp->screen     = (struct virtscreen *) kda.baseram;
    cstp->numplanes  = kda.numplanes = 4;
    cstp->lastactive = -1;		/* no previous console */
    MAXROW = kdmaxrow;			/* kd_setscroll will readjust */
    MAXCOL = 79;
    kdmode (cstp, kda.defmode, kda.defmode, 2);
    FILL.at = fillattr;			/* fill attribute */
    FILL.ch = fillchar;			/* fill character */
    ATRBTE = FILL.at;			/* current character attributes */
    if (kda.flkr = colrsnow)		/* if a flicker problem, */
	FLKRREG = CRT_CTRL;		/* ... indicate as such */
    if (fakecur) {			/* if fake cursor config flag on */
	cursoff (); 			/* turn off real cursor */
	MODEFLAGS |= FAKECUR;
    }
    CURSORSIZE = (cstp->ctrlregs [0x0A] << 8) | (cstp->ctrlregs [0x0B]);
    kd_setcur (cstp, lomem [BIOS_CURSOR_POSN + 1], lomem [BIOS_CURSOR_POSN]);
    kb_init (cstp);			/* init keyboard states */
    cstp->save = cstp->cur;		/* init saved cursor state */
    io_unmapbuf (lomem);
    kda.init = 1;
}

/*
 * kd_opendev
 *     open a new virtual console and initialize it's state.
 */
kd_opendev (unit)
    int unit;
{
    register struct colr_state *cstp;
    struct kda_mem {
	struct colr_state cst;
	struct biosinitdata bid;	/* bios init data */
	struct virtscreen vsd;		/* screen save data */
    };
    struct kda_mem *kdmem, *io_getbuf();

    DBG11 (printf ("kd_opendev: unit:%d\n", unit));
    if (unit >= NUMCONSOLES)
	return 1;		/* temporary */
    if (unit == 0)
	return 0;

    if (! (cstp = kdc.cstp [unit])) {
	if (! (kdmem = io_getbuf (sizeof (struct kda_mem), 1)))
	    return 1;
	cstp = kdc.cstp [unit] = &kdmem->cst;
	cstp->screen = cstp->vs = &kdmem->vsd;
	cstp->grafregs = &kdmem->bid;	/* graphic mode regs */
	cstp->unit = unit;
	cstp->lastactive = -1;		/* no previous console */
	FILL.at = fillattr;
	FILL.ch = fillchar;
	ATRBTE = FILL.at;
	if (fakecur) MODEFLAGS |= FAKECUR;
	MAXROW = kdmaxrow;
	MAXCOL = 79;
	cstp->numplanes = kda.numplanes;
	kdmode (cstp, kda.defmode, 0, 0);
	CURSORSIZE = (cstp->ctrlregs [0x0A] << 8) | (cstp->ctrlregs [0x0B]);
	kd_setcur (cstp, 0, 0);
	cstp->save = cstp->cur;
	FILLW (FILL, SCREEN, 0, NUMROWS * NUMCOLS);
	kb_init (cstp);	
    }
    return 0;
}

/*
 * kd_deledev
 *     delete a console openned by kd_opendev
 */
kd_deledev (cstp)
    struct colr_state *cstp;
{
    int x;
    DBG11 (printf ("kd_deledev: unit:%d\n", cstp->unit));

    if(cstp->unit && !(cstp->openned & ~KDOPEN_INITFLAG)){
	if ((cstp->unit == kda.actpage || MAPPED_ANY)
	&&   cstp->lastactive != -1)  /* if mapped */
	    kdapset (cstp->lastactive);		   /* try to change screens */

	if (cstp->unit == kda.actpage || MAPPED_ANY) /* if still mapped */
	    return 0;				     /* forget it for now */

	SPL6(x);
	kdc.cstp [cstp->unit] = 0;
	SPLX(x);
	if (cstp->gfxoff)
	    cstp->gfxoff (cstp);
	if (cstp->fonttable && cstp->fonttable != kda.fonttable)
	    io_freebuf (cstp->fonttable);

	for (x = 0; x < NUMCONSOLES; x++)
	    if (kdc.cstp [x] && kdc.cstp [x]->lastactive == cstp->unit)
		kdc.cstp [x]->lastactive = cstp->lastactive;

	io_freebuf (cstp);
	return 0;
    }
    return 1;
}

/*
 * Update the color flicker state 
 */
kd_updflkr ()
{
    register struct colr_state *cstp;
    int i;
    ushort flkr;

    if (kda.flkr = colrsnow)
	flkr = CRT_CTRL;
    else
	flkr = 0;

    for (i = 0; i < NUMCONSOLES; i++) {
	if (cstp = kdc.cstp [i])
	    FLKRREG = MAPPED ? flkr : 0;
    }
}

/*
 * set cursor position
 */

kd_setcur (cstp, row, col)
    struct colr_state *cstp;
    int row;
    int col;
{
    DBG7 (printf ("kd_setcur: cstp:%lx row:%d col:%d\n", cstp, row, col));
    if (row > MAXSCRROW) row = MAXSCRROW;
    if (col > MAXSCRCOL) col = MAXSCRCOL;
    CROW = row;
    CCOL = col;
    CURPOS = POSIT (row, col);
    
    if (MAPPED) {
	if (FAKE_CURSOR) {
	    cursoff (); 			/* turn off real cursor */
	    if (FAKEPOS)
		kd_fakecursor (cstp, 0);	/* turn off fake cursor */
	    kd_fakecursor (cstp, 1);		/* then turn it on */
	}
	else {
	    if (FAKEPOS)
		kd_fakecursor (cstp, 0);	/* turn off fake cursor */

	    if (kda.cursor != CURSORSIZE) {	/* if shape is different */
		kda.cursor  = CURSORSIZE;
		COLROUT6845 (R_CURSTART, CURSORSIZE);
	    }
	    COLROUT6845 (R_CURADRH, CURPOS);
	}
    }
}

/*
 * Turn off real cursor
 */
cursoff ()
{
    kda.cursor  = -1;
    COLROUT6845 (R_CURSTART, -1);	/* turn off real cursor */
}

/*
 * Turn ON/OFF Fake Cursor
 */
kd_fakecursor (cstp, on)
    struct colr_state *cstp;
    int on;
{
    register struct chcell *pos;
    unsigned char attr;

    DBG7 (printf ("kd_fakecursor: cstp:%lx on:%x\n", cstp, on));
    if (on)
	pos = (struct chcell *) & SCREEN [CURPOS];
    else if (FAKEPOS)
	pos = (struct chcell *) & SCREEN [FAKEPOS - 1];
    else
	return;		/* turning off cursor when already off */

    if (FLKRREG) {
	kd_hwait (FLKRREG + STATUS_REG); /* wait for horizontal retrace */
	attr = pos->at;
	asm (" sti");
    }
    else
	attr = pos->at;
    
    switch (faketype) {		/* different ways to reverse the character */
	default: case 0:
	    attr = (attr & 0x88) | (attr >> 4 & 0x07) | (attr << 4 & 0x70);
	    break;
	case 1:
	    attr = (attr >> 4 & 0x0F) | (attr << 4 & 0xF0);
	    break;
	/* add more here */
    }

    if (FLKRREG) {
	kd_hwait (FLKRREG + STATUS_REG); /* wait for horizontal retrace */
	pos->at = attr;
	asm (" sti");
    }
    else
	pos->at = attr;

    FAKEPOS = on ? CURPOS + 1 : 0;
}

/*
 * set scroll boundaries
 */
kd_setscroll (cstp, minrow, maxrow)
    struct colr_state *cstp;
    int minrow;
    int maxrow;
{
    DBG6 (printf ("kd_setscroll: cstp:%lx minrow:%d maxrow:%d\n",
	cstp, minrow, maxrow));
    if (minrow > MAXSCRROW)  minrow = MAXSCRROW;
    if (maxrow > MAXSCRROW)  maxrow = MAXSCRROW;
    if (MAXCOL > MAXSCRCOL)  MAXCOL = MAXSCRCOL;

    MINROW = minrow;
    MAXROW = maxrow;
}

/* Scroll the screen */
kd_scroll (cstp, row, dir)
    struct colr_state *cstp;
    int row;
    int dir;
{
    short minpos;
    short numchars;
    short clrline;
    char move = 0;

    if (dir) {
	if (row == NUMROWS) {	/* we're going to scroll up */
	    move++;
	    clrline = MAXROW * NUMCOLS;
	}
    }
    else {
	if (row == (MINROW - 1)) {	/* we're going to scroll down */
	    move++;
	    clrline = MINROW * NUMCOLS;
	}
    }
    if (move) {
	if (FAKEPOS)
	    kd_fakecursor (cstp, 0);	/* turn off fake cursor */
	if (numchars = (MAXROW - MINROW)) {
	    numchars *= NUMCOLS;
	    minpos = MINROW * NUMCOLS;
	    if (dir)
		MOVEB (SCREEN, minpos, NUMCOLS, numchars);
	    else
		MOVEF (SCREEN, minpos, NUMCOLS, numchars);
	}
	FILLW (FILL, SCREEN, clrline, NUMCOLS);
    }
    else {
	if (dir) {
	    if (row == (MAXSCRROW + 1))
		move++;
	}
	else {
	    if (row == (MINSCRROW - 1))
		move++;
	}
    }
    return (int) move;
}

/*
 * Put str on screen, filter to handle scrolling, automatic margins
 * and insert mode
 */
kd_puts (str, chrcnt, cstp, transparent)
    unsigned char *str;
    int chrcnt;
    struct colr_state *cstp;
    int transparent;
{
    struct chcell s;
    struct chcell *sp;
    short nch;
    short tmpcol;
    short row;
    short col;
    short flags;
    unsigned char fontbit;
    unsigned char c;

    if (FAKEPOS)
	kd_fakecursor (cstp, 0);		/* turn off fake cursor */
    flags  = 0;
    nch    = 0;
    row    = CROW;
    col    = CCOL;
    CURPOS = POSIT (row, col);
    sp     = (struct chcell *) & SCREEN [CURPOS];
    s.at   = ATRBTE;

#ifdef ATMERGE
    if (FLKRREG && usw_data.sw_curscreen == UNIXMODE) flags |= 1;
#else
    if (FLKRREG)			flags |=  1;	/* use poll/poke */
#endif
    if (IRM_MODE)			flags |=  2;	/* insert mode */
    if ((FONTMODE & 1) == 0)		flags |=  4;	/* interpret chars<32*/
    if (MAPPED_ANY && CRM_MODE && kernputc) flags |=  8; /* echo character */
    if (! LNM_MODE)			flags |= 16;	/* CR follows LF */
    if (WRAP_MODE)			flags |= 32;	/* dont wrap */
    if (BSUND_MODE)			flags |= 64;	/* _ BS seq */

    fontbit = FONTMODE & 2 ? 0x80 : 0;			/* add 8th bit */

    DBG3(printf("\nkd_puts: str=%lx chrcnt=%d flags=%x: ", str, chrcnt, flags));
    while (--chrcnt >= 0) {
	nch++;
	c = *str++;
	if (flags & 8) {
	    if (c < ' ') {
		putchar ('^');
		putchar ('@' + c);
	    }
	    else
		putchar (c);
	}
	if (!transparent) {
	    if (c == '\033') {
		ESCFLG = 1;
		break;
	    }
	    if ((flags & 4) && (c < ' ')) {
		switch (c) {
		    default:			break;
		    case '\007': beep();	break;
		    case '\b':	if (col > MINCOL) {
				    col--; sp--;
				}
				if (LASTCHAR == '_')
				    MODEFLAGS |= BSUND;
				break;
		    case '\t':	if (col <= (MAXCOL - TABLN)) {	/* tab */
				    tmpcol = col + TABLN;
				    tmpcol -= (tmpcol % TABLN);
				    sp += (tmpcol - col);
				    col = tmpcol;
				}
				break;
		    case '\n':	if ((++row > MAXROW) 		/* line feed */
				&& (kd_scroll (cstp, row, 1)))
				    --row;
				else
				    sp += NUMCOLS;
				if (flags & 16)		/* if not auto lf */
				    break;
		    case '\r':	if (col) {			/* return */
				    sp -= (col - MINCOL);
				    col = MINCOL;
				}
				break;
		}
		LASTCHAR = c;		/* record last character sent */
		continue;
	    }
	}
	if (flags & 2 && col < MAXCOL)
	    MOVEF (sp, 0, 1, MAXCOL - col);

	s.ch = c | fontbit;
	if (BSUND_MODE && LASTCHAR == '\b')
	    s.at = kd_underline (s.at);

	if (flags & 1)  	/* poll and poke */
	    kdasput (kda.modereg, sp, s, MODEREG);
	else
	    *sp = s;

	if (BSUND_MODE) {
	    s.at = ATRBTE;
	    MODEFLAGS &= ~BSUND;
	}
	LASTCHAR = c;		/* record last character sent */
	sp++;			/* bump to next position */
	if (++col > MAXCOL) {
	    if (flags & 32) {
		--col;
		--sp;
	    }
	    else {
		col = MINCOL;
		if ((++row > MAXROW) && (kd_scroll (cstp, row, 1))) {
		    --row;
		    sp -= NUMCOLS;
		}
	    }
	}
    }
    kd_setcur (cstp, row, col);
    DBG3 (printf (": nch=%d ", nch));
    return nch;
}

/* put character out to whatever current screen is being displayed */
kd_putc (ch)
    char ch;
{
    int tmp;
    register struct colr_state * cstp;

    if (! kda.init)		/* If not yet initialized, do it */
	kdcinit ();

    tmp = kdcons;				/* get console specifier */
    if (tmp < 0 || NUMCONSOLES <= tmp || !(cstp = kdc.cstp [tmp]))
	if (! (cstp = kdc.cstp [kda.actpage]))
	    return;

    tmp = FONTMODE;  /* use straight font for kernel putchar */
    FONTMODE = 0;
    kd_puts (&ch, 1, cstp, 0);
    FONTMODE = tmp;
}
int kdputsx = 0;		/* kd put string bypass (for debug) */

/* Asynchronous put string (kd_puts bypass) */
kd_asyputs (bufptr, bufcnt)
    char *bufptr;
    int bufcnt;
{
    int dev = kdputsx;	/* get bypass device	*/
    int cnt = bufcnt;	/* # chars to put 	*/

    while (--cnt >= 0) {
	asy_putc (dev, *bufptr);
	if (*bufptr++ == '\n')
	    asy_putc (dev, '\r');
    }
    return bufcnt;
}

/*===*/
