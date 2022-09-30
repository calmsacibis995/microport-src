#ifndef lint
static char *uportid = "@(#)kd_ansi.c	3.1 Microport Rev 2.3 10/5/87";
#endif

/* Modification History:
 *
 * M000	uport!mike Fri Apr 24 13:01:41 PST 1987
 *	Created.
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

#ifdef ATMERGE
#include "sys/realmode.h"
extern struct sw_data usw_data;
#endif /* ATMERGE */

extern struct kda_state kda;
extern struct kdc_state kdc;

/* table of color values for foreground and background color commands
 * set by ESC[nm
 * 
 * black, red, green, yellow or brown, blue, magenta, cyan, white
 */
char kd_colrtab [8] = {0, 4, 2, 6, 1, 5, 3, 7};

/* Define offset for mode settings that start with '?' */
#define	EXTMODE1_BASE	312
#define	EXTMODE1	EXTMODE1_BASE*10
#define	EXTMODE2	EXTMODE1_BASE*100

/*
 * Process character following the ESCAPE
 */
kddoesc (chr, cstp)
    unsigned char chr;
    register struct colr_state * cstp;
{
    short row, col;

    if (chr == '[') {
	ESCFLG      = 2;
	PARMPRES    = 0;
	CNTESC      = 0;
	ESCDATA [0] = 0;
    }
    else {
	ESCFLG = 0;	/* this will be the last char of the escape sequence */
	if (!FEAM_MODE) {
	    row = CROW;
	    col = CCOL;
	    switch (chr) {
		case '6':
		    MODEFLAGS ^= IRM;
		    break;

		case '7':		/* save cursor */
		    cstp->save = cstp->cur;
		    MODEFLAGS &= ~BSUND;
		    break;

		case '8':		/* restore cursor */
		    cstp->cur = cstp->save;
		    row = CROW;
		    col = CCOL;
		    break;

		case 'E':		/* NEL - next line */
		    col = 0;

		case 'D':		/* IND - index */
		    if ((++row > MAXROW) && (kd_scroll (cstp, row, 1)))
			--row;
		    break;

		case 'H':		/* HTS - horiz. tab stop */
		    break;

		case 'M':		/* RI - reverse index */
		    if ((--row < MINROW) && (kd_scroll (cstp, row, 0)))
			++row;
		    break;

		case 'Z':		/* DECID - identify terminal */
		    break;

		case 'c':		/* RIS - reset initialization sequence*/
		    kdmode (cstp, cstp->biosmode, 0, 1);
		    kb_init (cstp);	/* reset keyboard */
		    break;

		default:
		    break;
	    }
	    kd_setcur (cstp, row, col);
	}
    }
}
/*
 * Unpack a decimal number
 */
kdansunp (val, str)
    int val;
    char *str;
{
    int ret = 0;
    int digit = 10000;

    do {
	if (val >= digit || digit == 1) {
	    *str++ = val / digit + '0';
	    ret++;
	    val %= digit;
	}
    } while (digit /= 10);
    return ret;
}

/*
 *  Process characters following ESCAPE LEFT BRACKET
 */
