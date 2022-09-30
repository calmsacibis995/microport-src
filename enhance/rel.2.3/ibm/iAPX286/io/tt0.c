static char *uportid = "@(#)tt0.c	Microport Rev Id 1.3.5  6/26/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)tt0.c	1.6 */
/*
 * Line discipline 0
 * Includes Terminal Handling
 * M000 Suspect interrupt structure.  Modified to use spl7(). lcn 1-20-86
 * M001 Change close to clear WOPEN flag; can get there from signal death
 *		while waiting for carrier to raise.
 * M002 uport!dwight Fri Jun 27 10:42:22 PDT 1986
 *	Changed curlvl to curlevel.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/crtctl.h"
#include "sys/sysinfo.h"
#include "sys/ioctl.h"

#ifdef DEBUG 
#undef DEBUG
#endif

#ifdef DEBUGTP
#undef DEBUGTP
#endif

#ifdef DEBUGSP
#undef DEBUGSP
#endif

#define	TTYSPL	spl7		/* timeout-driver sio is level 6 M000 */

/*#define DEBUG 1 */
/*#define DEBUGTP 1 */
/*#define DEBUGSP 1 */
int lastcall = 0;		/* CJH tracing variable */
extern char partab[];
extern struct cblock *cfree;	/* M000 */

char colsave, rowsave;		/* temp save for high queue restore */
struct clist tempq;		/* temp for echo during high queue  */

