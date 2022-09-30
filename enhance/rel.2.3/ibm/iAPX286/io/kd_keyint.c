#ifndef	lint
static char *uportid = "@(#)kd_keyint.c	3.0 Microport Rev 2.3 10/1/87";
#endif

/*		Copyright 1985 Microport Systems
 *		All Rights Reserved.
 *
 *	Modification History:
 *	Initial coding: Wed Sep 18 08:11:03 PDT 1985
 *  This is the keyboard interface routine for the AT machine.
 *
 * M000: lance 3-11-86
 *  	Changed for monolithic screen driver.
 * M001: lance 4-3-86
 *		Changed for reboot key-combo.
 * M002: lance 5-4-86
 *		Changed to fix numeric-keyboard bug brought on by F-key mapping.
 * M003: mike	10-30-86
 *		Allow key mapping on keypad.
 *
 * M004: uport!mike Tue Feb 10 11:21:59 PST 1987
 *		added multiple keymaps and default keymaps.
 *
 * M003/386: lance 9-1-86  Removed eoi() for 386 port
 *
 * M005: uport!mike Thu Feb 19 09:30:26 PST 1987
 *		merged in changes from the 5.3/386 port
 *
 * M006 uport!mike Tue Mar 31 14:33:02 PST 1987
 *	Merge in ATMERGE changes from LCC
 *
 * M007 uport!mike Mon Apr 13 14:23:55 PST 1987
 *	Split into files "kb.c, kb_data.h, kb_info.h, kb_keymap.c, kb_merge.c,
 *	    kb_proc.c";
 *	Updated for 5.3 specification;
 *	New keymapping strategy.
 */

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
#include "sys/kd_video.h"
#include "sys/sysmacros.h"

#include "sys/kd.h"
#include "sys/kd_info.h"
#include "sys/setkey.h"
#include "sys/kd_color.h"

#define	DISABLE_KEYBD		/* turn on disable flag */
#define	KB_ENABKB	0xF4

int kb_bufwt = 0;		/* statistics for debugging */
int kb_ackwt = 0;

extern struct kda_state kda;
extern struct kdc_state kdc;

/* initialize keyboard info struct */
kb_init (cstp)
    struct colr_state *cstp;
{
    extern struct keydata kb_std_data [];
    extern unsigned char kb_std_control [];
    extern unsigned short kb_break_data [NUM_BREAKKEYS] [2];
    short unit = cstp->unit;
    unsigned short temp;
    short index, table;

    if (unit < 10) {
	index = F1_SCANCODE + unit;
	table = K_ALTTAB;
    }
    else {
	index = F1_SCANCODE + (unit - 10);
	table = K_ALTSHIFTTAB;
    }

    temp = kb_std_data [index].data [table];
    if ((temp & TYPEMASK) != BREAKKEY) {
        kb_std_data [index].data [table]  = CONS0_KEY + unit;
	kb_break_data [CONS_0 + unit] [0] = CONS0_KEY + unit;
	kb_break_data [CONS_0 + unit] [1] = temp;
    }
    cstp->kb_xlate   = K_XLATE;
    cstp->kb_data    = kb_std_data;
    cstp->kb_control = kb_std_control;
    kb_clearkm (cstp);
}

/* kb_sendctl
**     Send data or commands to the keyboard controller
*/
kb_sendctl (data)
    unsigned char   data;
{
    unsigned char   tmp;	/* for the status */
    unsigned short  timer;

    asm (" cli");		/* disable interrupts */
    timer = 0xFFFF;
    do {			/* wait for transmit buffer empty */
	tmp = kd_inb (KEYBD_STAT);
	kb_bufwt++;
    } while ((tmp & KB_INBF) && --timer);
    kb_bufwt--;

    kd_outb (KEYBD_ICMD, data);	/* send data to command port */
    asm (" sti");		/* re-enable interrupts */
}

/* Disable the keyboard interface and wait for the command to be accepted */
kb_disable ()
{ 
#ifdef	DISABLE_KEYBD
    int timer, instat;

    kb_sendctl (KB_DISABKB);
    timer = 0x2000;
    do {
	instat = kd_inb (KEYBD_STAT);
	kb_bufwt++;
    } while ((instat & KB_INBF) && (--timer > 0));
    kb_bufwt--;
#endif
}

/* Enable the keyboar interface */
kb_enable()
{
#ifdef	DISABLE_KEYBD
    kb_sendctl (KB_ENAB);
#endif
}

/*
 * kb_sendkybd
 *     Transmit command and data bytes to the keyboard.
 *     Recieves and handles acknowledment.
 *     Retries if necessary.
 */
