#ifndef lint
static char *uportid = "@(#)kd_merge.c	3.0 Microport Rev 2.3 10/1/87";
#endif

#ifdef ATMERGE
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/seg.h"

#ifdef MP386
#include "sys/fs/s5dir.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/cmn_err.h"
#include "sys/pump.h"
#include "sys/cio_defs.h"
#include "sys/vtoc.h"
#include "sys/open.h"
#include "sys/immu.h"
#else
#include "sys/dir.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/ppi.h"
#include "sys/mmu.h"
#include "sys/8259.h"
#endif

#include "sys/user.h"
#include "sys/conf.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/clock.h"
#include "sys/ioctl.h"

#include "sys/realmode.h"

#include "sys/kd_video.h"
#include "sys/kd.h"
#include "sys/kd_info.h"
#include "sys/setkey.h"
#include "sys/kd_color.h"

#define	BIOS_CRT_MODE	0x449

extern struct sw_data usw_data;
extern char dosok;
extern unsigned int doskb;
unsigned int kbsave;

extern struct kda_state kda;
extern struct kdc_state kdc;
extern int cga_colr [];

int u2dmap[] = { 0x01, 0x20, 0x40, 0x04, 0x08, 0x01, 0x04, 0x08, 0x02 };
int d2umap[] = { 0x00, CAPS_LOCK, NUM_LOCK, SCROLL_LOCK,
		 LEFT_ALT, LEFT_CTRL, LEFT_SHIFT, RIGHT_SHIFT };

/* check for switch screen key */
kb_chkswtch (tinsc, ki)
    unsigned char tinsc;
    struct kb_info *ki;
{
    register unsigned int code;

    if (dosok
    &&  tinsc != kda.lastchar			/* prevents auto repeat */
    &&  tinsc == usw_data.sw_swscrchar) {	/* insure it's a "make" */
	code = 0;
	if (kda.kb_state & ALTSET)		code |= 0x800;
	if (kda.kb_state & CTRLSET)		code |= 0x400;
	if (kda.kb_state & LEFT_SHIFT)		code |= 0x200;
	if (kda.kb_state & RIGHT_SHIFT)		code |= 0x100;
	if (code == (usw_data.sw_swscrchar & 0x7f00)) {
	    dspecialkb (tinsc << 8);
	    return 1;	/* indicate char was processed */
	}
    }
    return 0;	/* otherwise it wasn't what we wanted */
}

/*
 * Code to handle switchscreen and switchmode key
 *
 * Called at interrupt time.
 */
dspecialkb(code)
int code;
{
#ifdef	REMOVE		/* kda.lastchar prevents auto-repeat duplicates */
	static lastbreak = 1;
	static secondbreak = 0;    /* handles a break arriving before make */ 
	/* Only switch on first make after a break */
	/* so Dawson can't have any fun with typematic. */
	if ((code&0x8000)==0) {		/* "make" */
		if (lastbreak)
		{
#endif
		    if (usw_data.sw_modkeyhit)    
			switchmode();
		    else 
			swtchscr();
#ifdef	REMOVE
		}
		if (secondbreak) {
		    lastbreak = 1;
		    secondbreak = 0;
		} else { 
		    lastbreak = 0;
		}
	} else {				/* "break" */
		if (lastbreak)
                    secondbreak = 1;
                lastbreak = 1;
	}
#endif
}


set_dos_kb()
/* Map UNIX keyboard state into DOS. Set up LED's for DOS using */
/* UNIX state variables */
{
    int bit, indx;
    int doslocks = 0;
    
    usw_data.sw_kbstate = 0;
    for (bit=0x20,indx=3; bit; bit >>= 1, indx++) {
	if (kda.kb_state & bit)
	    usw_data.sw_kbstate |= u2dmap[indx];
    }

    kbsave = kda.kb_state & ~NONTOGGLES;       /* save lock state of unix kbd */
    for (bit=0x80, indx=0; indx<4; bit >>= 1, indx++) {
	if (doskb & bit)
	    doslocks |= d2umap[indx];
    }
    kda.kb_state = (kda.kb_state & NONTOGGLES) | doslocks;
    kb_sendleds ();
}

set_unix_kb()
/* Map DOS keyboard state into UNIX. Set up LED's */
{
    int bit, indx;

    kda.kb_table = K_NORMTAB;
    kda.kb_state = 0;
    for (bit=0x80,indx=0; bit ; bit >>= 1, indx++) {
	if (usw_data.sw_kbstate & bit)
	    kda.kb_state |= d2umap[indx];
    }
    /* restore lock states */
    kda.kb_state = (kda.kb_state & NONTOGGLES) | kbsave;
    kda.kb_table = ((kda.kb_state & SHIFTSET) ? K_SHIFTTAB : 0)
	     	 | ((kda.kb_state & ALTSET)   ? K_ALTTAB   : 0);
    kda.kb_leds = doskb >> 4;
    kb_sendleds();
}

vid_scroll (num)
    int num;
{
    struct colr_state *cstp = kdc.cstp[kda.actpage];
    int row = MAXROW + 1;

    while (--num >= 0) {
	kd_scroll (cstp, row, 1);
	kd_setcur (cstp, row, MINSCRCOL);
    }
}

