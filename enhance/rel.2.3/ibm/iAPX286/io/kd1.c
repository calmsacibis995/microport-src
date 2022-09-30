#ifndef lint
static char *uportid = "@(#)kd2.h	3.0 Microport Rev 2.3 10/1/87";
#endif

/* Modification History:
 * uport!mike	Wed Sep 30 13:24:07 PDT 1987
 *		Created.
 * uport!mide	Wed Sep 30 15:10:37 PDT 1987
 *		Moved kdmode from kd_ansi.c to here.
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

#ifdef	ATMERGE
#include "sys/realmode.h"
#endif

#ifndef	SMALLKD
#define	KDINIT	kdinit
#define	KDOPEN	kdopen
#define	KDCLOSE	kdclose
#define	KDREAD	kdread
#define	KDWRITE	kdwrite
#define	KDIOCTL	kdioctl
#define	KDINTR	kdintr
#define	KD_TTY	kd_tty

#else
#define	KDINIT	skdinit
#define	KDOPEN	skdopen
#define	KDCLOSE	skdclose
#define	KDREAD	skdread
#define	KDWRITE	skdwrite
#define	KDIOCTL	skdioctl
#define	KDINTR	skdintr
#define	KD_TTY	skd_tty
#endif

/* patchable parameters */
int colrsnow = 0;			/* flicker problem present */
int kdsavekeys = 1;			/* save keymaps after close */
int kb_ignrt = 1;			/* ignore special rt keys (101) */

/* state structures */
extern struct kda_state  kda;		/* adapter state structs */
extern struct kdc_state  kdc;		/* console state pointers */
extern struct tty KD_TTY  [];		/* tty structures */
extern int kd_numttys;			/* number consoles in config */

/*
 * KDINIT - Device Initialize
 */
KDINIT ()
{
    /* The following message should only be displayed if any of
     * the Microport Systems Corporation drivers are present.
     * For now, we assume that the kernel will use these drivers.
     */
#ifdef	MP386
    printf("System V/386 Drivers Copyright (c) 1986 Microport Systems Corp.\n");
#else
    printf("System V/AT Drivers Copyright (c) 1987 Microport Systems Corp.\n");
#endif
    printf("All Rights Reserved\n\n");
    if (kd_numttys > NUMCONSOLES) {
	kd_numttys = NUMCONSOLES;	/* trim number to MAX allowed */
	printf ("Configured number of consoles (%d) too many, reduced to %d\n",
	    kd_numttys, NUMCONSOLES);
    }
    return;
}

/*
 * KDOPEN - Open a device
 */
KDOPEN (dev, flag)
    dev_t dev;
    int flag;
{
    int kdproc ();
    int devc = minor (dev);
    int unit = UNIT (devc);
    register struct tty *tp;
    struct colr_state * cstp;

    /* Insure we're initialized.
     * (happens when kernel printf redirected to the tty port)
     */
    if (!kda.init)
	kdcinit ();

    DBG10 (printf ("kdopen: dev:%d flag:%d\n", dev, flag));

    /* minor device validation */
    if (unit >= NUMCONSOLES || unit >= kd_numttys) {
	u.u_error = EINVAL;
	return;
    }
    else if (kd_opendev (unit)) {
	u.u_error = ENOMEM;
	return;
    }
    else if (!(cstp = kdc.cstp [unit])) {
	u.u_error = ENODEV;
	return;
    }
    tp = &KD_TTY [unit];
    cstp->tty_tp = tp;		/* init struct tty pointer */

    /*
     * if we are not open, or not waiting for open
     * then we should initialize this tty
     */

    if (! (tp->t_state & (ISOPEN | WOPEN))) {
	ttinit (tp);
/* #ifdef MP386 */
	if (devc == 0) {				/* M003 */
	    tp->t_iflag = ICRNL|ISTRIP;			/* M003 */
	    tp->t_oflag = OPOST|ONLCR;			/* M003 */
	    tp->t_cflag = SSPEED|CS8|CREAD|HUPCL|CLOCAL;/* M003 */
	    tp->t_lflag = ECHO|ISIG|ICANON;		/* M003 */
	}
/* #endif MP386 */
	tp->t_proc = kdproc;
	kdparam (devc);
    }
    tp->t_state |= CARR_ON;

    /* execute the common open code for this line discipline */
    (*linesw [tp->t_line].l_open) (tp);
    cstp->openned = (cstp->openned & ~KDOPEN_INITFLAG) | KDOPEN_ALPHA;

#ifdef	REMOVE
    tp->t_iskd = 1;	/* removed pending study of 5.3 interrupts */
#endif
}

