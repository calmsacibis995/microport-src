#ifndef lint
static char *uportid = "@(#)kd_setup.c	3.0 Microport Rev 2.3 10/1/87";
#endif

/*
 *	uport!mike Mon Mar  2 16:07:50 PST 1987
 *	Created.
 *
 * M001	uport!mike Mon Mar 23 14:29:42 PST 1987
 *	Added latest LCC changes (from 1.3.8 Beta 7)
 *
 * M002 uport!mike Mon May 11 09:33:48 PDT 1987
 *	Merge kd_init.c and kd_mode.c into kd_init.c
 *
 * M003 uport!mike Tue Sep 22 15:44:35 PDT 1987
 *	seperated into kd_mode.c (includes kd_mode.h mode data)
 *	and kd_setup.c (mode changing routines)
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
#include "sys/seg.h"
#include <sys/sysmacros.h>

#include "sys/kd_video.h"
#include "sys/kd.h"
#include "sys/kd_info.h"
#include "sys/setkey.h"
#include "sys/kd_color.h"
#include "sys/kd_mode.h"

extern unsigned char geteqsw(); /* reads switches on the AT motherboard */
extern struct kda_state kda;
extern struct kdc_state kdc;
int kdmscflg = 0;
int kd_mdfrc = 1;

#ifdef	ATMERGE
#include "sys/realmode.h"
extern struct sw_data usw_data;
#endif