kddoansi (chr, cstp)
    unsigned char chr;
    register struct colr_state * cstp;
{
    short row, col;
    short lastparm, lastparm1;
    short tmp, tmp2;
    char tmpbuf [10];

    /*
     * First look at prefix characters
     */
    switch (chr) {
	/* numeric prefixes: '0'..'9' */
	case '0': case '1': case '2': case '3': case '4': 
	case '5': case '6': case '7': case '8': case '9': 
	    ESCDATA [CNTESC] = ESCDATA [CNTESC] * 10 + (chr - '0');
	    PARMPRES = 1;
	    return;

	/* prefix char '?' *DEC STANDARD* */
	case '?':
	    if (PARMPRES || ++CNTESC >= MAXESCDATA)
		ESCFLG = 0;		/* overrun */
	    else {
		ESCDATA [CNTESC] = EXTMODE1_BASE;
		PARMPRES = 1;
	    }
	    return;

	/* prefix char '=': hardware mode change */
	case '=':
	    ESCFLG = (PARMPRES || CNTESC > 0) ? 0 : 4;
	    return;

	/* parameter seperator: ';' */
	case ';':
	    if (++CNTESC >= MAXESCDATA)
		ESCFLG = 0;		/* overrun */
	    else {
		ESCDATA [CNTESC] = 0;
		PARMPRES = 0;
	    }
	    return;

	/* While not a prefix character ('l'), we must process this
	 * before we check FEAM or we'll never be able to reset it!
	 */
	case 'l':		/* RM - Reset Mode		*/
	    if (PARMPRES)
		CNTESC++;
	    for (tmp = 0; tmp < CNTESC; tmp++)
		if (ESCDATA [tmp] == 13)	/* allow reset FEAM */
		    MODEFLAGS &= ~FEAM;
		else if (!FEAM_MODE)
		    switch (ESCDATA [tmp]) {
			case  3:	MODEFLAGS &= ~CRM;	break;
			case  4:	MODEFLAGS &= ~IRM;	break;
			case  7:	MODEFLAGS &= ~VEM;	break;
			case 10:	MODEFLAGS &= ~HEM;	break;
			case 20:	MODEFLAGS &= ~LNM;	break;
			case EXTMODE1+6:  MODEFLAGS &= ~OM;	break;
			case EXTMODE1+7:  MODEFLAGS &= ~WM;	break;
			case EXTMODE2+33: MODEFLAGS &= ~CRTERM; break;
		    }
	    ESCFLG = 0;
	    return;
    }
    if (FEAM_MODE) {		/* we're ignoring escape sequences */
	ESCFLG = 0;
	return;
    }
    if (lastparm = ESCDATA [CNTESC]) /* if last parameter wasn't a zero */
	lastparm1 = lastparm;	     /* it's ok */
    else			     /* otherwise set param 1 default to .. */
	lastparm1 = 1;		     /* one for those cmds that want it 1 */

    row = CROW;			/* get current row */
    col = CCOL;			/* get current column */

    if (ESCFLG == 2) 		/* ESC [ px X */
	switch (chr) {
	    case '@':			/* ICH - insert character */
		if (! IRM_MODE) {
		    tmp = NUMCOLS - col;	/* max. possible move */
		    if (FAKEPOS)
			kd_fakecursor (cstp, 0);	/* turn off fake cur */
		    if (lastparm1 < tmp) {
			tmp -= lastparm1;	/* amt we have to move*/
			MOVEF (SCREEN, CURPOS, lastparm1, tmp);
		    }
		    else
			lastparm1 = tmp;	/* trim to MAX ()   */
		    FILLW (FILL, SCREEN, CURPOS, lastparm1);
		}
		break;

	    case 'F':		/* CPL - Cursor Preceeding Line	*/
		col = MINCOL;
		/* FALL THROUGH */

	    case 'A':		/* CUU - Cursor Up		*/
		tmp = row - lastparm1;
		if ((row -= lastparm1) < MINROW)
		    row = MINROW;
		break;

	    case 'E':		/* CNL - Cursor Next Line	*/
		col = MINCOL;
		/* FALL THROUGH */

	    case 'B':		/* CUD - Cursor Down		*/
		if ((row += lastparm1) > MAXROW)
		    row = MAXROW;
		break;

	    case 'C':		/* CUF - Cursor Forward		*/
		if ((col += lastparm1) > MAXCOL)
		    col = MAXCOL;
		break;

	    case 'D':		/* CUB - Cursor Backward	*/
		if ((col -= lastparm1) < MINCOL)
		    col = MINCOL;
		break;

	    case 0x60:		/* HPA - Horizontal Pos. Abs	*/
	    case 'G':		/* CHA - Cursor Horiz. Abs	*/
		if ((col = lastparm1 - 1) > MAXSCRCOL)
		    col = MAXSCRCOL;
		break;

	    case 'H':		/* CUP - Cursor Position	*/
		/* get the last two parms, if only 1 then it's the row */
		if (CNTESC > 0) {
		    if ((row = ESCDATA [CNTESC - 1] - 1) < 0)
			 row = 0;
		    col = lastparm1 - 1;
		}
		else {
		    row = lastparm1 - 1;
		    col = 0;
		}
#ifdef	ORIGIN_LIMIT		/* limit cursor to end of logical screen */
		if (ORIGIN_MODE) {
		    row += MINROW;
		    col += MINCOL;
		    if (row > MAXROW) row = MAXROW;
		    if (col > MAXCOL) col = MAXCOL;
		}
		else {
		    if (row > MAXSCRROW) row = MAXSCRROW;
		    if (col > MAXSCRCOL) col = MAXSCRCOL;
		}
#else				/* limit cursor to end of physical screen */
		if (ORIGIN_MODE) {
		    row += MINROW;
		    col += MINCOL;
		}
		if (row > MAXSCRROW) row = MAXSCRROW;
		if (col > MAXSCRCOL) col = MAXSCRCOL;
#endif
		break;

    /*	    case 'I':			/* CHT - Cursor Horiz. Tab	*/

	    case 'J':   	/* ED - Erase in Display		  */
	    {	    		/*  0 - erase to end of display (default) */
				/*  1 - erase from start to present	  */
				/*  2 - erase all of display		  */

		if (lastparm != 1 && lastparm != 2)
		    tmp = CURPOS;
		else if (!ORIGIN_MODE || MINROW==MINSCRROW || row < MINROW)
		    tmp = MINSCRROW * NUMCOLS;
		else
		    tmp = MINROW * NUMCOLS;

		if (lastparm == 1)
		    tmp2 = CURPOS + 1;
		else if (!ORIGIN_MODE || row > MAXROW)
		    tmp2 = (MAXSCRROW + 1) * NUMCOLS;
		else
		    tmp2 = NUMROWS * NUMCOLS;

		if (FAKEPOS)
		    kd_fakecursor (cstp, 0);	/* turn off fake cur */
		FILLW (FILL, SCREEN, tmp, tmp2 - tmp);
		break;
	    }

	    case 'K':	/* EL - Erase Line			*/
			    /*  0 - erase to end of line (default)	*/
			    /*  1 - erase from start to present	*/
			    /*  2 - erase all of line		*/
		if (FAKEPOS)
		    kd_fakecursor (cstp, 0);	/* turn off fake cur */
		switch (lastparm) {
		    default:
		    case 0:	tmp = CURPOS; tmp2 = NUMCOLS - CCOL;  break;
		    case 1:	tmp = CROW * NUMCOLS; tmp2 = CCOL;    break;
		    case 2:	tmp = CROW * NUMCOLS; tmp2 = NUMCOLS; break;
		}
		FILLW (FILL, SCREEN, tmp, tmp2);
		break;

	    case 'L':		/* IL - Insert Line		*/
		tmp = (NUMROWS - row) * NUMCOLS;
		lastparm1 *= NUMCOLS;
		if (FAKEPOS)
		    kd_fakecursor (cstp, 0);	/* turn off fake cur */
		if (lastparm1 < tmp) {
		    tmp -= lastparm1;
		    MOVEF (SCREEN, row * NUMCOLS, lastparm1, tmp);
		}
		else
		    lastparm1 = tmp;
		FILLW (FILL, SCREEN, row * NUMCOLS, lastparm1);
		break;

	    case 'M':		/* DL - Delete Line		*/
		tmp = (NUMROWS - row) * NUMCOLS;
		lastparm1 *= NUMCOLS;
		tmp2 = row * NUMCOLS;
		if (FAKEPOS)
		    kd_fakecursor (cstp, 0);	/* turn off fake cur */
		if (lastparm1 < tmp) {
		    tmp -= lastparm1;
		    MOVEB (SCREEN, tmp2, lastparm1, tmp);
		    tmp2 += tmp;
		}
		else
		    lastparm1 = tmp;
		FILLW (FILL, SCREEN, tmp2, lastparm1);
		break;

    /*	    case 'N':			/* EF - Erase Field		*/
    /*	    case 'O':			/* EA - Erase Area		*/

	    case 'P':		/* DCH - Delete Character	*/
		tmp = NUMCOLS - col;
		if (FAKEPOS)
		    kd_fakecursor (cstp, 0);	/* turn off fake cur */
		if (lastparm1 < tmp) {
		    tmp -= lastparm1;
		    MOVEB (SCREEN, CURPOS, lastparm1, tmp);
		}
		else
		    lastparm1 = tmp;
		FILLW (FILL, SCREEN, CURPOS + tmp, lastparm1);
		break;

    /*	    case 'Q':			/* SEM - Select Editing Extent Mode */
    /*	    case 'R':			/* CPR - Cursor Position Report	*/

	    case 'S':		/* SU - Scroll Up		*/
		tmp = (NUMROWS - MINROW) * NUMCOLS;
		lastparm1 *= NUMCOLS;
		if (FAKEPOS)
		    kd_fakecursor (cstp, 0);	/* turn off fake cur */
		if (lastparm1 < tmp) {
		    tmp -= lastparm1;
		    MOVEB (SCREEN, MINROW * NUMCOLS, lastparm1, tmp);
		}
		else {
		    lastparm1 = tmp;
		    tmp = MINROW * NUMCOLS;
		}
		FILLW (FILL, SCREEN, tmp, lastparm1);
		break;

	    case 'T':		/* SD - Scroll Down		*/
		tmp = (NUMROWS - MINROW) * NUMCOLS;
		lastparm1 *= NUMCOLS;
		if (FAKEPOS)
		    kd_fakecursor (cstp, 0);	/* turn off fake cur */
		if (lastparm1 < tmp) {
		    tmp -= lastparm1;
		    MOVEF (SCREEN, MINROW * NUMCOLS, lastparm1, tmp);
		}
		else
		    lastparm1 = tmp;
		FILLW (FILL, SCREEN, MINROW * NUMCOLS, lastparm1);
		break;
	    
    /*	    case 'U':			/* NP - Next Page		*/
    /*	    case 'V':			/* PP - Preceeding Page		*/
    /*	    case 'W':			/* CTC - Cursor Tabulation Conrol */

	    case 'X':		/* ECH - Erase Characters	*/
		tmp = NUMCOLS - col;	/* num chars left in line */
		if (lastparm1 > tmp)
		    lastparm1 = tmp;
		FILLW (FILL, SCREEN, CURPOS, lastparm1);
		break;

	    case 'Y':		/* CVT - Cursor Vertical Tab	*/
		if (MINSCRROW < MINROW && row < MINROW)
		    { row = MINROW; break; }
		if (MAXROW < MAXSCRROW) {
		    if (row <= MAXROW)
			row = MAXROW + 1;
		    else
			row = MINSCRROW;
		    break;
		}
		if (MINSCRROW < MINROW)
		    row = MINSCRROW;
		else
		    row = MAXSCRROW;
		break;

	    case 'Z':		/* CBT - Cursor Back Tab	*/
		if ((col & TABLN-1) == 0)	/* if at tab stop */
		    col--;
		col &= -TABLN;		/* do first one	*/
		if (tmp = lastparm1 - 1) {
		    col -= tmp * TABLN;	/* do extras if needed */
		    if (col < MINCOL)	/* adjust if gone too far */
			col = MINCOL;
		}
		break;

	    case 'a':		/* HPR - Horiz. Pos. Relative	*/
		col = lastparm + MINCOL;
		if (col > MAXCOL)
		    col = MAXCOL;
		break;

	    case 'b':		/* REP - repeat last char	*/
		tmp = 0;
		while (--lastparm >= 0) {
		    if (tmp >= 10) {
			kd_puts (tmpbuf, 10, cstp, LASTCHAR == '\033');
			tmp = 0;
		    }
		    tmpbuf [tmp++] = LASTCHAR;
		}
		if (tmp > 0)
		    kd_puts (tmpbuf, tmp, cstp, 0);
		break;

    /*	    case 'c':			/* DA - Device Attributes	*/

	    case 'd':		/* VPA - Vertical Pos Abs	*/
		if ((row = lastparm1) > MAXSCRROW)
		    row = MAXSCRROW;
		break;

	    case 'h':		/* SM - Set screen Mode		*/
		if (PARMPRES)
		    CNTESC++;
		for (tmp = 0; tmp < CNTESC; tmp++)
		    switch (ESCDATA [tmp]) {
			case  3:	MODEFLAGS |= CRM;	break;
			case  4:	MODEFLAGS |= IRM;	break;
			case  7:	MODEFLAGS |= VEM;	break;
			case 10:	MODEFLAGS |= HEM;	break;
			case 13:	MODEFLAGS |= FEAM;	break;
			case 20:	MODEFLAGS |= LNM;	break;
			case EXTMODE1+6:  MODEFLAGS |= OM;	break;
			case EXTMODE1+7:  MODEFLAGS |= WM;	break;
			case EXTMODE2+33: MODEFLAGS |= CRTERM;	break;
		    }
		break;

	    case 'm':		/* SGR - Set Graphic Rendition	*/
		kdattr (cstp);
		break;

	    case 'n':		/* DSR - Device Status Report	*/
		{
		    if (PARMPRES)
			CNTESC++;
		    for (tmp = 0; tmp < CNTESC; tmp++) {
			switch (ESCDATA [tmp]) {
			    case 5:		/* status report	*/
						/* BUSY if NOT MAPPED	*/
				tmpbuf [0] = '\033';
				tmpbuf [1] = '[';
				tmpbuf [2] = MAPPED ? '0' : '1';
				tmpbuf [3] = 'n';
				tmp2 = 4;
				break;

			    case 6:		/* report cursor	*/
				tmpbuf [0] = '\033';
				tmpbuf [1] = '[';
				tmp2  = kdansunp (row + 1, &tmpbuf [2]) + 2;
				tmpbuf [tmp2++] = ';';
				tmp2 += kdansunp (col + 1, &tmpbuf [tmp2]);
				tmpbuf [tmp2++] = 'R';
				break;
			    
			    case 20:		/* report scrolling region */
				tmpbuf [0] = '\033';
				tmpbuf [1] = '[';
				tmp2  = kdansunp (MINROW + 1, &tmpbuf [2]) + 2;
				tmpbuf [tmp2++] = ';';
				tmp2 += kdansunp (MAXROW + 1, &tmpbuf [tmp2]);
				tmpbuf [tmp2++] = 'r';
				break;

			    case 21:		/* report max screen size */
				tmpbuf [0] = '\033';
				tmpbuf [1] = '[';
				tmp2  = kdansunp (MINSCRROW + 1,&tmpbuf [2]) +2;
				tmpbuf [tmp2++] = ';';
				tmp2 += kdansunp (MAXSCRROW + 1,&tmpbuf [tmp2]);
				tmpbuf [tmp2++] = 'r';
				break;

			    default:
				tmp2 = 0;
				break;
			}
			if (tmp2) {
			    if (CR_TERM)
				tmpbuf [tmp2++] = '\r';
			    keyinstr (cstp, tmpbuf, tmp2);
			}
		    }
		}
		break;

    /*	        case 'o':		/* DAQ - Define Area Qualification */

		case 'r':		/* DECSTBM - set scrolling regiin */
		    /* get the last two parms,
		     * if only 1 then it's the minrow
		     */
		    if (CNTESC > 0) {
			if ((row = ESCDATA [CNTESC - 1] - 1) < MINSCRROW)
			     row = MINSCRROW;
			else if (row > MAXSCRROW)
			     row = MAXSCRROW;
			if ((tmp = lastparm1 - 1) > MAXSCRROW)
			     tmp = MAXSCRROW;
		    }
		    else {
			if ((row = lastparm1 - 1) > MAXSCRROW)
			     row = MAXSCRROW;
			tmp = MAXSCRROW;
		    }
#ifdef	REMOVE		/* don't know if we want this or not */
		    if (row > MINSCRROW || tmp < MAXSCRROW)
			MODEFLAGS |= OM;		/* set origin mode */
		    else
			MODEFLAGS &= ~OM;		/* set origin mode */
#endif
		    col = MINCOL;
		    kd_setscroll (cstp, row, tmp);
		    break;

		case 'q':		/* Program Function Key */
		    if (CNTESC > 0) {
			tmp = ESCDATA [CNTESC - 1];
			ESCDATA [0] = lastparm;
		    }
		    else {
			tmp = lastparm % 10;
			ESCDATA [0] = 0;
		    }
		    cstp->sk.k_shift = tmp / 10;
		    cstp->sk.k_code  = F1_SCANCODE + tmp % 10;
		    cstp->sk.k_len   = 0;
		    ESCFLG = 6;
		    return;
	    }

	else if (ESCFLG == 4)			/* ESC [ = p X */
	    switch (chr) {
		case 'h':
		    kdmode (cstp, -1);
		    break;
	    }

    kd_setcur (cstp, row, col);		/* update new cursor position */
    ESCFLG = 0;
}