/*
 * KDCLOSE - Close a device
 */
KDCLOSE (dev)
    dev_t dev;
{
    int devc = minor (dev);
    int unit;
    struct tty *tp;
    struct colr_state * cstp;

    DBG10 (printf ("kdclose: dev:%d\n", dev));

    /* execute the common close code for this line discipline */
    unit = UNIT (devc);
    if (unit < kd_numttys && (cstp = kdc.cstp [unit])) {
	cstp->openned &= ~KDOPEN_ALPHA;
	tp = &KD_TTY [unit];
	(*linesw [tp->t_line].l_close) (tp);

#ifndef	SMALLKD
	kd_newmode (cstp, kda.defmode, cstp->biosmode, 0);
	if (!cstp->openned && cstp->gfxoff)
	    cstp->gfxoff (cstp);	/* remove the graphics buffers */
	if (!kdsavekeys)
	    kb_init (cstp);
	if (cstp->lastactive != -1)	/* if so requested		*/
	    kd_deledev (cstp);		/* attempt to delete this console */
#endif
	return ;
    }
    u.u_error = ENODEV;
}

/*
 * KDREAD - Read from a device
 */
KDREAD (dev)
    dev_t dev;
{
    int devc = minor (dev);
    int unit = UNIT (devc);
    struct tty *tp;

    if (unit < kd_numttys && (kdc.cstp [unit])) {
	/* execute the common read code for this line discipline */
	tp = &KD_TTY [UNIT (devc)];
	(*linesw [tp->t_line].l_read) (tp);
	return;
    }
    u.u_error = ENODEV;
}

/*
 * KDWRITE - Write to a device
 */
KDWRITE (dev)
    dev_t dev;
{
    int devc = minor (dev);
    int unit;
    struct tty *tp;
    struct colr_state *cstp;
    extern hz;

    unit = UNIT (devc);
    if (unit < kd_numttys && (cstp = kdc.cstp [unit])) {
	/* avoid possible contention bug here */
	/*
	while (cstp->inwrite)
	    delay (hz/5);
	*/
	cstp->inwrite = 1;
	/* execute the common write code for this line discipline */
	tp = &KD_TTY [unit];
	(*linesw [tp->t_line].l_write) (tp);
	cstp->inwrite = 0;
	return;
    }
    u.u_error = ENODEV;
}

/*
 * kdparam - Modify the state of the device as indicated in
 *      the tty structure
 */
kdparam (unit)
    int unit;
{
    register struct tty *tp = &KD_TTY [unit];
    int flags;	/* to ease access to c_flags */

    flags = tp->t_cflag;
    /* if baud rate is zero, turn off the line */
    if (! (flags & CBAUD)) {
	signal (tp->t_pgrp, SIGHUP);
	return;
    }
}

/*
 * kdproc - General command routine that initiates action
 */
