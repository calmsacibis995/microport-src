static char *uportid = "@(#)sig.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)sig.c	1.23 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/mmu.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/reg.h"
#include "sys/text.h"
#include "sys/seg.h"
#include "sys/var.h"
#include "sys/psl.h"
#include "sys/fp.h"

/*
 * Priority for tracing
 */
#define	IPCPRI	PZERO

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */
struct
{
	int	ip_lock;
	int	ip_req;
	int	*ip_addr;
	int	ip_data;
} ipc;

/*
 * Send the specified signal to
 * all processes with 'pgrp' as
 * process group.
 * Called by tty.c for quits and
 * interrupts.
 */
signal(pgrp, sig)
register pgrp;
{
	register struct proc *p;

	if (pgrp == 0)
		return;
	for(p = &proc[1]; p < (struct proc *)v.ve_proc; p++)
		if (p->p_pgrp == pgrp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
register struct proc *p;
register sig;
{

	sig--;
	if (sig < 0 || sig >= NSIG)
		return;
	p->p_sig |= 1L<<sig;
	if (p->p_stat == SSLEEP && p->p_pri > PZERO)
		setrun(p);
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
issig()
{
	register n;
	register struct proc *p, *q;

	p = u.u_procp;
	while(p->p_sig) {
		n = fsig(p);
		if (n == SIGCLD) {
			if (u.u_signal[SIGCLD-1] == SIG_IGN) {
				for (q = &proc[1];
				 q < (struct proc *)v.ve_proc; q++)
					if (p->p_pid == q->p_ppid &&
					 q->p_stat == SZOMB)
						freeproc(q, 0);
			} else if (u.u_signal[SIGCLD-1])
				return(n);
		} else if (n == SIGPWR) {
			if (u.u_signal[SIGPWR-1] &&
				(u.u_signal[SIGPWR-1] != SIG_IGN))
				return(n);
		} else if ((u.u_signal[n-1] != SIG_IGN) || (p->p_flag&STRC))
			return(n);
		p->p_sig &= ~(1L<<(n-1));
	}
	return(0);
}

/*
 * Enter the tracing STOP state.
 * In this state, the parent is
 * informed and the process is able to
 * receive commands from the parent.
 */
stop()
{
	register struct proc *pp, *cp;

loop:
	cp = u.u_procp;
	if (cp->p_ppid != 1)
	for (pp = &proc[0]; pp < (struct proc *)v.ve_proc; pp++)
		if (pp->p_pid == cp->p_ppid) {
			wakeup((caddr_t)pp);
			cp->p_stat = SSTOP;
			swtch();
			if ((cp->p_flag&STRC)==0 || procxmt())
				return;
			goto loop;
		}
	exit(fsig(u.u_procp));
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if (issig())
 *		psig();
 */
psig(arg)
{
	register n;
	int	(*p)();
	register struct proc *rp;

	rp = u.u_procp;
	if (rp->p_flag&STRC)
	{
		/*
		** save the floating point registers
		*/
		fpsave( FPCHECK );
		stop();
	}
	n = fsig(rp);
	if (n==0)
		return;
	rp->p_sig &= ~(1L<<(n-1));
	if ((p=u.u_signal[n-1]) != 0) {
		if (p == SIG_IGN)
			return;
		u.u_error = 0;
		if (n != SIGILL && n != SIGTRAP)
			u.u_signal[n-1] = 0;
		if(sendsig(p, n, arg))
			n = SIGSEGV;
		else
			return;
	}
	switch(n) {

	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGIOT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
		if (core())
			n += 0200;
	}
	exit(n);
}

/*
 * find the signal in bit-position
 * representation in p_sig.
 */
fsig(p)
struct proc *p;
{
	register i;
	register long n;

	n = p->p_sig;
	for(i=1; i<=NSIG; i++) {
		if (n & 1L)
			return(i);
		n >>= 1;
	}
	return(0);
}

/*
 * Create a core image on the file "core"
 *
 * It writes the process' u_area, LDT, and all of its data & stack segments.
 */
core()
{
	register struct inode *ip;
	register s;
	register struct seg_desc *sp;	/* ptr to LDT entry */
	struct seg_desc *ep;		/* ptr to spot after last LDT entry */
	extern schar();

	if (u.u_uid != u.u_ruid)
		return(0);
	u.u_error = 0;
	u.u_dirp = "core";
	ip = namei(schar, 1);
	if (ip == NULL) {
		if (u.u_error)
			return(0);
		ip = maknode(0666);
		if (ip==NULL)
			return(0);
	}
	if (!access(ip, IWRITE) && (ip->i_mode&IFMT) == IFREG) {
		/*
		** save the floating point registers
		*/
		fpsave( FPCHECK );

		itrunc(ip);
		u.u_offset = 0;
		u.u_base = (caddr_t)&u;
		u.u_count = ctob(USIZE);
		u.u_segflg = 1;
		u.u_limit = (daddr_t)ctod(MAXMEM);
		u.u_fmode = FWRITE;
		writei(ip);

		/* Write out the LDT. */
		u.u_base = (caddr_t)u.u_ldtadv;
		u.u_count = ctob(u.u_lsize);
		if(u.u_count == 0) {

			/* Full segment overflowed u_count. */
			/* Write 1 click followed by remainder. */
			u.u_count = ctob(1);
			writei(ip);
			u.u_count = ctob((long)stoc(1) - 1);
		}
		writei(ip);

		/* If stack not in data, dump the stack segment. */
		u.u_segflg = 0;
		if(u.u_ssize) {

			/* Verify LDT entry for user stack. */
			sp = u.u_ldtadv + STACK_SEL;
			if(sp->sd_access & DSC_PRESENT &&
				!(sp->sd_access & SD_CODE)) {

				/* Stack is present and not code. */
				u.u_base = (caddr_t)lstouv(STACK_SEL);
				if(sp->sd_access & SD_EXPND_DN)

					/* Add in offset to start of stack,
						since it expands down. */
					u.u_base += ((ushort)(sp->sd_limit + 1));
				if((u.u_count = ctob(u.u_ssize)) == 0) {

					/* Break into 2 writes if full segment. */
					u.u_count = ctob(1);
					writei(ip);
					u.u_count = ctob(u.u_ssize - 1);
				}
				writei(ip);
			}
		}

		/* Loop through the LDT dumping data segments. */
		/* End point is the end of the allocated LDT. */
		ep = (struct seg_desc *)((char *)u.u_ldtadv + ctob(u.u_lsize));

		/* Start point 1st LDT entry after CODE segment selectors. */
		sp = u.u_ldtadv + CODE1_SEL + 1;
		if(u.u_model & U_MOD_MTEXT)

			/* Skip code segments in multi code segment models. */
			while(sp < ep && (ACC_UCODE) ==
				(sp->sd_access & (ACC_UCODE)))
				sp++;

		/* Write out all the remaining segments. */
		for(;sp < ep;sp++) {
			if((sp->sd_access & (DSC_PRESENT | DSC_SEG)) !=
				(DSC_PRESENT | DSC_SEG))

				/* Skip if not active segment. */
				continue;
			u.u_base = lstouv(sp - u.u_ldtadv);
			if(sp->sd_access & SD_EXPND_DN) {

				/* Expand down segments need adjustment. */
				u.u_base += ((ushort)(sp->sd_limit + 1));
				u.u_count = ctob(stoc(1)) - sp->sd_limit;
			} else
				u.u_count = sp->sd_limit + 1L;
			if(u.u_count == 0) {

				/* Full segment overflowed u_count. */
				u.u_count = ctob(1);
				writei(ip);
				u.u_count = ctob((long)stoc(1) - 1);
			}
			writei(ip);
		} 
	} else
		u.u_error = EACCES;
	iput(ip);
	return(u.u_error==0);
}

/*
 * grow the stack to include the SP
 * true return if successful.
 */

grow(sp)
unsigned sp;
{
	register si, i;
	register struct proc *p;
	unsigned a;
	long al;
	struct seg_desc *sdp;
	unsigned min();

	if (!(u.u_model & U_MOD_SSTACK))	/* fixed stack always fails */
		return(0);

	/* adjust sp so we always grab at least SINCR clicks */
	/* the 286 fault sp is usually legal because of instruction restart */
	sp = min(sp, (unsigned)(ctob((long)stoc(1))-ctob((long)u.u_ssize)));

	/* make sure we grab at least SINCR or all the rest */
	if (sp <= ctob(SINCR))
		sp = 0;
	else
		sp -= ctob(SINCR);
	si = btoc(ctob((long)stoc(1))-sp) - u.u_ssize;
	/* we can't grow the stack so fail */
	if (si == 0)
		return(0);

	/* stack can only grow to 1/2k from full segment */
	if (u.u_ssize+si > stoc(1) - btoc(NBPC))
		return(0);

	if (chksize(u.u_tsize, u.u_dsize, u.u_ssize+si, u.u_lsize))
		return(0);
	p = u.u_procp;
	expand(p->p_size+si);
	a = p->p_addr + p->p_size;
	for(i=u.u_ssize; i; i--) {
		a--;
		copyseg(a-si, a);
	}
	for(i=si; i; i--)
		clearseg(--a);
	u.u_ssize += si;
	p->p_ssize += si;

	sdp = u.u_ldtadv + STACK_SEL;
	al = ctob((long)p->p_addr + p->p_size - stoc(1));
	sdp->sd_hibase = lobyte(hiword(al));
	sdp->sd_lowbase = loword(al);
	sdp->sd_limit = (ctob((long)stoc(1)) - ctob((long)u.u_ssize)) - 1;
	return(1);
}

/*
 * sys-trace system call.
 */
ptrace()
{
	register struct proc *p;
	register struct a {
		int	req;
		int	pid;
		int	*addr;
		int	data;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->req <= 0) {
		u.u_procp->p_flag |= STRC;
		return;
	}
	for (p=proc; p < (struct proc *)v.ve_proc; p++) 
		if (p->p_stat==SSTOP
		 && p->p_pid==uap->pid
		 && p->p_ppid==u.u_procp->p_pid)
			goto found;
	u.u_error = ESRCH;
	return;

    found:
	while (ipc.ip_lock)
		sleep((caddr_t)&ipc, IPCPRI);
	ipc.ip_lock = p->p_pid;
	ipc.ip_data = uap->data;
	ipc.ip_addr = uap->addr;
	ipc.ip_req = uap->req;
	p->p_flag &= ~SWTED;
	setrun(p);
	while (ipc.ip_req > 0)
		sleep((caddr_t)&ipc, IPCPRI);
	u.u_rval1 = ipc.ip_data;
	if (ipc.ip_req < 0)
		u.u_error = EIO;
	ipc.ip_lock = 0;
	wakeup((caddr_t)&ipc);
}

#define PSMASK	(PS_T | PS_IE | PS_IOPL | PS_NT)
int ipcreg[] = { ES, DS, DI, SI, BP, BX, DX, CX, AX, IP, SP };

/*
 * Code that the child process
 * executes to implement the command
 * of the parent process in tracing.
 */
procxmt()
{
	register int i;
	register *p;
	register struct text *xp;

	if (ipc.ip_lock != u.u_procp->p_pid)
		return(0);
	i = ipc.ip_req;
	ipc.ip_req = 0;
	wakeup((caddr_t)&ipc);
	switch (i) {

	/* read user I */
	case 1:
		ipc.ip_data = fuiword((caddr_t)ipc.ip_addr);
		break;

	/* read user D */
	case 2:
		ipc.ip_data = fuword((caddr_t)ipc.ip_addr);
		break;

	/* read u */
	case 3:
		i = (int)ipc.ip_addr;
		if (i<0 || i >= ctob(USIZE))
			goto error;
		ipc.ip_data = ((physadr)&u)->r[i>>1];
		break;

	/* write user I */
	/* Must set up to allow writing */
	case 4:
		/*
		 * If text, must assure exclusive use
		 */
		if (xp = u.u_procp->p_textp) {
			if (xp->x_count!=1 || xp->x_iptr->i_mode&ISVTX)
				goto error;
			xp->x_iptr->i_flag &= ~ITEXT;
		}
		/* kinda slow, huh? */
		chgprot((caddr_t)ipc.ip_addr, RW);
		i = suiword((caddr_t)ipc.ip_addr, 0);
		suiword((caddr_t)ipc.ip_addr, ipc.ip_data);
		chgprot((caddr_t)ipc.ip_addr, RO);
		if (i<0)
			goto error;
		if (xp)
			xp->x_flag |= XWRIT;
		break;

	/* write user D */
	case 5:
		if (suword((caddr_t)ipc.ip_addr, 0) < 0)
			goto error;
		suword((caddr_t)ipc.ip_addr, ipc.ip_data);
		break;

	/* write u */
	case 6:
		i = (int)ipc.ip_addr;
		p = (int *)&((physadr)&u)->r[i>>1];
		for (i = 0; i < sizeof(ipcreg)/sizeof(int); i++)
			if (p == &u.u_ar0[ipcreg[i]])
				goto ok;
		if (p == &u.u_ar0[FLGS]) {
			/* keep the user's mitts off my bits */
			ipc.ip_data = (u.u_ar0[FLGS] & PSMASK)
				| (ipc.ip_data & ~PSMASK);
			goto ok;
		} else {
			/*
			** if the guy wants to change cs, make
			** sure he changed it to a valid segment
			*/
			if (p == &u.u_ar0[CS])
				if (useracc(u.u_ar0[IP], *p, 1, 1) &&
					useracc(u.u_ar0[IP],*p,1,0) == 0)
						goto ok;
		}

		goto error;

	ok:
		*p = ipc.ip_data;
		break;

	/* set signal and continue */
	/* one version causes a trace-trap */
	case 9:
		u.u_ar0[FLGS] |= PS_T;
	case 7:
		if ((long)ipc.ip_addr != 1L)
		{
			if (useracc(ipc.ip_addr, 1, 1) &&
				useracc(ipc.ip_addr,1,0) == 0)
					*(int **)(&u.u_ar0[IP]) = ipc.ip_addr;
			else
				goto error;
		}
		u.u_procp->p_sig = 0L;
		if (ipc.ip_data)
			psignal(u.u_procp, ipc.ip_data);
		return(1);

	/* force exit */
	case 8:
		exit(fsig(u.u_procp));

	default:
	error:
		ipc.ip_req = -1;
	}
	return(0);
}