/*
* ANSI SGR - Set attribute bits or change color sets for all modes
* non-ansi: ESC[3;1/0 for switch blink on & off,
* 	   : ESC[
*      ESC[1[012] for font access
* Need a whole new one for EGA.
*/
kdattr (cstp) 
    register struct colr_state * cstp;
{
    short parm, indx;
    char set_bgintens = 0, set_fgintens = 0;
    short cursor;
    short tmp1, tmp2;

    if (FEAM_MODE)		/* if "transparent" mode, do nothing */
	return;

    if (PARMPRES)
	CNTESC++;

    if (CNTESC == 0) {
	ATRBTE = FILL.at;
	return;
    }

    for (indx = 0; indx < CNTESC; ) {
	parm = ESCDATA [indx++];
	if (parm <= 19) {
	    switch (parm) {
		case 0:				/* normal attributes	*/
		    ATRBTE = FILL.at;
		    set_bgintens = 0;
		    set_fgintens = 0;
		    break;

		case 1:				/* brighten foreground	*/
		case 2:				/* lighten foreground	*/
		    if (FILL.at & INTEN_ATTRIB_ON)
			ATRBTE &= ~INTEN_ATTRIB_ON;
		    else
			ATRBTE |= INTEN_ATTRIB_ON;
		    set_fgintens = INTEN_ATTRIB_ON;
		    break;

		/* NON-ANSI */
		case 3:	/* mode reg blink bit:	3;0 -> off,
			 *			3;1 -> on,
			 *			3;2 -> ~bgintens
			 */
		    if (indx < CNTESC) {
			parm = ESCDATA [indx++];
			if (kda.board & CGABOARD) {
			    if (parm == 1)
				MODEREG |= MODEREG_BLINK;
			    else if (parm == 2) {
				if (FILL.at & BLINK_ATTRIB_ON)
				    MODEREG &= ~MODEREG_BLINK;
				else
				    MODEREG |= MODEREG_BLINK;
			    }
			    else
				MODEREG &= ~MODEREG_BLINK;
			}
		    }
		    break;

		case 4: 			/* Start underline	*/
		    ATRBTE = kd_underline (ATRBTE);
		    break;

		case 5:				/* blink or bright	*/
		case 6:
		    if (cstp->biosmode == 7)
			ATRBTE |= BLINK_ATTRIB_ON;
		    else
			ATRBTE ^= BLINK_ATTRIB_ON;
		    set_bgintens = BLINK_ATTRIB_ON;
		    break;

		case 7: 			/* inverse video	*/
		    if (kda.biosmode == 7)
			ATRBTE = (ATRBTE & 0x88) | RVID_ATTRIB_ON;
		    else
			ATRBTE =  (ATRBTE & 0x88)
			       | ((ATRBTE & 0x70) >> 4)
			       | ((ATRBTE & 0x07) << 4);
		    break;

		/* NON-ANSI */
		case 8:				/* Invisible */
		    ATRBTE = (ATRBTE & 0x88) | ALL_ATTRIB_OFF;
		    break;

		/* NOT EXACTLY ANSI! */
		case 10: FONTMODE  = 0;	break;	/* normal font */
		case 11: FONTMODE |= 1;	break;	/* print chars <32 */
		case 12: FONTMODE |= 2;	break;	/* turn high bit on */

	    } /* end switch */
	} /* end if */

	else if (parm <= 29) {		/* special functions	*/
	    cursor = CURSORSIZE;
	    switch (parm) {
		case 21:		/* load color reg	*/
		    if (kda.ega) {
			if (indx < CNTESC)
			    tmp1 = ESCDATA [indx++];
			if (indx < CNTESC) {
			    tmp2 = ESCDATA [indx++];
			    if (MAPPED)
				kd_putmode (CRT_STAT, tmp1, tmp2 ,1);
			}
		    }
		    else {
			if (indx < CNTESC)
			    COLRREG = ESCDATA [indx++];
		    }
		    break;
		case 22:		/* set 8 bit attribute	*/
		    if (indx < CNTESC)
			FILL.at = ATRBTE = ESCDATA [indx++];
		    break;
		case 23:		/* cursor underline */
		    cursor = cstp->ctrlregs [0x0A] << 8
			   | cstp->ctrlregs [0x0B];
		    break;

		case 24:		/* cursor block */
		    cursor = cstp->ctrlregs [0x0B];
		    break;

		case 25:		/* cursor off */
		    cursor = -1;
		    break;

		case 26:		/* fake block cursor on */
		    MODEFLAGS |= FAKECUR;
		    break;

		case 27:		/* fake block cursor off */
		    MODEFLAGS &= ~FAKECUR;
		    break;

		case 28:		/* change cursor style */
		    if (indx < CNTESC)
			cursor = ESCDATA [indx++];
		    if (indx < CNTESC)
			cursor = (cursor << 8) | (ESCDATA [indx++] & 0xFF);
		    break;

		case 29:		/* EMPTY */
		    break;
	    } /* end switch (parm <= 29) */

	    CURSORSIZE = cursor;
	    kd_setcur (cstp, CROW, CCOL);
	}
 	else if (parm <= 39)		/* set foreground color */
	    FILL.at = ATRBTE = (ATRBTE & 0xf0)
			     | kd_colrtab [parm - 30] | set_fgintens;

	else if (parm <= 49)		/* set background color */
	    FILL.at = ATRBTE = (ATRBTE & 0x0f)
			     | (kd_colrtab [parm-40]<<4)|set_bgintens;

	else switch (parm) {
	    case 50:			/* save current screen */
		if (MAPPED)
		    *cstp->vs = *cstp->screen;
		break;

	    case 51:			/* restore saved screen */
		if (MAPPED)
		    *cstp->screen = *cstp->vs;
		break;

	    case 52:			/* block attribute change */
		if (indx < CNTESC) tmp1 = ESCDATA [indx++]; else tmp1 = NUMROWS;
		if (indx < CNTESC) tmp2 = ESCDATA [indx++]; else tmp2 = NUMCOLS;
		if (MAPPED)
		    kd_blockfill (cstp, tmp1, tmp2);
		break;

	    case 53:			/* turn video off */
		if (MAPPED)
		    kd_egaoff (CRT_STAT);
		break;

	    case 54:			/* turn video on  */
		if (MAPPED)
		    kd_putmode (CRT_STAT, 0xff, 0, 1);
		break;
	    
	    case 55:		/* set default attribute same as current */
		FILL.at = ATRBTE;
		break;

	    case 56:		/* get current attr from current char pos */
		ATRBTE = SCREEN [POSIT (CROW, CCOL)].chc.at;
		break;
	}
    } /* end for */

    /* Figure this out! */
    if ((kda.board & (CGABOARD | 0xC)) != CGABOARD) 	/* if color graphic */
	COLRREG |= (ATRBTE & INTEN_ATTRIB_ON);

    if (MAPPED)
	kd_outmode (MODEREG, COLRREG, 0);
}