kdproc (tp, cmd)
    struct tty *tp;
    int cmd;
{
    extern ttrstrt();
    int dev, unit;
    int x, now, intlevel = 0;
    register struct colr_state * cstp;
    unsigned char chr;
    int tmpcnt;
    caddr_t bufptr;
    ushort bufcnt;
    struct setkey sk;

    extern curlevel;				/* M002 */
    extern hz;

    unit = tp - &KD_TTY [0];
    if (! (cstp = kdc.cstp [unit]))	/* console is gone */
	return;
    now  = cmd & 0x80;
    cmd &= 0x7f;

    if (curlevel > 6)
	intlevel = spl6();

    /* based on cmd, do various things to the device */
    switch (cmd) {
	case T_TIME:            /* stop sending a break */
	    DBG2(printf("cmd = T_TIME\n"));
	    tp->t_state &= ~TIMEOUT;
	    goto start;

	case T_WFLUSH:          /* output flush */
	    DBG2(printf("cmd = T_WFLUSH\n"));
	    tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
	    tp->t_tbuf.c_count = 0;
	    /* FALL THROUGH */

	case T_RESUME:          /* enable output */
	    DBG2(printf("\ncmd = T_RESUME\n"));
	    tp->t_state &= ~TTSTOP;
	    /* FALL THROUGH */

	case T_OUTPUT:          /* do some output */
				/* M002 */
	    /* If re-entering, (from keybrd) quit */
	    if (VIDLOCK) {
		break;
	    }
start:	/* Up here so that we can switch pages during writes */
	    while ((tp->t_cflag & LOBLK) && (unit != kda.actpage)) {
		sleep( (caddr_t) cstp, PZERO + 1);
		/* lowest possible pri that sigs wake up */
	    }
	    /* If CGA card, consult colrsnow (patchable) */
	    if (colrsnow != kda.flkr)
		kd_updflkr ();
	    DBG2(printf("cmd = T_OUTPUT\n"));
	    {
		if (kda.nextpage != -1)
		    kdapset (kda.nextpage);
		SPL6(x);
		VIDLOCK = 1; 
		/*
		** if we are stopped, timed-out, or
		** just plain busy, don't do anything
		*/
		if (tp->t_state & (TIMEOUT | TTSTOP | BUSY)) {
		    VIDLOCK = 0;
		    SPLX(x);
		    break;
		}
		/* 386:310/tt1.c: assumes c_count never goes < 0 */
		if (tp->t_tbuf.c_count < 0) {
		    DBG2(printf("tp->t_tbuf.c_count < 0\n"));
		    tp->t_tbuf.c_count = 0;
		}
		if (tp->t_tbuf.c_ptr == NULL || tp->t_tbuf.c_count == 0) {
		    if (tp->t_tbuf.c_ptr)
			tp->t_tbuf.c_ptr -= tp->t_tbuf.c_size;
		    if (!(CPRES & (*linesw [tp->t_line].l_output)(tp))) {
			VIDLOCK = 0;
			SPLX(x);
			break;
		    }
		}
		tp->t_state |= BUSY;
		if (! ESCFLG) {
		    extern int kdputsx;
		    bufptr = tp->t_tbuf.c_ptr;
		    bufcnt = tp->t_tbuf.c_count;
		    SPLX(x);
		    if (kdputsx)
			tmpcnt = kd_asyputs (bufptr, bufcnt);
		    else
			tmpcnt = kd_puts (bufptr, bufcnt, cstp, 0);
		    SPL6(x);
		    /* if we got WFLUSH'd during write */
		    if (tp->t_tbuf.c_count == 0) {
			goto endwrite;
		    }
		    tp->t_tbuf.c_ptr += tmpcnt;
		    /* if we've finished clist */
		    if (0 == (tp->t_tbuf.c_count -= tmpcnt)) {
			goto endwrite;
		    }
		}
		while (ESCFLG) {
		    chr = *tp->t_tbuf.c_ptr++;
		    tp->t_tbuf.c_count--;
		    sysinfo.xmtint++;
		    SPLX(x);
		    DBG3 (printf (" ESCFLG=%d chr=%c ", ESCFLG, chr));

		    if (ESCFLG == 1 && chr < ' ') {	/* ESC x */
			kd_puts (&chr, 1, cstp, 1);
			ESCFLG = 0;
		    }
		    else {
			if (kda.actpage == cstp->unit && CRM_MODE) {
			    if (chr < ' ') {
				putchar ('^');
				putchar ('@' + chr);
			    }
			    else
				putchar (chr);
			}
			if (ESCFLG == 1)
			    kddoesc (chr, cstp);

			else if (ESCFLG == 6) {
			    tmpcnt = cstp->sk.k_len;
			    if (tmpcnt < ESCDATA [0])
				cstp->sk.k_data [tmpcnt++] = chr;

			    cstp->sk.k_len = tmpcnt;
			    if (tmpcnt == ESCDATA [0]) {
				kb_setmap (&cstp->sk, cstp);
				ESCFLG = 0;
			    }
			}
			else
			    kddoansi (chr, cstp);
		    }

		    SPL6(x);
		    if (tp->t_tbuf.c_count == 0)
			break;
		}
    endwrite:
		VIDLOCK = 0; 
		tp->t_state &= ~BUSY;
		SPLX(x);
		/* Honor SYSREQ key after char write finishes */
		if (kda.nextpage != -1)
		    kdapset (kda.nextpage);
		/* If a printing char is typed here, T_OUTPUT will be
		 * run at spl 7 - otherwise, it never is - c'est la vie,
		 * best I can do
		 */
		goto start;
	    }
	    break;

	case T_SUSPEND:         /* block on output */
	    DBG2(printf("\ncmd = T_SUSPEND\n"));
	    tp->t_state |= TTSTOP;
	    break;

	case T_BLOCK:           /* block on input */
	    DBG2(printf("cmd = T_BLOCK\n"));
	    break;

	case T_RFLUSH:          /* flush input */
	    DBG2(printf("cmd = T_RFLUSH\n"));
	    /* FALL THROUGH */

	case T_UNBLOCK:         /* enable input */
	    DBG2(printf("cmd = T_UNBLOCK\n"));
	    break;

	case T_BREAK:           /* send a break */
	    DBG2(printf("cmd = T_BREAK\n"));
	    tp->t_state |= TIMEOUT;
	    timeout( ttrstrt, tp, hz / 4 );
	    break;

	case T_SWTCH:           /* shell layer switch */
	    DBG2(printf("cmd = T_SWTCH\n"));
	    /*
	     * nothing for us to do
	     */
	    break;

	case T_PARM:            /* update parameters */
	    DBG2(printf("cmd = T_PARM\n"));
	    kdparam( unit );
	    break;

	default:
	    DBG2(printf("cmd = default = %x\n",cmd));
	    break;
    }
    if (intlevel)
	splx (intlevel);
}