/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
ttopen(tp)
register struct tty *tp;
{
	register struct proc *pp;
#ifdef DEBUG
printf("Reached ttopen in tt0.c\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

	pp = u.u_procp;
	if ((pp->p_pid == pp->p_pgrp)
	 && (u.u_ttyp == NULL)
	 && (tp->t_pgrp == 0)) {
		u.u_ttyp = &tp->t_pgrp;
		tp->t_pgrp = pp->p_pgrp;
	}
	/* (caddr_t) substituting for (union ioctl_arg) */
	ttioctl(tp, LDOPEN, (caddr_t)0, 0);
#ifdef DEBUG
printf("returned from ttioctl call in ttopen\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
	tp->t_iscon = 0;
}

ttclose(tp)
register struct tty *tp;
{
#ifdef DEBUG
printf("reached ttclose call in tt0.c \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	if ((tp->t_state&ISOPEN) == 0)
		return;
	tp->t_state &= ~(ISOPEN|WOPEN);
	tp->t_pgrp = 0;
	/* (caddr_t) substituting for (union ioctl_arg) */
	ttioctl(tp, LDCLOSE, (caddr_t)0, 0);
#ifdef DEBUG
printf("returned from ttioctl call in ttclose\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;
{
	register struct clist *tq;
#ifdef DEBUG
printf("reached ttread in tt0 \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

	tq = &tp->t_canq;

	if (tq->c_cc == 0)
		canon(tp);
	while (u.u_count!=0 && u.u_error==0) {
		if (u.u_count >= CLSIZE) {
			register n;
			register struct cblock *cp;

			if ((cp = getcb(tq)) == NULL)
				break;
			n = min(u.u_count, cp->c_last - cp->c_first);
			if (copyout(&cp->c_data[cp->c_first], u.u_base, n))
				u.u_error = EFAULT;
			putcf(cp);
			u.u_base += n;
			u.u_count -= n;
		} else {
			register c;

			lastcall = 1;		/* CJH */
			if ((c = getc(tq)) < 0)
				break;
			if (subyte(u.u_base++, c))
				u.u_error = EFAULT;
			u.u_count--;
		}
	}
	if (tp->t_state&TBLOCK) {
		if (tp->t_rawq.c_cc<TTXOLO) {
#ifdef DEBUG
printf("gonna call T_UNBLOCK from ttread in tt0 \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
			(*tp->t_proc)(tp, T_UNBLOCK);
		}
	}
}

/*
 * Called from device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(tp)
register struct tty *tp;
{
	int hqflag;
	int col, row;
	extern curlevel;				/* M002 */

	int x;
#ifdef DEBUGSP
printf("got to ttwrite in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

qwait:
	x = tp->t_iscon ? spl6() : spl7();
	while(tp->t_tmflag & QLOCKB) {
		tp->t_state |= OASLP;
		sleep((caddr_t)&tp->t_outq, TTOPRI);
	}
	splx(x); 
	if (!(tp->t_state&CARR_ON)) {
		return;
		}
	hqflag = 0;
#ifdef DEBUG
printf("before first while loop\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	while (u.u_count) {

		if (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD]) {
			if (hqflag) {
				col = tp->t_col;
				row = tp->t_row;
				hqrelse(tp);
			}
			x = tp->t_iscon ? curlevel : TTYSPL();	/* M002 */
			(*tp->t_proc)(tp, T_OUTPUT);
			while (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD]) {
				tp->t_state |= OASLP;
				if (sleep((caddr_t)&tp->t_outq,
					hqflag ? PZERO : (TTOPRI|PCATCH))) {
					spl0();
					goto out;
				}
			}
			splx(x);
			if (hqflag) {
				tp->t_tmflag |= QLOCKI;
				colsave = tp->t_col;
				rowsave = tp->t_row;
				ttyctl(LCA, tp, col, row);
				continue;
			}
			if (tp->t_tmflag & QLOCKB)
				goto qwait;
		}
		if (u.u_count >= (CLSIZE/2) && tp->t_term == 0) {
			register n;
			register struct cblock *cp;

			if ((cp = getcf()) == NULL)
				break;
			n = min(u.u_count, cp->c_last);
			if (copyin(u.u_base, cp->c_data, n)) {
				u.u_error = EFAULT;
				putcf(cp);
				break;
			}
			u.u_base += n;
			u.u_count -= n;
			cp->c_last = n;
#ifdef DEBUG
printf("gonna call ttxput from ttwrite in tt0 \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

			ttxput(tp, n, cp);
		} else {
			register c;

			c = fubyte(u.u_base++);
			if (c<0) {
				u.u_error = EFAULT;
				break;
			}
			u.u_count--;
			if (c == ESC && tp->t_term) {
				switch (c = cpass()) {
					int col;

				case -1:
					continue;
				case ESC:
					goto norm;
				case HIQ:
					if (hqflag++)
						continue;
					tp->t_tmflag |= QLOCKB|QLOCKI;
					tp->t_hqcnt++;
					colsave = tp->t_col;
					rowsave = tp->t_row;
					continue;
				case LCA:
				case SVID:
				case DVID:
				case CVID:
					col = cpass();
				default:
					ttyctl(c, tp, col, c==LCA ? cpass() : 0);
				}
			} else {
norm:
#ifdef DEBUG
printf("gonna call ttxput from norm label in ttwrite in tt0 \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

				ttxput(tp, 0, c);
			}
		}
	}
	if (hqflag) {
		hqrelse(tp);
		putc(QESC, &tp->t_outq);
		putc(HQEND, &tp->t_outq);
		x = TTYSPL();						/* M000 */
		if (tp->t_state & OASLP) {
			tp->t_state &= ~OASLP;
			wakeup((caddr_t)&tp->t_outq);
		}
		splx(x);
	}
out:
	tp->t_tmflag &= ~(QLOCKB|QLOCKI);
	x = tp->t_iscon ? curlevel : TTYSPL();		/* M002 */
#ifdef DEBUGSP
printf("gonna call T_OUTPUT from norm label in ttwrite in tt0 \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	(*tp->t_proc)(tp, T_OUTPUT);
	splx(x);
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 */
#define	LCLESC	0400

ttin(tp, code)
register struct tty *tp;
{
	register c;
	register flg;
	register char *cp;
	ushort nchar, nc;
#ifdef DEBUG
printf("reached ttin in tt0 \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

	if (code == L_BREAK){
		signal(tp->t_pgrp, SIGINT);
		ttyflush(tp, (FREAD|FWRITE));
		return;
	}
	nchar = tp->t_rbuf.c_size - tp->t_rbuf.c_count;
	/* reinit rx control block */
	tp->t_rbuf.c_count = tp->t_rbuf.c_size;
	if (nchar==0)
		return;
	flg = tp->t_iflag;
	/* KMC does all but IXOFF */
	if (tp->t_state&EXTPROC)
		flg &= IXOFF;
	nc = nchar;
	cp = tp->t_rbuf.c_ptr;
	if (nc < cfreelist.c_size || (flg & (INLCR|IGNCR|ICRNL|IUCLC))
		|| tp->t_term) {
			/* must do per character processing */
		for ( ;nc--; cp++) {
			c = *cp;
			if (tp->t_term) {
				c &= 0177;
				if ((c = (*termsw[tp->t_term].t_input) (c,tp))
					== -1)
					continue;
				if (c & CPRES) {
					putc(ESC, &tp->t_rawq);
					putc(c, &tp->t_rawq);
					continue;
				}
			}
			if (c == '\n' && flg&INLCR)
				*cp = c = '\r';
			else if (c == '\r')
				if (flg&IGNCR)
					continue;
				else if (flg&ICRNL)
					*cp = c = '\n';
			if (flg&IUCLC && 'A' <= c && c <= 'Z')
				c += 'a' - 'A';
			if (putc(c, &tp->t_rawq))
				continue;
			sysinfo.rawch++;
		}
		cp = tp->t_rbuf.c_ptr;
	} else {
		/* may do block processing */
		putcb(CMATCH((struct cblock *)cp), &tp->t_rawq);
		sysinfo.rawch += nc;
		/* allocate new rx buffer */
		if ((tp->t_rbuf.c_ptr = getcf()->c_data)
			== ((struct cblock *)NULL)->c_data) {
			tp->t_rbuf.c_ptr = NULL;
			return;
		}
	}

	/*
	** Moved these lines down so that they happen both
	** on block and character processing. Somehow character
	** processing screws up c_count which results in smashing
	** cfreelist.
	*/
	tp->t_rbuf.c_count = cfreelist.c_size;
	tp->t_rbuf.c_size = cfreelist.c_size;


	if (tp->t_rawq.c_cc > TTXOHI) {
		if (flg&IXOFF && !(tp->t_state&TBLOCK))
			(*tp->t_proc)(tp, T_BLOCK);
		if (tp->t_rawq.c_cc > TTYHOG) {
			ttyflush(tp, FREAD);
			return;
		}
	}
#ifdef DEBUG
printf("sanity check 1 in ttin in tt0 \n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	flg = lobyte(tp->t_lflag);
	if (tp->t_outq.c_cc > (tthiwat[tp->t_cflag&CBAUD] + TTECHI))
		flg &= ~(ECHO|ECHOK|ECHONL|ECHOE);
	if (flg) while (nchar--) {
		c = *cp++;
		if (flg&ISIG) {
			if (c == tp->t_cc[VINTR]) {
				signal(tp->t_pgrp, SIGINT);
				if (!(flg&NOFLSH) && tp->t_hqcnt==0)
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VQUIT]) {
				signal(tp->t_pgrp, SIGQUIT);
				if (!(flg&NOFLSH) && tp->t_hqcnt==0)
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VSWTCH]) {
				if (!(flg&NOFLSH) && tp->t_hqcnt==0)
					ttyflush(tp, FREAD);
				(*tp->t_proc)(tp, T_SWTCH);
				continue;
			}
		}
		if (flg&ICANON) {
			if (tp->t_state&CLESC) {
				flg |= LCLESC;
				tp->t_state &= ~CLESC;
			}
			if (c == '\n') {
				if (flg&ECHONL)
					flg |= ECHO;
				tp->t_delct++;
			} else if (c == '\\') {
				tp->t_state |= CLESC;
				if (flg&XCASE) {
					c |= QESC;
					if (flg&LCLESC)
						tp->t_state &= ~CLESC;
				}
			} else if (c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
				tp->t_delct++;
			else if (!(flg&LCLESC)) {
				if (c == tp->t_cc[VERASE] && flg&ECHOE) {
					if (flg&ECHO)
						ttxputi(tp, '\b');
					flg |= ECHO;
					ttxputi(tp, ' ');
					c = '\b';
				} else if (c == tp->t_cc[VKILL] && flg&ECHOK) {
					if (flg&ECHO)
						ttxputi(tp, c);
					flg |= ECHO;
					c = '\n';
				} else if (c == tp->t_cc[VEOF]) {
					flg &= ~ECHO;
					tp->t_delct++;
				}
			}
		}
#ifdef DEBUG
printf("sanity check 2 in ttin in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		if (flg&ECHO) {
			ttxputi(tp, c);
			(*tp->t_proc)(tp, T_OUTPUT);
		}
	}
#ifdef DEBUG
printf("sanity check 3 in ttin in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	if (!(flg&ICANON)) {
		tp->t_state &= ~RTO;
		if (tp->t_rawq.c_cc >= tp->t_cc[VMIN])
			tp->t_delct = 1;
		else if (tp->t_cc[VTIME]) {
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
	}
#ifdef DEBUG
printf("sanity check 4 in ttin in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	if (tp->t_delct && (tp->t_state&IASLP)) {
		tp->t_state &= ~IASLP;
		wakeup((caddr_t)&tp->t_rawq);
	}
}

/*
 * Interrupt interface to ttxput.
 * Checks for High Queue write in progress, and saves characters to be echoed.
 */
ttxputi(tp, c)
register struct tty *tp;
{
#ifdef DEBUG
printf("reached ttxputi in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	if (tp->t_tmflag & QLOCKI) {
		putc(c,&tempq);
		return;
	} else
		ttxput(tp, 0, c);
}

/*
 * Put character(s) on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the base level for output, and from
 * interrupt level for echoing.
 */
ttxput(tp, ncode, ucp)
register struct tty *tp;
union {
	ushort	ch;
	struct cblock *ptr;
} ucp;
{
	register c;
	register flg;
	register unsigned char *cp;
	register char *colp;
	int ctype;
	int cs;
	struct cblock *scf;
	int	x;						/* M000 */

#ifdef DEBUG
printf("reached ttxput in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
/*	x = TTYSPL();						/* M000 */
	/* KMC does all but XCASE, virt term needs CR info for t_col */
	if (tp->t_state&EXTPROC) {
		if (tp->t_term || tp->t_lflag&XCASE)
			flg = tp->t_oflag&(OPOST|OLCUC|ONLRET|ONLCR);
		else
			flg = 0;
	} else
		flg = tp->t_oflag;
	if (ncode == 0) {
		ncode++;
		if (!(flg&OPOST)) {
			sysinfo.outch++;
			putc(ucp.ch, &tp->t_outq);
			return;
		}
		cp = (unsigned char *)&ucp.ch;
		scf = NULL;
	} else {
		if (!(flg&OPOST)) {
			sysinfo.outch += ncode;
			putcb(ucp.ptr, &tp->t_outq);
			return;
		}
		cp = (unsigned char *)&ucp.ptr->c_data[ucp.ptr->c_first];
		scf = ucp.ptr;
	}
#ifdef DEBUG
printf("sanity check 1 in ttxput in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	while (ncode--) {

		c = *cp++;
		if (c >= 0200) {
	/* spl5-0 */
			if (c == QESC && !(tp->t_state&EXTPROC))
				putc(QESC, &tp->t_outq);
			sysinfo.outch++;
			putc(c, &tp->t_outq);
			continue;
		}
		/*
		 * Generate escapes for upper-case-only terminals.
		 */
		if (tp->t_lflag&XCASE) {
			colp = "({)}!|^~'`\\\\";
			while(*colp++)
				if (c == *colp++) {
					ttxput(tp, 0, '\\'|0200);
					c = colp[-2];
					break;
				}
			if ('A' <= c && c <= 'Z')
				ttxput(tp, 0, '\\'|0200);
		}
		if (flg&OLCUC && 'a' <= c && c <= 'z')
			c += 'A' - 'a';
		cs = c;
		/*
		 * Calculate delays.
		 * The numbers here represent clock ticks
		 * and are not necessarily optimal for all terminals.
		 * The delays are indicated by characters above 0200.
		 */
		ctype = partab[c];
		colp = &tp->t_col;
		c = 0;
#ifdef DEBUG
printf("sanity check 2 in ttxput in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		switch (ctype&077) {

		case 0:	/* ordinary */
			(*colp)++;

		case 1:	/* non-printing */
			break;

		case 2:	/* backspace */
			if (flg&BSDLY)
				c = 2;
			if (*colp)
				(*colp)--;
			break;

		case 3:	/* line feed */
top:
			if (tp->t_term) {
				if (tp->t_vrow && tp->t_row >= tp->t_lrow) {
					ttyctl(UVSCN, tp);
					continue;
				}
				if (tp->t_tmflag & SNL) {
					ttyctl(NL, tp);
					continue;
				}
			}
			if (flg&ONLRET)
				goto cr;
			if (tp->t_row < tp->t_lrow)
				tp->t_row++;
			if (flg&ONLCR) {
				if ((!(tp->t_state&EXTPROC)) &&
					!(flg&ONOCR && *colp==0)) {
					sysinfo.outch++;
					putc('\r', &tp->t_outq);
				}
				goto cr;
			}
		nl:
			if (flg&NLDLY)
				c = 5;
			break;

		case 4:	/* tab */
			c = 8 - ((*colp)&07);
			*colp += c;
			if (!(tp->t_state&EXTPROC)) {
				ctype = flg&TABDLY;
				if (ctype == TAB0) {
					c = 0;
				} else if (ctype == TAB1) {
					if (c < 5)
						c = 0;
				} else if (ctype == TAB2) {
					c = 2;
				} else if (ctype == TAB3) {
					sysinfo.outch += c;
					do
						putc(' ', &tp->t_outq);
					while (--c);
					continue;
				}
			}
			break;

		case 5:	/* vertical tab */
			if (flg&VTDLY)
				c = 0177;
			break;

		case 6:	/* carriage return */
			if (flg&OCRNL) {
				cs = '\n';
				goto nl;
			}
			if (flg&ONOCR && *colp == 0)
				continue;
		cr:
			if (!(tp->t_state&EXTPROC)) {
				ctype = flg&CRDLY;
				if (ctype == CR1) {
					if (*colp)
						c = max((*colp>>4) + 3, 6);
				} else if (ctype == CR2) {
					c = 5;
				} else if (ctype == CR3) {
					c = 9;
				}
			}
			*colp = 0;
			break;

		case 7:	/* form feed */
			if (flg&FFDLY)
				c = 0177;
			break;
		}
#ifdef DEBUG
printf("sanity check 3 in ttxput in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		sysinfo.outch++;
		if (tp->t_term && *colp >= 80 && tp->t_row >= tp->t_lrow
			&& tp->t_tmflag & LCF) {
			ttyctl(VHOME, tp);
			ttyctl(DL, tp);
			ttyctl(LCA, tp, 79, tp->t_lrow-1);
			(*colp)++;
		}
                if (tp->t_term==0)
			putc(cs, &tp->t_outq);
		else
			qputc(cs, &tp->t_outq);
		if (!(tp->t_state&EXTPROC)) {
			if (c) {
				if ((c < 32) && flg&OFILL) {
					if (flg&OFDEL)
						cs = 0177;
					else
						cs = 0;
					putc(cs, &tp->t_outq);
					if (c > 3)
						putc(cs, &tp->t_outq);
				} else {
					putc(QESC, &tp->t_outq);
					putc(c|0200, &tp->t_outq);
				}
			}
		}
#ifdef DEBUG
printf("sanity check 4 in ttxput in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		if (*colp >= 80 && tp->t_term && tp->t_tmflag&ANL)
			if (tp->t_tmflag&LCF)
				ttyctl(LCA, tp, 0, tp->t_row+1);
			else {
				if ((flg&ONLCR) == 0)
					ttxputi(tp,'\r');
				cs = '\n';
				goto top;
			}
	}
/*	splx(x);						/* M000 */
#ifdef DEBUG
printf("sanity check 5 in ttxput in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	if (scf != NULL)
		putcf(scf);
}

/*
 * Get next packet from output queue.
 * Called from xmit interrupt complete.
 */

ttout(tp)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	register c;
	register char *cptr;
	register retval;

	extern ttrstrt();

#ifdef DEBUGSP
printf("reached ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	if (tp->t_state&TTIOW && tp->t_outq.c_cc==0) {
		tp->t_state &= ~TTIOW;
		wakeup((caddr_t)&tp->t_oflag);
	}
ttodelay:
	tbuf = &tp->t_tbuf;
	if (hibyte(tp->t_lflag)) {
		if (tbuf->c_ptr) {
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
			tbuf->c_ptr = NULL;
		}
#ifdef DEBUG
printf("sanity check 1 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, (hibyte(tp->t_lflag)&0177)+6);
		hibyte(tp->t_lflag) = 0;
		return(0);
	}
#ifdef DEBUG
printf("sanity check 2 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	retval = 0;
	if (((tp->t_state&EXTPROC) || (!(tp->t_oflag&OPOST))) &&
		tp->t_term==0) {
#ifdef DEBUG
printf("sanity check 3 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		if (tbuf->c_ptr)
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
		if ((tbuf->c_ptr = (char *)getcb(&tp->t_outq)) == NULL)
			return(0);
		tbuf->c_count = ((struct cblock *)tbuf->c_ptr)->c_last -
				((struct cblock *)tbuf->c_ptr)->c_first;
		tbuf->c_size = tbuf->c_count;
		tbuf->c_ptr = &((struct cblock *)tbuf->c_ptr)->c_data
				[((struct cblock *)tbuf->c_ptr)->c_first];
		retval = CPRES;
	} else {			/* watch for timing	*/
#ifdef DEBUG
printf("sanity check 4 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		if (tbuf->c_ptr == NULL) {
			if ((tbuf->c_ptr = getcf()->c_data)
				== ((struct cblock *)NULL)->c_data) {
				tbuf->c_ptr = NULL;
#ifdef DEBUG
printf("sanity check 5 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
				return(0);	/* Add restart? */
			}
		}
		tbuf->c_count = 0;
		cptr = tbuf->c_ptr;
		lastcall = 2;		/* CJH */
		while ((c=getc(&tp->t_outq)) >= 0) {
			if (c == QESC) {
				lastcall = 3;		/* CJH */
				if ((c = getc(&tp->t_outq)) < 0)
					break;
				if (c == HQEND) {
					if (tp->t_term)
						tp->t_hqcnt--;
					continue;
				}
				if (c > 0200 && !(tp->t_state&EXTPROC)) {
					hibyte(tp->t_lflag) = c;
					if (!retval)
						goto ttodelay;
					break;
				}
			}
#ifdef DEBUG
printf("sanity check 6 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
			retval = CPRES;
			*cptr++ = c;
			tbuf->c_count++;
			if (tbuf->c_count >= cfreelist.c_size)
				break;
		}
#ifdef DEBUG
printf("sanity check 7 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		tbuf->c_size = tbuf->c_count;
	}
#ifdef DEBUG
printf("sanity check 8 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

	if (tp->t_state&OASLP &&
		tp->t_outq.c_cc<=ttlowat[tp->t_cflag&CBAUD]) {
		tp->t_state &= ~OASLP;
		wakeup((caddr_t)&tp->t_outq);
	}
#ifdef DEBUG
printf("sanity check 9 in ttout in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	return(retval);
}

tttimeo(tp)
register struct tty *tp;
{
	extern hz;
#ifdef DEBUG
printf("reached ttimeo in tt0\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
	tp->t_state &= ~TACT;
	if (tp->t_lflag&ICANON || tp->t_cc[VTIME] == 0)
		return;
	if (tp->t_rawq.c_cc == 0 && tp->t_cc[VMIN])
		return;
	if (tp->t_state&RTO) {
		tp->t_delct = 1;
		if (tp->t_state&IASLP) {
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	} else {
		tp->t_state |= RTO|TACT;
		timeout(tttimeo, tp, tp->t_cc[VTIME]*(hz/10));
	}
}

/*
 * I/O control interface
 */
ttioctl(tp, cmd, arg, mode)
register struct tty *tp;
union ioctl_arg	arg;
{
	ushort	chg;
	struct termcb termblk;
#ifdef DEBUG
printf("reached ttioctl in tt0.c\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif

	switch(cmd) {
	case LDOPEN:
#ifdef DEBUG
printf("reached LDOPEN in ttioctl in tt0.c\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		if (tp->t_rbuf.c_ptr == NULL) {
			/* allocate RX buffer */
			while((tp->t_rbuf.c_ptr = getcf()->c_data)
				== ((struct cblock *)NULL)->c_data) {
				tp->t_rbuf.c_ptr = NULL;
				cfreelist.c_flag = 1;
				sleep(&cfreelist, TTOPRI);
			}
			tp->t_rbuf.c_count = cfreelist.c_size;
			tp->t_rbuf.c_size  = cfreelist.c_size;
			(*tp->t_proc)(tp, T_INPUT);
		}
		break;

	case LDCLOSE:
#ifdef DEBUG
printf("reached LDCLOSE in ttioctl in tt0.c\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		if (!tp->t_iscon)
			TTYSPL();						/* M000 */
		(*tp->t_proc)(tp, T_RESUME);
		spl0();
		ttywait(tp);
		ttyflush(tp, (FREAD|FWRITE));
		if (tp->t_tbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_tbuf.c_ptr));
			tp->t_tbuf.c_ptr = NULL;
			tp->t_tbuf.c_count = 0;
			tp->t_tbuf.c_size = 0;
		}
		if (tp->t_rbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_rbuf.c_ptr));
			tp->t_rbuf.c_ptr = NULL;
			tp->t_rbuf.c_count = 0;
			tp->t_rbuf.c_size = 0;
		}
		tp->t_tmflag = 0;
		break;

	case LDCHG:
#ifdef DEBUG
printf("reached LDCHG in ttioctl in tt0.c\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		chg = tp->t_lflag^arg.iarg;
		if (!(chg&ICANON))
			break;
		if (tp->t_iscon)
			spl6();
		else	TTYSPL();
		if (tp->t_canq.c_cc) {
			if (tp->t_rawq.c_cc) {
				tp->t_canq.c_cc += tp->t_rawq.c_cc;
				tp->t_canq.c_cl->c_next = tp->t_rawq.c_cf;
				tp->t_canq.c_cl = tp->t_rawq.c_cl;
			}
			tp->t_rawq = tp->t_canq;
			tp->t_canq = ttnulq;
		}
		tp->t_delct = tp->t_rawq.c_cc;
		spl0();
		break;

	case LDGETT:
#ifdef DEBUG
printf("reached LDGETT in ttioctl in tt0.c\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		termblk.st_flgs = tp->t_tmflag;
		termblk.st_termt = tp->t_term;
		termblk.st_crow = tp->t_row;
		termblk.st_ccol = tp->t_col;
		termblk.st_vrow = tp->t_vrow;
		termblk.st_lrow = tp->t_lrow;
		if (copyout((caddr_t)&termblk, arg.stparg, sizeof(termblk)))
			u.u_error = EFAULT;
		break;

	case LDSETT:
#ifdef DEBUG
printf("reached LDSETT in ttioctl in tt0.c\n");
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		if (copyin(arg.stparg, (caddr_t)&termblk, sizeof(termblk))) {
			u.u_error = EFAULT;
			break;
		}
		if ((unsigned)termblk.st_termt >= termcnt) {
			u.u_error = ENXIO;
			break;
		}
		if (termblk.st_termt) {
			(*termsw[termblk.st_termt].t_ioctl)
			    (tp,
			    tp->t_term==termblk.st_termt ? LDCHG : LDOPEN,
			    termblk.st_vrow);
			if (u.u_error)
				break;
			tp->t_vrow = termblk.st_vrow;
			tp->t_term = termblk.st_termt;
			if (termblk.st_flgs&TM_SET)
				tp->t_tmflag = termblk.st_flgs & ~TM_SET;
		} else {
			tp->t_term = 0;
		}
		tp->t_state &= ~CLESC;
		break;
	default:
#ifdef DEBUG
printf("reached default in ttioctl in tt0.c cmd = %x\n",cmd);
#ifdef DEBUGTP
prnttp(tp);
#endif
#endif
		break;
	}
}

/************** ADDITIONS FOR TERMINAL HANDLERS **********************/

/*
 * release the high priority queue for interrupts.
 * copy over any received characters while queue was locked.
 */
hqrelse(tp)
register struct tty *tp;
{
	register c;
#ifdef DEBUG
	printf("\n\ngot to hqrelease!\n\n\n");
#endif

	ttyctl(LCA, tp, colsave, rowsave);
	if (tp->t_iscon)
		spl6();
	else	TTYSPL();
	lastcall = 4;		/* CJH */
	while((c = getc(&tempq)) >= 0)
		ttxput(tp, 0, c);
	tp->t_tmflag &= ~QLOCKI;
	spl0();
}


/*
 * put a character on the output queue,
 * checking first to see if it is a ESC.
 */
qputc(c, qp)
register c;
register qp;
{
	if (c == ESC)
		putc(c, qp);
	putc(c, qp);
}

/* simulate Up Variable SCreeN as common routine */
ttuvscn(tp)
register struct tty *tp;
{
	ttyctl(VHOME, tp);
	ttyctl(DL, tp);
	ttyctl(LCA, tp, 0, tp->t_lrow);
}

/* simulate Down Variable SCreeN as common routine */
ttdvscn(tp)
register struct tty *tp;
{
	ttyctl(VHOME, tp);
	ttyctl(IL, tp);
}

char colpres, rowpres;

ttyctl(ac, tp, acol, arow)
register struct tty *tp;
{
	register char *colp;
	register c;
	int sps;

	c = ac;
	colp = &tp->t_col;
	sps = tp->t_iscon ? spl6() : TTYSPL();

	colpres = *colp;
	rowpres = tp->t_row;
	switch(c) {
	case CUP:
	case DSCRL:
		if (tp->t_row == 0)
			goto out;
		tp->t_row--;
		break;
	case CDN:
	case USCRL:
		if (tp->t_row >= tp->t_lrow)
			goto out;
		tp->t_row++;
		break;
	case UVSCN:
		*colp = 0;
		tp->t_row = tp->t_lrow;
		break;
	case DVSCN:
		*colp = 0;
		tp->t_row = tp->t_vrow;
		break;
	case CRI:
	case STB:
	case SPB:
		if (*colp >= 79)
			goto out;
		(*colp)++;
		break;
	case CLE:
		if (*colp == 0)
			goto out;
		(*colp)--;
		break;
	case HOME:
	case CS:
	case CM:
		tp->t_row = 0;
	case DL:
	case IL:
		*colp = 0;
		break;
	case VHOME:
		*colp = 0;
		tp->t_row = tp->t_vrow;
		c = LCA;
		break;
	case LCA:
		*colp = acol;
		tp->t_row = arow;
		break;
	case ASEG:
		tp->t_row = (tp->t_row+24)%(tp->t_lrow+1);
		break;
	case NL:
		if (tp->t_row < tp->t_lrow)
			tp->t_row++;
	case CRTN:
		*colp = 0;
		break;
	case SVID:
		tp->t_dstat |= acol;
		c = DVID;
		break;
	case CVID:
		tp->t_dstat &= ~acol;
		c = DVID;
		break;
	case DVID:
		tp->t_dstat = acol;
		break;
	}
	(*termsw[tp->t_term].t_output)(c, tp);
    out:
	splx(sps);
}