/*
 * Setup the color mode register
 * In addition, turns on the video after a mode change.
 */
kd_outmode (mode, color, force)
    unsigned char mode, color, force;
{
    if (!kda.ega) {
	DBG6 (printf ("kd_outmode: CGA: mode:%x color:%x\n", mode, color));
	kda.modereg = mode;
	kda.colrreg = color;
	WRITEMODE (mode);
	WRITECOLOR (color);
    }
    else if (force || kda.video_off || mode != kda.modereg) {
	int attr = kda.modeattr & ~0x08;	/* attribute */
	if (mode & MODEREG_BLINK)		/* blink/bright background */
	    attr |= 0x08;

	kda.modereg = mode;
	kda.colrreg = color;

	DBG6 (printf ("kd_outmode: EGA: mode:%x attr:%x\n", mode, attr));
	kd_putmode (CRT_STAT, 0x10, attr, 1);
	kda.video_off = 0;		/* video is now on */

#ifdef	REMOVE
	(void) READSTATUS;
	kd_vwait (CRT_STAT);	/* wait for vertical retrace */
	kd_outb2 (CRT_ATRB, 0x10, attr);

	/* do misc right */
	kd_outb (CRT_MISC, kd_egamodes [kda.egamode]->misc);
	(void) READSTATUS;	/* reset attr address flip-flop */
	kd_outb (CRT_ATRB, 0x20);	/* allow pallette access	*/
	asm (" sti");		/* turn on int's, off in kd_vwait */
#endif
#ifdef	REMOVE
	if (kd_alphamodes [kda.biosmode]) {
	    attr = (kd_egamodes [kda.modeindex])->c11;
	    kd_outb21 (CRT_CTRL, 0x11, attr & ~0x10 | 0x20);
	    kd_outb21 (CRT_CTRL, 0x11, attr & ~0x20 | 0x10);
	}
#endif
    }
}