/*
 * kdmode 
 *     handle the ESC[=nh sequence
 */
kdmode (cstp, mode, old, change)
    register struct colr_state * cstp;
    int mode, old, change;
{
#ifndef	SMALLKD
    if (mode == -1) {		/* ascii sequence to change modes */
	if (FAKEPOS)
	    kd_fakecursor (cstp, 0);	/* turn off fake cur */
	if (PARMPRES)
	    CNTESC++;
	if (CNTESC == 0) {			/* ESC[=h resets mode */
	    ATRBTE = FILL.at;
	    kd_newmode (cstp, kda.defmode, cstp->biosmode, 0);
	}
	else
	    kd_newmode (cstp, ESCDATA [CNTESC - 1], cstp->biosmode, 0);

	kd_setcur (cstp, CROW, CCOL);
    }
    else			/* internal change */
	kd_newmode (cstp, mode, old, change);

#endif
}


/*
** KDIOCTL
**      Call the common ioctl code, and
**      modify the state of the device if necessary
*/
#define	SK	parm.sk
#define	KE	parm.ke
#define	SIGK	parm.sigk
#define	SIGD	parm.sigd

KDIOCTL (dev, cmd, arg, mode)
    dev_t   dev;
    int     cmd;
    IOCTLARG arg;
    unsigned int mode;
{
    int unit = UNIT (minor (dev));
    struct colr_state *cstp;
    struct colr_state *cstp2;
    union {
	struct setkey sk;
	struct kbentry ke;
	struct setsigkey sigk;
	union sigkeydata sigd;
    } parm;

    if (!(cstp = kdc.cstp [unit])) {
	u.u_error = ENODEV;
	return;
    }

    switch (cmd) {
	case IOCSETKEY:			/* set keymap */
	    if (copyin (IOCTLCPTR (arg), (caddr_t) &SK, sizeof (SK)))
		u.u_error = EINVAL;
	    else if (kb_setmap (&SK, cstp))		/* M005 */
		u.u_error = ENODEV;
	    break;

	case IOCGETKEY:			/* get keymap */
	    if (copyin (IOCTLCPTR (arg), (caddr_t) &SK, sizeof (SK)))
		u.u_error = EINVAL;
	    else if (kb_getmap (&SK, cstp))		/* M005 */
		u.u_error = ENODEV;
	    else if (copyout ((caddr_t) &SK, IOCTLCPTR (arg), sizeof (SK)))
		u.u_error = EINVAL;
	    break;

	case IOCCLRKEY:			/* clear keymap */
	    if (kb_clearkm (cstp))
		u.u_error = ENODEV;
	    break;

#ifndef	SMALLKD
	case KDSETSIGKEY:		/* set signal key */
	    if (copyin (IOCTLCPTR (arg), (caddr_t) &SIGK, sizeof (SIGK)))
		u.u_error = EINVAL;
	    else if (kb_clrsigkey (&SIGK, cstp) || kb_setsigkey (&SIGK, cstp))
		u.u_error = ENODEV;
	    break;

	case KDGETSIGKEY:		/* get signal key data */
	    SIGD.key.data   = kda.sigdata;	/* user data */
	    SIGD.key.active = kda.sigactive;	/* active page when pressed */
	    if (copyout ((caddr_t) &SIGD, IOCTLCPTR (arg), sizeof (SIGD)))
		u.u_error = EINVAL;
	    break;

	case KDCLRSIGKEY:		/* clear signal key */
	    if (copyin (IOCTLCPTR (arg), (caddr_t) &SIGK, sizeof (SIGK)))
		u.u_error = EINVAL;
	    else if (kb_clrsigkey (&SIGK, cstp))
		u.u_error = ENODEV;
	    break;
#endif

	case KDGKBENT:			/* get table entry */
	    if (copyin (IOCTLCPTR (arg), (caddr_t) &KE, sizeof (KE)))
		u.u_error = EINVAL;
	    else if (kb_keyent (&KE, cstp, 0))
		u.u_error = ENODEV;
	    else if (copyout ((caddr_t) &KE, IOCTLCPTR (arg), sizeof (KE)))
		u.u_error = EINVAL;
	    break;

	case KDSKBENT:			/* set table entry */
	    if (copyin (IOCTLCPTR (arg), (caddr_t) &KE, sizeof (KE)))
		u.u_error = EINVAL;
	    else if (kb_keyent (&KE, cstp, 1))
		u.u_error = ENODEV;
	    break;

	case KDGKBMODE:			/* get the keyboard mode */
	    u.u_rval1 = (int) (cstp->kb_xlate);
	    break;
	    
	case KDSKBMODE:			/* set the keyboard mode */
	    cstp->kb_xlate = IOCTLINT (arg);
	    break;

#ifndef	SMALLKD
	case KDDELECONS:		/* delete console */
	    DBG12 (printf ("KDDELECONS: console:%d\n", IOCTLINT (arg)));
	    if (IOCTLINT (arg) >= NUMCONSOLES)
		u.u_error = EINVAL;
	    else if (!(cstp = kdc.cstp [IOCTLINT (arg)]))
		u.u_error = ENODEV;
	    else if (kd_deledev (cstp))
		u.u_error = EBUSY;
	    break;

	case KDADDCONS:			/* add a console */
	    DBG12 (printf ("KDADDCONS: console:%d\n", IOCTLINT (arg)));
	    if (IOCTLINT (arg) >= NUMCONSOLES)
		u.u_error = EINVAL;
	    else if (kd_opendev (IOCTLINT (arg)))
		u.u_error = ENOMEM;
	    break;

	case KDCHANGECONS:		/* change console */
	    DBG12 (printf ("KDCHANGECONS: console:%d\n", IOCTLINT (arg)));
	    u.u_rval1 = kda.actpage;
	    if ((IOCTLINT (arg) != -2) && (kdapset (IOCTLINT (arg))))
		u.u_error = ENODEV;
	    break;

	case KDSETLASTACTIVE:		/* set last active console */
	    if (copyin (IOCTLCPTR (arg), (caddr_t) &SIGD, sizeof (SIGD)))
		u.u_error = EFAULT;
	    else {
		DBG12 (printf ("KDSETLASTACTIVE: parent:%d child:%d\n",
		    SIGD.link.parent, SIGD.link.child));

		if ((SIGD.link.parent >= NUMCONSOLES)
		||  (SIGD.link.child  >= NUMCONSOLES))
		    u.u_error = EINVAL;
		else if (! (cstp = kdc.cstp [SIGD.link.child ] )
		     ||  !         kdc.cstp [SIGD.link.parent] )
		    u.u_error = ENODEV;
		else {
		    if (!cstp->openned)
			 cstp->openned = KDOPEN_INITFLAG;
		    cstp->lastactive = SIGD.link.parent;
		}
	    }
	    break;

	case KDSETPARENT:		/* set my parent */
	    DBG12 (printf ("KDSETPARENT: unit:%d parent:%d\n",
		unit, IOCTLINT (arg)));
	    if (IOCTLINT (arg) >= NUMCONSOLES)
		u.u_error = EINVAL;
	    else if (!kdc.cstp [IOCTLINT (arg)])
		u.u_error = ENODEV;
	    else
		cstp->lastactive = IOCTLINT (arg);
	    break;

	case KDSETCHILD:		/* set my child */
	    DBG12 (printf ("KDSETCHILD: unit:%d child:%d\n",
		unit, IOCTLINT (arg)));
	    if (IOCTLINT (arg) >= NUMCONSOLES)
		u.u_error = EINVAL;
	    else if (!(cstp = kdc.cstp [IOCTLINT (arg)]))
		u.u_error = ENODEV;
	    else
		cstp->lastactive = unit;
	    break;

	case KDCLOSEGRAPHICS:		/* shut down graphics */
	    DBG12 (printf ("KDCLOSEGRAPHICS: console:%d\n", IOCTLINT (arg)));
	    if (IOCTLINT (arg) != -1) {
		if (IOCTLINT (arg) >= NUMCONSOLES)
		    u.u_error = EINVAL;
		else if (!(cstp = kdc.cstp [IOCTLINT (arg)])) {
		    u.u_error = ENODEV;
		    break;
		}
	    }
	    else if (cstp->gfxoff)
	        cstp->gfxoff (cstp);
	    break;
#endif

	default:
	    /*
	    ** if the ioctl is one in which the parameters of
	    ** the chip have to be changed, change them
	    */
	    if (ttiocom (&KD_TTY [unit], cmd, arg, mode))
		kdparam (unit);
    }
}