char grfx1pos = 0;
char grfx2pos = 1;
char ega_feat [] = {	/* user patchable feature values */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#ifdef	MP386		/* I'm still not sure about this ! */
#define	LOBYTE(x)	(unsigned char) (x)
#else
#define	LOBYTE(x)	lobyte (x)
#endif

/*
 * Program the initialization table into the EGA
 */
kd_egainit (initmode, initdata)
    int initmode;
    struct biosinitdata *initdata;
{
    register int indx;
    register unsigned char *tab;

    kd_egaoff (CRT_STAT);
    tab = & initdata->s0;

#ifdef	REMOVE
    (void) READSTATUS;			/* reset addr flip-flop */ 
    kd_outb (CRT_ATRB, 0);			/* turn off screen */
    WRITESTATUS (ega_feat [modeindex]);	/* Feature control reg */
    (void) kd_inb (CRT_CTRL);	/* reset controller select */
#endif

    /* asm (" cli");			/* interrupts off */

    /* program the sequencer */
    kd_outb21 (CRT_SEQ, 0, 1);		/* reset sequencer */

    for (indx = 1; indx <= EGA_NUMSEQREGS; indx++)
	kd_outb21 (CRT_SEQ, indx, LOBYTE (*tab++));

    /* Set the current I/O address based on whether we're going to
     * a COLOR mode or a BLACK & WHITE (MONO) mode
     */
#ifdef	REMOVE
    kda.ad_6845.io = (*tab & 1) ? COL_CRTR : BW_CRTR;
#else
    kdmscflg = (*tab & 1) ? COL_CRTR : BW_CRTR;
#endif

    kd_outb (CRT_MISC, LOBYTE (*tab++));	/* set the misc reg */
    kd_outb21 (CRT_SEQ, 0, 3);		/* enable sequencer */
    /* asm (" sti");			/* interrupts back on */

    /* program the CRT controller */
    for (indx = 0; indx < EGA_NUMCTLREGS; indx++)
	kd_outb21 (CRT_CTRL, LOBYTE (indx), *tab++);

    /* program the attribute chip */
    (void) READSTATUS;			/* reset addr flip-flop */ 
    for (indx = 0; indx < EGA_NUMATRBREGS; indx++)
	kd_outb2 (CRT_ATRB, LOBYTE (indx), *tab++);

    kd_outb (CRT_ATRB, 0);

    /* program the graphics chip */
    kd_outb (CRT_POS1, grfx1pos);
    kd_outb (CRT_POS2, grfx2pos);
    for (indx = 0; indx < EGA_NUMGFXREGS; indx++)
	kd_outb21 (CRT_GRAPH, LOBYTE (indx), *tab++);

    kda.modeindex = initmode;
    kda.moderegs  = initdata;
    kda.modeattr  = initdata->a10;	/* ega attribute */
    kda.video_off = 1;			/* video is now off */
    DBG6 ( printf ("Done.\n"));
}

/*
 * program the initialization table into the CGA
 */
kd_cgainit (modeindex)
    int modeindex;
{
    register int indx;
    register unsigned char *tab;

    DBG6 (printf ("cgainit: modeindex:%x ", modeindex));

    /* reset COL video card */
    (void) READSTATUS;
    kd_outb (CRT_ATRB, 0);	/* turn off video */

    if (modeindex == kd_cgahercmode) {
	kd_outb (0x3bf, 1);			/* allow herc mode */
	tab = herc_crtc;
    }
    else if (modeindex < kd_numcgamodes)
	tab = cga_crtc [modeindex];
    else
	tab = cga_crtc [kda.defmode];

    for (indx = 0; indx < CGA_NUMCTLREGS; indx++)
	kd_outb21 (CRT_CTRL, LOBYTE (indx), *tab++);

    kda.video_off = 1;			/* video is now off */
    kda.modeindex = modeindex;		/* remember mode we're in */
    DBG6 ( printf ("Done.\n"));
}

#ifndef	TABLE_BUG	/* compiler problems */
static char kd_alphamodes [] = {
    1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif
static char kd_biosmodes [] = {
    0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -1, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0xF, 0x10, 0, 1, 2, 3
};

static char kd_brokenmodes [] = {
    0, 1, 2, 3, 4, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0x10, 0xF, 0x10, 0, 1, 2, 3
};

/*
 * kd_alphamode
 *	Returns the fact that a mode is an alpha mode.
 *	can be sent biosmode or modeindex.
 */
kd_alphamode (biosmode)
    int biosmode;
{
    return kd_alphamodes [biosmode];
}

/*
 * kd_modeindex
 *	Determines the correct mode index for a given bios mode.
 *	biosmode = -1; advances to next valid mode.
 *	After return kd_biosmodes [modeindex] should be used to
 *	  set the biosmode that was determined by kd_modeindex.
 */
kd_modeindex (biosmode, curmode)
    int biosmode;
    int curmode;
{
    int modeindex;
    DBG6 (printf ("kd_modeindex: biosmode:%x curmode:%x ", 
	    biosmode, curmode));

    if (! (kda.ega || kda.hercules) && (kda.board & MONOBOARD))
	biosmode = kda.defmode;		/* don't allow mode change */

repeat:
    if (kda.ega) switch (modeindex = biosmode) {
	default:
	    biosmode = kda.defmode;
	    goto repeat;

	case -1:
	    curmode++;
	    if (curmode >= kd_numegamodes)	/* wrap mode back */
		curmode = 0;
	    else if (curmode == kd_egahercmode && !kda.hercules)
		goto repeat;
	    else if (kd_brokenmodes [curmode] == -1)	/* weed out invalid */
		goto repeat;

	    biosmode = curmode;
	    goto repeat;

	case 0: case 1: case 2: case 3:
	    if (kda.ecd)
		modeindex += kd_ecd;
	    break;

	case 4:   case 5:   case 6:   case 7:
	case 0xB: case 0xC: case 0xD: case 0xE:
	    break;

	case 0xF: case 0x10:
	    if (kda.egamem > 0x00)
		modeindex += kd_extm;
	    break;

	case 0x11:	/* hercules */
	    if (kda.hercules)
		modeindex = kd_egahercmode;
	    else
		modeindex = kda.modeindex;
	    break;
    }
    else {	/* we have a CGA */
	if (biosmode == -1) {
	    if (++curmode >= kd_numcgamodes)
		curmode = 0;
	    biosmode = curmode;
	}
	else if (biosmode >= kd_numcgamodes)
	    biosmode = kda.defmode;

	modeindex = biosmode;
    }
    DBG6 ( printf (" returning modeindex:%x\n", modeindex));
    return modeindex;
}

/*
 * Set the desired mode.
 * assumes that the page is the currently mapped page.
 */
kd_setmode (biosmode, merge)
    int biosmode;
    int merge;
{
    int modeindex;
    char *io_getbuf();

    modeindex = kd_modeindex (biosmode, kda.biosmode);
    biosmode  = kda.biosmode = kd_biosmodes [modeindex];
    if (kda.ega) {
#ifdef	REMOVE	/* Mode change from MERGE requires a direct request   */
		/* to cstp->gfxmove to save the font table since here */
		/* we don't know which device is being dealt with.    */
	/* save font table if not yet saved */
	if (cstp->gfxmove && !kda.fonttable
	&& (kda.fonttable = io_getbuf (VIDRAMLEN, 0))) {
	    cstp->gfxmove (1, 2, kda.fonttable, kda.modeindex);
	    kd_egainit (modeindex, kd_egamodes [modeindex]);
	    kda.curfont = 0;	/* assume the worst */
	}
#endif
    }
    else
	kd_cgainit (modeindex);

#ifdef	MERGE386
    kdscrmode = kda.biosmode;
#endif	/* MERGE386 */

    return biosmode;
}

/*
 * Set virtual console for new mode
 * The page reference may not necessarily be the currently mapped page.
 */
kd_newmode (cstp, biosmode, oldmode, setscreen)
    struct colr_state *cstp;
    int biosmode;
    int oldmode;
    int setscreen;
{
    int modeindex;
    char newalpha;
    char oldalpha;
    char modechange = 0;
    struct biosinitdata *initdata;
    char *io_getbuf();

    modeindex = kd_modeindex (biosmode, oldmode);
    biosmode  = kd_biosmodes [modeindex];
    newalpha  = kd_alphamodes [biosmode];
    oldalpha  = kd_alphamodes [oldmode];

    if ((MAPPED_ANY || setscreen) && (modeindex != kda.modeindex)) {
	if (newalpha != oldalpha) {
	    kd_savescreen ();			/* save current screen data */
	    if (!setscreen) setscreen = 1;
	}
	else if (!setscreen) setscreen = 2;
	modechange = 1;
    }
    MODEREG = cga_mode [modeindex];
    COLRREG = cga_colr [modeindex];

    if (kda.ega) {
	initdata = kd_egamodes [modeindex];
        if (newalpha) {
	    MAXSCRROW = initdata->rows;			/* set max row */
	    MAXSCRCOL = initdata->cols - 1;		/* set max column */
	}
	else {
	    if ( !cstp->modegraph || cstp->modegraph != modeindex) {
		cstp->modegraph = modeindex;
		*cstp->grafregs = *initdata;
	    }
	    initdata = cstp->grafregs;
	}
	cstp->ctrlregs = (char *) &initdata->c0;
    }
    else {
	if (modeindex == kd_cgahercmode)
	    cstp->ctrlregs = (char *) herc_crtc;
	else
	    cstp->ctrlregs = (char *) cga_crtc [modeindex];

	if (modeindex < 2)		/* if low res mode */
	    MAXSCRCOL = 39;		/* ... 40 columns max */
	else
	    MAXSCRCOL = 79;		/* ... 80 columns max */
	MAXSCRROW = 24;			/* all modes are 25 rows */
    }

    if (setscreen) {
	if (modechange) {
	    if (kda.ega) {
		if (newalpha) {
		    if (!kda.curfont && kda.fonttable) {
			if (!cstp->fonttable)
			    cstp->fonttable = kda.fonttable;
			DBG6 (printf ("kd: restoring font table, unit:%d.\n",
			    cstp->unit));
			if (cstp->gfxmove) {
			    cstp->gfxmove (0, 2, cstp->fonttable, modeindex);
			    kda.curfont = cstp->fonttable;
			}
		    }
		}
		else {
		    if (!kda.fonttable) { /* Save default font if necessary */
			if (!(kda.fonttable = io_getbuf(VIDRAMLEN,0)))
			    printf ("kd: no memory for font table\n");
			else {
			    DBG6 (printf ("kd: saving font table\n"));
			    if (cstp->gfxmove) {
				cstp->gfxmove (1, 2, kda.fonttable, modeindex);
				cstp->fonttable = kda.fonttable;
			    }
			}
		    }
		    kda.curfont = 0;	/* font table is presumed gone */
		    if (cstp->gfxrest)
			cstp->gfxrest (cstp);	/* restore graphics data */
		}
		kd_egainit (modeindex, initdata);
	    }
	    else
		kd_cgainit (modeindex);
	}

	/* if we need to restore the display data, do it */
	if (newalpha) {
	    if (setscreen != 2) {
		*(struct virtscreen *) kda.baseram = *cstp->vs;  
		cstp->screen = (struct virtscreen *) kda.baseram;
		if (kda.flkr)
		    FLKRREG = CRT_CTRL;
	    }
	    MAPPED_ANY = MAPIT_ALPHA;
	}
	else
	    MAPPED_ANY = MAPIT_GRAPHIC;

	kd_outmode (MODEREG, COLRREG, kd_mdfrc); /* and turn on video */
    }
    if (newalpha)
	kd_setscroll (cstp, MINROW, MAXROW);

    cstp->biosmode = biosmode;
    wakeup ((caddr_t) cstp);		/* wakeup "other" driver */
}

/*
 * Save current screen data
 */
kd_savescrdata (cstp)
    register struct colr_state *cstp;
{
    /* save screen data in virtual console save area */
    DBG6 (printf ("kd_savescr: saving data for %d %lx\n",cstp->unit,cstp->vs));
    *cstp->vs = *(struct virtscreen *) kda.baseram;
    cstp->screen = cstp->vs;
}

/*
* Save current state of the screen
*/
kd_savescreen ()
{
    int ret = kda.actpage;
    register struct colr_state *cstp;
    int x;

    if (cstp = kdc.cstp [ret]) {
	x = VIDLOCK;
	VIDLOCK = 1;
	if (MAPPED) {
	    MAPPED_ANY = 0;
	    kd_savescrdata (cstp);
	    FLKRREG = 0;
	    cursoff ();			/* turn off real cursor */
	}
	else if (MAPPED_GRAPHIC && cstp->gfxsave)
	    cstp->gfxsave (cstp);	/* save graphics data */

	if (cstp->lastactive != -1)
	    kd_deledev (cstp);		/* attempt to delete console */

	VIDLOCK = x;
	return ret;
    }
}

/*
 * Restore current screen data from save area
 */
kd_restscreen (unit)
    int unit;
{
    register struct colr_state *cstp;

    if (unit < NUMCONSOLES && (cstp = kdc.cstp [unit])) {
	DBG6 (printf ("kd_restscreen: unit:%d\n", unit));
	kd_newmode (cstp, cstp->biosmode, kda.biosmode, 1);
	kda.actpage = unit;
	kd_setcur (cstp, CROW, CCOL);
	wakeup ((caddr_t) cstp);
    }
}

#ifdef	REMOVE
kd_structcmp (a, b, len)
    char *a, *b;
    int len;
{
    asm ("	lds	6(%bp),%si		");
    asm ("	les	10(%bp),%di		");
    asm	("	mov	14(%bp),%cx		");
    asm	("	repz				");
    asm ("	scmpb	%ds:(%si),%es:(%di)	");
    asm ("	je	kd_seq			");
    asm ("	mov	$0,%ax			");
    asm	("	jmp	kd_sneq			");
    asm ("kd_seq:				");
    asm ("	mov	$1,%ax			");
    asm ("kd_sneq:				");
}
#endif

/* === */