/*
 * Fill an area of the screen with the current attribute
 */
kd_blockfill (cstp, rowcnt, colcnt)
    struct colr_state *cstp;
    int rowcnt, colcnt;
{
    short curpos = CURPOS;
    struct chcell fill;

    fill.at = ATRBTE;
    fill.ch = ' ';
    if (rowcnt > (NUMROWS - CROW)) rowcnt = (NUMROWS - CROW);
    if (colcnt > (NUMCOLS - CCOL)) colcnt = (NUMCOLS - CCOL);

    while (--rowcnt > 0) {
	FILLW (fill, SCREEN, curpos, colcnt);
	curpos += NUMCOLS;
    }
}

kd_underline (atrb)
    int atrb;
{
    if (kda.board & MONOBOARD)
	atrb = (atrb & 0x88) | UNDL_ATTRIB_ON;
    else {		/* invert a color bit that doesn't make fg == bg */
	unsigned char tmpf =  atrb       & 0x0F;
	unsigned char tmpb = (atrb >> 4) & 0xF0;
	     if ((tmpf ^ 0x08) != tmpb) atrb ^= 0x08;
	else if ((tmpf ^ 0x04) != tmpb) atrb ^= 0x04;
	else if ((tmpf ^ 0x02) != tmpb) atrb ^= 0x02;
	else if ((tmpf ^ 0x01) != tmpb) atrb ^= 0x01;
    }
    return atrb;
}

/* === */
