#ifndef lint
static char *uportid = "@(#)kd_keymap.c	3.0 Microport Rev 2.3 10/1/87";
#endif

#include "sys/types.h"
#include "sys/kd_video.h"
#include "sys/kd.h"
#include "sys/kd_info.h"
#include "sys/setkey.h"
#include "sys/kd_color.h"
#include "sys/kd_data.h"

extern struct kda_state kda;
extern struct kdc_state kdc;
unsigned char kb0_keymap [NUMSETCHARS];	/* unit 0 and system default keymap */
int kb_testdos = 0;

/* layout of keymap data:
 *    [0] = code
 *    [1] = shift state
 *    [2] = length of replacement string
 *    [3] = beginning of replacement string
 */
#define	KM_CODE		0
#define	KM_SHIFT	1
#define	KM_LEN		2
#define	KM_STRING	3
#define	KM_OVHD		3		/* overhead */

kb_clearkm (cstp)
    struct colr_state *cstp;
{
    if (cstp->unit == 0) {
	cstp->kb_keymap = kb0_keymap;
	kda.signext = 0;
    }
    cstp->kb_nextkm = cstp->kb_keymap;
    cstp->kb_lastkm = cstp->kb_keymap + sizeof (kb0_keymap);
    cstp->kb_locd = 255;
    cstp->kb_hicd = 0;
    return 0;
}  

struct setsigkey *
kb_lkupsigkey (code, shift, cstp)
    unsigned char code;
    unsigned char shift;
    struct colr_state *cstp;
{
    struct setsigkey *sigk;

    for (sigk = kda.sigmap; sigk < &kda.sigmap [kda.signext]; sigk++)
	if (sigk->k_code == code && sigk->k_shift == shift)
	    return sigk;
    return (struct setsigkey *) 0;
}

#define	SK_LEN(a,b)	((int) ((long) (a) - (long) (b)))

kb_clrsigkey (sigk, cstp)
    struct setsigkey *sigk;
    struct colr_state *cstp;
{
    struct setsigkey *dk;

    if (sigk->k_code == 0)		/* clear all signals */
	kda.signext = 0;
    else if (dk = kb_lkupsigkey (sigk->k_code, sigk->k_shift, cstp)) {
	bcopy (dk + 1, dk, SK_LEN (kda.sigmap + kda.signext, dk + 1));
	kda.signext--;
    }
    return 0;
}

kb_setsigkey (sigk, cstp)
    struct setsigkey *sigk;
    struct colr_state *cstp;
{
    if (kda.signext < NUMSIGKEYS) {
	kda.sigmap [kda.signext++] = *sigk;
	return 0;
    }
    return 1;
}

unsigned char *
kb_lkup (code, shift, cstp, def)
    unsigned char code, shift;
    struct colr_state *cstp;
    unsigned char def;
{
    unsigned char *km, *nxtkm;

    while (1) {
	/* check for keymap and code within stored limits */
	if ((km = cstp->kb_keymap)
	&&  cstp->kb_locd <= code && code <= cstp->kb_hicd) {
	    while (km < cstp->kb_nextkm) {
		nxtkm = km + km [KM_LEN] + KM_OVHD;
		if ((code == *km++) && (shift == *km++))
		    return km;
		km = nxtkm;
	    }
	}
	if (!def || !cstp->unit)
	    break;
	def = 0;
	cstp = kdc.cstp [0];
    }
    return 0;
}

/*
 * Add new keymap string
 */
kb_setmap (skptr, cstp)		/* M004 */
    struct setkey *skptr;
    struct colr_state *cstp;
{
    unsigned char *km, *from;
    unsigned short len;
    unsigned short dellen = 0;

    if (skptr->k_len) {			/* adding code */
	if (!cstp->kb_keymap) {		/* need to allocate keymap */
	    if (len = io_expbuf (cstp, sizeof (kb0_keymap), 0)) {
		cstp->kb_keymap = (unsigned char *) cstp + len;
		kb_clearkm (cstp);
	    }
	    else
		return 1;
	}
	if (km = kb_lkup (skptr->k_code, skptr->k_shift, cstp, 0))
	    dellen = *km + KM_OVHD;

	len = skptr->k_len + KM_OVHD - dellen;
	if ((cstp->kb_nextkm + len) < cstp->kb_lastkm) {	/* fits */
	    if (dellen) {
		from = km + *km + 1;
		bcopy (from, km - KM_LEN, cstp->kb_nextkm - from);
		cstp->kb_nextkm -= dellen;
	    }
	    *cstp->kb_nextkm++ = skptr->k_code;
	    *cstp->kb_nextkm++ = skptr->k_shift;
	    *cstp->kb_nextkm++ = skptr->k_len;
	    bcopy (skptr->k_data, cstp->kb_nextkm, skptr->k_len);
	    cstp->kb_nextkm += skptr->k_len;

	    /* update code limits */
	    if (cstp->kb_locd > skptr->k_code)
		cstp->kb_locd = skptr->k_code;
	    if (cstp->kb_hicd < skptr->k_code)
		cstp->kb_hicd = skptr->k_code;

	    return 0;
	}
	return 1;
    }
    /* just deleting a code */
    if (km = kb_lkup (skptr->k_code, skptr->k_shift, cstp, 0)) {
	dellen = *km + KM_OVHD;
	/* delete current keymap string */
	from = km + *km + 1;
	bcopy (from, km - KM_LEN, cstp->kb_nextkm - from);
	cstp->kb_nextkm -= dellen;
    }
    return 0;
}