kb_sendkybd (data)
    unsigned char   data;	/* the data or command to send */
{
    int tmdly;			/* temporary used to establish timeouts */
    unsigned short  retry;	/* retry count */
    unsigned short  timer;
    unsigned char   tin;

    retry = RETRYCNT;
    do {
	asm (" cli");		/* clear interrupts		*/
				/* clear ack and resend flags	*/
	kda.kb_status &= ~(KB_ACK_FLG | KB_RES_FLG);
	timer = 0xFFFF;
	do {			/* wait for transmit buffer empty */
	    tin = kd_inb (KEYBD_STAT);
	    kb_bufwt++;
	} while ((tin & KB_INBF) && --timer);
	kb_bufwt--;
	    
	kd_outb (KEYBD_OUT, data);	/* send data */
	asm (" sti");		/* enable interrupts */
	tmdly = 0x2000;		/* set the time delay for 10 mSeconds */

	/* wait for response */
	while ((! (kda.kb_status & (KB_ACK_FLG | KB_RES_FLG))) && --tmdly)
	    kb_ackwt++;

	/* see if this is an acknowldege */
	if (kda.kb_status & KB_ACK_FLG)
	    return;			/* yes, good transission */

    } while (--retry);
    
    /* too many retries, set error flag */
    asm (" cli");
    kda.kb_status |= KB_ERR_FLG;
    asm (" sti");
}

/* kb_sendleds 
**     send the LEDs mode data to the keyboard
*/
kb_sendleds ()
{
    int x;
    int leds;

    asm (" cli");

    /* If update is underway skip this one */
    if (kda.kb_status & KB_LED_FLG) {
	asm (" sti");
	return;
    }
    leds = 0;
    if (kda.kb_state & CAPS_LOCK)	leds |= LED_CAP;
    if (kda.kb_state & NUM_LOCK)	leds |= LED_NUM;
    if (kda.kb_state & SCROLL_LOCK)	leds |= LED_SCROLL;

    /* if no change, exit now */
    if (leds == kda.kb_leds) {
	asm (" sti");
	return;
    }
    /* there is a change, so update the leds */
    kda.kb_status |= KB_LED_FLG;	/* turn on update in process flag */
    asm (" sti");
    x = spl5();
    kb_sendkybd (LED_WARN);	/* send the LED comand to keyboard */

    /* check for a transmission error */
    if (kda.kb_status & KB_ERR_FLG) {
	/* if error, send keyboard enable, */
	/* update error flag and mode indicator */
	kb_sendkybd (KB_ENABKB);
    }
    else {
	/* send the led data byte */
	kb_sendkybd (leds);

	/* test for transmission error */
	if (kda.kb_status & KB_ERR_FLG) {
	    /* if error, send keyboard enable, */
	    /* update error flag  and mode indicator */
	    kb_sendkybd (KB_ENABKB);
	}
	asm (" cli");
	kda.kb_leds = leds;
	asm (" sti");
    }
    asm (" cli");
    kda.kb_status &= ~(KB_ERR_FLG | KB_LED_FLG);
    asm (" sti");
    splx (x);
}


/*
 * Process input characters checking for
 * XON, XOFF, stripping parity and inserting data into
 * the input queue.
 *
 * *special* case: if input char == 0, then toggle XON, XOFF
 */
keyinstr (cstp, tinchar, nchar)
    struct colr_state *cstp;
    unsigned char  *tinchar, nchar;
{
    int x, indx;		/* for interrupt */
    struct tty *tp;
    unsigned char   ctmp;
#ifndef MP386
    extern int ualock;		/* M001 */

    if (ualock)
	return;			/* M001 */
#endif

    x = spl6 ();
    sysinfo.rcvint++;

    tp = cstp->tty_tp;

    /* if we aren't open, nobody to give the char to, so chuck it. */
    if (!(tp->t_state & (ISOPEN | WOPEN))) {
	splx (x);
	return;
    }
    /* if nchar = 0, then this is a toggle XON,XOFF cmd */
    if (nchar)
	ctmp = *tinchar & 0x7F;
    else
	ctmp = 0xFF;

    /* if we are supposed to do xon/xoff protocol, and the char we got is one
     * of those guys, take care of it. 
     */
    if (tp->t_iflag & IXON) {

	/* if we are stopped, and if the char is a xon, or if we should
	 * restart on any char, resume 
	 */
	if (tp->t_state & TTSTOP) {
	    if (ctmp == 0xFF || ctmp == CSTART || (tp->t_iflag & IXANY)) {
		kdproc (tp, T_RESUME);
		splx (x);
		return;
	    }
	}
	/* if char is a XOFF, suspend I/O */
	else {
	    if (ctmp == 0xFF || ctmp == CSTOP) {
		kdproc (tp, T_SUSPEND);
		splx (x);
		return;
	    }
	}
	if (ctmp == 0xFF || ctmp == CSTART || ctmp == CSTOP) {
	    splx (x);		/* get more chars */
	    return;
	}
    }
    /* if it was a "toggle xon, xoff" character, then return */
    if (ctmp == 0xFF) {
	splx (x);
	return;
    }

    /* if we are supposed to strip char to 7 bits, do it */
    /* note: we only strip the first character */
    if (tp->t_iflag & ISTRIP)
	ctmp = *tinchar & 0x7F;
    else
	ctmp = *tinchar;

    /* if we don't have anywhere to put the char, continue */
    if (tp->t_rbuf.c_ptr == NULL) {
	splx (x);
	return;
    }
    if (tp->t_rbuf.c_count < nchar)
	(*linesw[tp->t_line].l_input) (tp, L_BUF);

    for (indx = 0; indx < nchar; indx++) {
	/* stuff the char into buffer and signify that we have a char */
	tp->t_rbuf.c_ptr [indx] = *tinchar++;
	tp->t_rbuf.c_count--;
    }
    (*linesw[tp->t_line].l_input) (tp, L_BUF);

    splx (x);
    return;
}

/* === */