int kdintcnt = 0;
/* service keyboard interrupts */
KDINTR (vector)
    int vector;
{
    unsigned char inchar;
    unsigned char *instr, *kb_proc();
    unsigned char cnt;
    unsigned short timer;
    struct colr_state *cstp;

#ifdef	REMOVE
    if (vector == 41) {
	extern struct biosinitdata *kd_egamodes [];
	kdintcnt++;
	inchar = kd_egamodes [kda.modeindex]->c11;
	kd_outb21 (CRT_CTRL, 0x11, inchar & ~0x10 | 0x20);
	kd_outb21 (CRT_CTRL, 0x11, inchar & ~0x20 | 0x10);
	return;
    }
#endif
	
    inchar = kd_inb (KEYBD_STAT);
    if (! (inchar & KB_OUTBF))		/* no data available */
	return;

    kb_disable();			/* disable the keyboard interface */

    /* read the character form the data port */
    inchar = kd_inb (KEYBD_OUT);	/* read data from keybd out port */
    DBG4 (printf (":%x ", inchar));

    /* check for keyboard controller control character */
    switch (inchar) {
	case KB_RESEND:			/* character was a resend command */
	    kda.kb_status |= KB_RES_FLG; /* mark resend in the status byte */
	    goto enable;

	case KB_ACK:			/* character was an acknowledge cmd */
	    kda.kb_status |= KB_ACK_FLG; /* update the mode indicator */
	    goto enable;

	case 0xE0:			/* 101 keyboard, extended code flag */
	case 0xE1:			/* 101 keyboard, extended code flag */
	    if (!kb_ignrt)		/* if ignore not set */
		kda.lastchar = inchar;	/* remember this character */
	    goto enable;

	case 0xFF:			/* character was input overrun code */
	    goto enable;
    }

    /* if this console has been removed, send this data to the system console */
    if (!(cstp = kdc.cstp [kda.actpage]))
	  cstp = kdc.cstp [0];

#ifdef	ATMERGE
    /* check for the switch screen character */
    if (kb_chkswtch (inchar, cstp)) {
	kda.lastchar = inchar;
	goto enable;
    }
#endif

    /* character is a data character, decode it */
    instr = kb_proc (inchar, cstp);

    /* If in raw mode, send the scan code to the user */
    if (cstp->kb_xlate == K_RAW)
	keyinstr (cstp, &inchar, 1);

    /* otherwise, send the translated ascii sequence */
    else if (cnt = *instr++)
	keyinstr (cstp, instr, cnt);

    /* update scroll status if needed */
    if (KD_TTY [kda.actpage].t_state & TTSTOP)
	kda.kb_state |= SCROLL_LOCK;
    else
	kda.kb_state &= ~SCROLL_LOCK;

    /* One last check for led updating */
    kb_sendleds ();

    /* Pauhana! */
enable:
    kb_enable ();
}