kb_getmap (skptr, cstp)		/* M004 */
    struct setkey  *skptr;
    struct colr_state *cstp;
{
    unsigned char *km;
    short len;

    if (km = kb_lkup (skptr->k_code, skptr->k_shift, cstp, 1)) {
	len = *km++;
	bcopy (km, skptr->k_data, len);
	skptr->k_len = len;
	return 0;
    }
    skptr->k_len = 0;
    return 0;
}

unsigned char *
kb_proc (c, cstp)
    unsigned char c;
    struct colr_state *cstp;
{
    unsigned char mch = c & 0x7F;		/* masked character */
    struct keydata *kd = &cstp->kb_data [mch];	/* keybrd data ptr */
    unsigned short table = kda.kb_table & 0x03;	/* which table we're using */
    unsigned short keycode = kd->data [table];	/* xlat'd data */
    union {
	unsigned char *kb;			/* return data */
	struct setsigkey *sigk;
    } map;
    unsigned char gotctrl = 0;
    DBG5 (printf ("  shift:%x keycode:%x ", kda.kb_state, keycode));
    cstp->kb_buff [0] = '\0';		/* default = zero char return */

    if (!(c & 0x80)) {			/* "make" code ? */

again:
	if ((keycode & NUMLCK) && (kda.kb_state & NUM_LOCK)) {
	    table ^= K_SHIFTTAB;
	    keycode = kd->data [table];
	}
	if ((keycode & CAPLCK) && (kda.kb_state & CAPS_LOCK)) {
	    table ^= K_SHIFTTAB;
	    keycode = kd->data [table];
	}
	if (!gotctrl && ((keycode & TYPEMASK) == BREAKKEY)) {
	    keycode = kb_break_data [keycode & 0xFF] [kda.kb_state & CTRLSET ?1:0];
	    table = kda.kb_table & 0x03;
	    gotctrl = 1;
	    goto again;
	}
	if (kda.kb_state & CTRLSET) {
	    table |= K_CTRLTAB;
	    if (keycode & CTLKEY)
		keycode = keycode & 0xFF00 | cstp->kb_control [mch];
	}

	/*
	 * If in xlate mode, check if character has been remapped
	 * and if so, returned mapped string.
	 * Also check if this is a "signal" key, and send the signal
	 * if so.
	 */
	DBG5 (printf (" table:%d keycode:%x\n", table, keycode));

	if (cstp->kb_xlate == K_XLATE) {
	    if (map.kb = kb_lkup (mch, table, cstp, 1)) {
		kda.lastchar = c;
		return map.kb;
	    }
	    if (map.sigk = kb_lkupsigkey (mch, table, cstp)) {
		kda.sigdata   = map.sigk->k_data;
		kda.sigactive = cstp->unit;
		signal (map.sigk->k_pid, map.sigk->k_signal);
		kda.lastchar = c;
		return (unsigned char *) cstp->kb_buff;
	    }
	}

	map.kb = (unsigned char *) cstp->kb_buff;	/* data pointer */

	switch (keycode & TYPEMASK) {
	    case NORMKEY:
		*map.kb++ = 1;
		*map.kb = keycode & 0xFF;
		break;

	    case SHIFTKEY:
		if (c != kda.lastchar) {
		    if (keycode & NONTOGGLES) {
			kda.kb_state |= keycode & 0xFF;
			kda.kb_table = 
			    ((kda.kb_state & SHIFTSET) ? K_SHIFTTAB : 0)
			  | ((kda.kb_state & ALTSET)   ? K_ALTTAB   : 0);
		    }
		    else		/* is a toggled shift type */
			kda.kb_state ^= keycode & 0xFF;
		}
		break;

	    case BREAKKEY:
		switch (keycode & 0xFF) {

		    case SYS_REQUEST:
			/* maybe should spawn a proc to change pages? */
			kdapset (-1);
			break;

		    case SYS_REBOOT:
			kerndebug ();
			break;

		    case SCROLL_REQUEST:	/* toggle XON, XOFF */
			if (cstp->kb_xlate != K_RAW)
			    keyinstr (cstp, (unsigned char *) 0, 0);
			break;

		    case DEBUG_REQUEST:
			if (cstp->kb_xlate != K_RAW) {
			    kb_enable ();
			    dbg_enter ();	/* call debugger */
			    kb_disable ();
			}
			break;

		    case DOS_SWITCH:		/* switch dos screen */
			if (c != kda.lastchar)
#ifdef	ATMERGE
			    dspecialkb ();
#endif
			break;

		    default:
			keycode &= 0xFF;
			if (CONS_0 <= keycode && keycode < CONS_0 + NUMCONSOLES)
			    /* Switch to addressed virtual screen */
			    if (cstp->kb_xlate != K_RAW)
				kdapset (keycode - CONS_0);
			break;
		}
		break;

	    case SS2PFX:
		*map.kb++ = 3;
		*map.kb++ = '\033';
		*map.kb++ = 'N';
		*map.kb = keycode & 0xFF;
		break;

	    case SS3PFX:
		*map.kb++ = 3;
		*map.kb++ = '\033';
		*map.kb++ = 'O';
		*map.kb = keycode & 0xFF;
		break;

	    case CSIPFX:
		*map.kb++ = 3;
		*map.kb++ = '\033';
		*map.kb++ = '[';
		*map.kb = keycode & 0xFF;
		break;

	    case CS2PFX:
		*map.kb++ = 4;
		*map.kb++ = '\033';
		*map.kb++ = '[';
		*map.kb++ = '!';
		*map.kb = keycode & 0xFF;
		break;

	    case DOSKEY:
		*map.kb++ = 2;
		*map.kb++ = '\0';
		*map.kb = keycode & 0xFF;
		break;

	    case NOKEY:
	    default:
		break;
	}
    }
    else {					/* key was a "break" code */
	if (((keycode & TYPEMASK) == SHIFTKEY)
	&&  (keycode & NONTOGGLES)
	&&  (c != kda.lastchar)) {
	    kda.kb_state &= ~(keycode & 0xFF);
	    kda.kb_table = ((kda.kb_state & SHIFTSET) ? K_SHIFTTAB : 0)
		         | ((kda.kb_state & ALTSET)   ? K_ALTTAB   : 0);
	}
    }
    kda.lastchar = c;
    return (unsigned char *) cstp->kb_buff;
}