video_off()
{
    if (!kda.ega) 
	kd_outb (BIOS_CRT_MODE, 1); 
    else { 
	kd_inb (CRT_STAT);
	kd_outb (0x3c0, 0); 
    } 
}

video_on()
{
    struct colr_state *cstp = kdc.cstp[kda.actpage];

    if (!kda.ega) 
	WRITEMODE(MODEREG);
    else { 
	kd_inb (CRT_STAT); 
	kd_outb (0x3c0, 0x20); 
    } 
}

unix_mode ()
{
    kd_restscreen (kda.unixunit);
}

dos_mode (biosmode)
    int biosmode;
{
    kda.unixunit = kd_savescreen ();

    if (biosmode != -1)
	return kd_setmode (biosmode, 1);
    else
	return kd_setmode (kda.biosmode, 1);
}

setvid (startscr, cursor, colors)
int startscr, cursor, colors;    
{
    if (startscr == -1)
	startscr = 0;          /* start register is never modified */
    if (cursor == -1)
	/* good */;
    if (colors == -1) 
        colors = cga_colr [kda.modeindex];

    COLROUT6845 (R_STARTADRH, startscr);
    COLROUT6845 (R_CURADRH, cursor);

    if (kda.board != MONOBOARD)
	WRITECOLOR (colors);
}

unsigned int
scrsize()
{
    /* have to truncate size to 16 bits for now */
    return ((unsigned int) kda.screensize);
}


readcursor()
{
    int x = spl7 ();
    unsigned int pos;

    pos = (kd_inb21 (CRT_CTRL, R_CURADRH) << 8)
	| (kd_inb21 (CRT_CTRL, R_CURADRL)     );
#ifdef	REMOVE
    kd_outb (CRT_CTRL, 0xf);
    pos = kd_inb (CRT_DATA);
    kd_outb (CRT_CTRL, 0xe);
    pos = (kd_inb (CRT_DATA) << 8) | (pos & 0xff);
#endif
    splx (x);
    return (pos);
}

/* switch screen mode */
switchmode()
{
    char *bcom_mod = gstokv (LOW_SEL(0)) + BIOS_CRT_MODE;
    unsigned int doscursor;

    doscursor = readcursor();
    *bcom_mod = kd_setmode (-1, 1);	/* inc mode to next */
    setvid (-1, doscursor, -1);
}

/* Set up DOS screen mode to be the same as in UNIX */
/* Set up cursor shape                              */
mode_setup ()
{
    char *bcom_mod = gstokv (LOW_SEL(0)) + BIOS_CRT_MODE;

    *bcom_mod = kda.biosmode;
    if (kda.modeindex < 4)
	COLROUT6845 (R_CURSTART, 0x0607);
    else
	COLROUT6845 (R_CURSTART, 0x0B0C);
}

#endif /* ATMERGE */

#ifdef COMMENT
/*
 * isconsole() tries to figure out if the current process
 * is on the console, which is harder than it might otherwise
 * be due to shell layers.
 * Returns 1 when on the console
 *         0 when on a (remote) tty
 *        -1 when no controlling tty
 */

/*	This needs to be checked with shell layers! */
isconsole()
{
struct	tty	*tp;
unsigned dev, unit;
#ifdef	SHELL_LAYERS
#include  "sys/sxt.h"
Link_p lp;
extern struct tty con_tty[];
extern conproc();
extern sxtopen();
extern Link_p linkTable[];
#endif	/* SHELL_LAYERS */

	dev =u.u_ttyd;
	unit = UNIT(minor(dev));

    /* minor device validation (taken from conopen() ) */
    /* Only allow one board, which is set at kernel start-up */

#ifdef REMOVE
	if (( dev >= (NUMPAGES * (NUMCRDS + 1))) || 
	    (( dev >= NUMPAGES) && (
	    (((kda.board & MONOBOARD) == MONOBOARD) && (dev/NUMPAGES != 1)) ||
	    (((kda.board & CGABOARD)  == CGABOARD)  && (dev/NUMPAGES != 2)) ||
	    (((kda.board & EGABOARD)  == EGABOARD)  && (dev/NUMPAGES != 3)) ||
	    (((kda.board & PGABOARD)  == PGABOARD)  && (dev/NUMPAGES != 4)))))
#else
	if (unit >= NUMCONSOLES)
		return(0);	/* Not on console ! */
#endif /* REMOVE */

	tp = &kd_tty[unit];
	if (tp->t_iscon)
		return(1);	/* On console ! */

#ifdef	SHELL_LAYERS
	if((cdevsw[major(dev)].d_open) == sxtopen)
	{                               /* In a shell layer */
		dev = minor(dev);
		lp = linkTable[LINK(dev)];
		if (lp->line->t_proc == conproc)	/* sxtproc??? */
			return(1);      /* Shell layer on console */
		else return(0);         /* Shell layer Not on console */
					/* Must be remote tty */
	}
#endif	/* SHELL_LAYERS */

	if (u.u_ttyp == NULL)
		return(-1);             /* No controlling tty */

	/* Not on console, and has a u.u_ttyp, so must be remote tty */
	return(0);
}
#else
isconsole()
{
    return (1);
}
#endif /* ATMERGE */

/* === */