/* increment console number wrapping at end */
#define	NXTCONS(c)	((c + 1) % NUMCONSOLES)

/*
 * Set new console
 */
kdapset (unit)
    int  unit ;
{
#ifndef	SMALLKD
    register struct colr_state *cstp;

    DBG6 (printf ("kdapset: unit:%d\n", unit));
    if (unit == -1) {				/* switch to next console */
	for (unit=NXTCONS(kda.actpage); unit != kda.actpage; unit=NXTCONS(unit))
	    if (kdc.cstp [unit])
		break;
    }
    else if (unit >= NUMCONSOLES || !(kdc.cstp [unit]))
	return 1;

    /* if switching to current page (ALT-F[1-4] do this) */
    /* if this code is not here, switching windows during output reboots */
    if (unit == kda.actpage || !(cstp = kdc.cstp [kda.actpage]))
	return 0;

    /* if currently writing to current screen, mark it and return */
    if (VIDLOCK || cstp->graphlock || kdc.cstp [unit]->vidlock) {
	if (kda.nextpage == -1) /* if already one pending, ignore this one */
	    kda.nextpage = unit;
	return 0;
    }
    kd_savescreen ();		/* save current screen */
    kd_restscreen (unit);	/* restore new screen */
    kda.nextpage = -1;
#endif

    return 0;
}

/*===*/