#ifdef	DOSKEYS
/* switch keys to another (ie., DOS) mode */
kb_switch (cstp, newkp)
    struct colr_state *cstp;
    struct doskeys *newkp;
{
    unsigned char index, table;

    while (index = newkp->index) {
	switch (table = newkp->table) {
	    case K_NORMTAB:
	    case K_SHIFTTAB:
	    case K_ALTTAB:
	    case K_ALTSHIFTTAB:
		cstp->kb_data [index].data [table] = newkp->value;
		break;

	    case K_CTRLTAB:
		cstp->kb_control [index] = newkp->value;
		break;
	}
	newkp++;
    }
}
#endif

kb_keyent (ke, cstp, set)
    struct kbentry *ke;
    struct colr_state *cstp;
    int set;
{
    if ((ke->kb_index >= 128) || (ke->kb_table > K_CTRLTAB))
	return 1;

    if (ke->kb_table & K_CTRLTAB) {
	if (set)
	    cstp->kb_control [ke->kb_index] = ke->kb_value;
	else
	    ke->kb_value = cstp->kb_control [ke->kb_index];
    }
    else {
	if (set)
	    cstp->kb_data [ke->kb_index].data [ke->kb_table] = ke->kb_value;
	else
	    ke->kb_value = cstp->kb_data [ke->kb_index].data [ke->kb_table];
    }
    return 0;
}

/* kernel get character support */
#define	GETBUFSIZE	64
unsigned char kd_kgetcnt = 0;
unsigned char *kd_kgetstr;
unsigned char kd_kgetbuf [GETBUFSIZE];

kd_getc (wait)
    int wait;
{
    unsigned char inchar;	/* raw key scan code */
    unsigned char waiting;
    struct colr_state *cstp;

    /* we need a alpha console */
    while (! (cstp = kdc.cstp [kda.actpage]) && !MAPPED)
	kdapset (-1);

    do {
	if (kd_kgetcnt) {
	    kd_kgetcnt--;
	    return *kd_kgetstr++;
	}
	waiting = wait;
	inchar = kd_inb (KEYBD_STAT);
	if (inchar & KB_OUTBF) {
	    inchar = kd_inb (KEYBD_OUT);/* read data from keybd out port */
	    waiting = 1;		/* read at least one more after this */

	    /* check for keyboard controller control character */
	    switch (inchar) {
		case KB_RESEND:		 	   /* resend command */
		    kda.kb_status |= KB_RES_FLG;   /* update status byte */
		    continue;
		case KB_ACK:			   /* acknowledge command */
		    kda.kb_status |= KB_ACK_FLG;   /* update status byte */
		    continue;
		case 0xE0:			   /* extended char */
		case 0xE1:
		    continue;			   /* ignore for now */
		case 0xFF:			   /* input overrun */
		    continue;
	    }
	    kd_kgetstr = kb_proc (inchar, cstp);
						/* process input character */
	    kd_kgetcnt = *kd_kgetstr++;		/* set character count */
	    if (kd_kgetcnt > GETBUFSIZE)	/* trim size */
		kd_kgetcnt = GETBUFSIZE;
	    bcopy (kd_kgetstr, kd_kgetbuf, kd_kgetcnt);	/* move to buffer */
	    continue;				/* and loop once more */
	}
    } while (waiting);
    return 0xFF;		/* no character received */
}

/* === */
