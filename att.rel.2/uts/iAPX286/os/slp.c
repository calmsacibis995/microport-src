/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)slp.c	1.27 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/text.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/fp.h"

#define	NHSQUE	64	/* must be power of 2 */
#define	sqhash(X)	(&hsque[((int)X >> 3) & (NHSQUE-1)])
struct proc *hsque[NHSQUE];
char	runin, runout, runrun, curpri;
struct proc *curproc, *runq;

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
#define	TZERO	10
sleep(chan, disp)
caddr_t chan;
{
	register struct proc *rp = u.u_procp;
	register struct proc **q = sqhash(chan);
	register s;

	s = splhi();
	if (panicstr) {
		spl0();
		splx(s);
		return(0);
	}
	rp->p_stat = SSLEEP;
	rp->p_wchan = chan;
	rp->p_link = *q;
	*q = rp;
	if (rp->p_time > TZERO)
		rp->p_time = TZERO;
	if ((rp->p_pri = (disp&PMASK)) > PZERO) {
		if (rp->p_sig && issig()) {
			rp->p_wchan = 0;
			rp->p_stat = SRUN;
			*q = rp->p_link;
			spl0();
			goto psig;
		}
		spl0();
		if (runin != 0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
		swtch();
		if (rp->p_sig && issig())
			goto psig;
	} else {
		spl0();
		swtch();
	}
	splx(s);
	return(0);

	/*
	 * If priority was low (>PZERO) and there has been a signal,
	 * if PCATCH is set, return 1, else
	 * execute non-local goto to the qsav location.
	 */
psig:
	splx(s);
	if (disp&PCATCH)
		return(1);
	longjmp(u.u_qsav);
	/* NOTREACHED */
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
register caddr_t chan;
{
	register struct proc *p;
	register struct proc **q;
	register s;


	s = splhi();
	for (q = sqhash(chan); p = *q; )
		if (p->p_wchan==chan && p->p_stat==SSLEEP) {
			p->p_stat = SRUN;
			p->p_wchan = 0;
			/* take off sleep queue, put on run queue */
			*q = p->p_link;
			p->p_link = runq;
			runq = p;
			if (!(p->p_flag&SLOAD)) {
				p->p_time = 0;
				/* defer setrun to avoid breaking link chain */
				if (runout > 0)
					runout = -runout;
			} else if (p->p_pri < curpri)
				runrun++;
		} else
			q = &p->p_link;
	if (runout < 0) {
		runout = 0;
		setrun(&proc[0]);
	}
	splx(s);
}

setrq(p)
register struct proc *p;
{
	register struct proc *q;
	register s;

	s = splhi();
	for(q=runq; q!=NULL; q=q->p_link)
		if (q == p) {
			printf("proc on q\n");
			goto out;
		}
	p->p_link = runq;
	runq = p;
out:
	splx(s);
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
register struct proc *p;
{
	register struct proc **q;
	register s;

	s = splhi();
	if (p->p_stat == SSLEEP) {
		/* take off sleep queue */
		for (q = sqhash(p->p_wchan); *q != p; q = &(*q)->p_link) ;
		*q = p->p_link;
		p->p_wchan = 0;
	} else if (p->p_stat == SRUN) {
		/* already on run queue - just return */
		splx(s);
		return;
	}
	/* put on run queue */
	p->p_stat = SRUN;
	p->p_link = runq;
	runq = p;
	if (!(p->p_flag&SLOAD)) {
		p->p_time = 0;
		if (runout > 0) {
			runout = 0;
			setrun(&proc[0]);
		}
	} else if (p->p_pri < curpri)
		runrun++;
	splx(s);
}

/*
 * The main loop of the scheduling (swapping) process.
 * The basic idea is:
 *  see if anyone wants to be swapped in;
 *  swap out processes until there is room;
 *  swap him in;
 *  repeat.
 * The runout flag is set whenever someone is swapped out.
 * Sched sleeps on it awaiting work.
 *
 * Sched sleeps on runin whenever it cannot find enough
 * memory (by swapping out or otherwise) to fit the
 * selected swapped process.  It is awakened when the
 * memory situation changes and in any case once per second.
 */

sched()
{
	register struct proc *rp, *p;
	register outage, inage;
	register struct proc *inp;
	int maxbad;
	int tmp;
	int inmem;			/* number of procs in memory */

	/*
	 * find user to swap in;
	 * of users ready, select one out longest
	 */

loop:
	spl7();
	outage = -20000;
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++)
	if (rp->p_stat==SRUN && (rp->p_flag&SLOAD) == 0 &&
	    rp->p_time > outage) {
		p = rp;
		outage = rp->p_time;
	}
	/*
	 * If there is no one there, wait.
	 */
	if (outage == -20000) {
		runout++;
		sleep((caddr_t)&runout, PSWP);
		goto loop;
	}
	spl0();

	/*
	 * See if there is memory for that process;
	 * if so, swap it in.
	 */

	if (swapin(p))
		goto loop;

	/*
	 * none found.
	 * look around for memory.
	 * Select the largest of those sleeping
	 * at bad priority; if none, select the oldest.
	 */

	spl7();
	inp = p;		/* we are trying to swap this guy in */
	p = NULL;
	maxbad = 0;
	inage = 0;
	inmem = 0;
	for (rp = &proc[0]; rp < (struct proc *)v.ve_proc; rp++) {
		if (rp->p_stat==SZOMB)
			continue;
		if (rp == inp)
			continue;
		if ((rp->p_flag&(SSPART|SLOCK)) == SSPART) {
			p = rp;
			maxbad = 1;
			inmem++;
			break;
		}
		if ((rp->p_flag&(SSYS|SLOAD))==SLOAD)
			inmem++;
		if ((rp->p_flag&(SSYS|SLOCK|SLOAD))!=SLOAD)
			continue;
		if (rp->p_textp && rp->p_textp->x_flag&XLOCK)
			continue;
		if (rp->p_stat==SSLEEP || rp->p_stat==SSTOP) {
			tmp = rp->p_pri - PZERO + rp->p_time;
			if (maxbad < tmp) {
				p = rp;
				maxbad = tmp;
			}
		} else if (maxbad<=0 && rp->p_stat==SRUN) {
			tmp = rp->p_time + rp->p_nice - NZERO;
			if (tmp > inage) {
				p = rp;
				inage = tmp;
			}
		}
	}
	spl0();
	/*
	 * inmem tells us whether there is anyone in memory
	 * who might go out or be able to be thrown out soon.
	 * We use this to try to keep the system alive when
	 * memory becomes fragmented or otherwise available.
	 * What otherwise happens is that the scheduler decides
	 * that this is the guy he wants in memory, and he won't
	 * let anyone else in until this guy gets in first.  If
	 * there simply isn't enough unlocked memory for this
	 * guy right now then we set his outage down to give
	 * others a chance.  The right way to fix this is to
	 * have a maxmem array which dynamically changes as the
	 * locked memory situation changes, and have that checked
	 * above when it selects the process to bring in.  See
	 * the comments in machdep.c function chksize().
	 */
	if (inmem==0) {
		/* There wasn't room to bring this guy in, so we	*/
		/* were looking for someone to throw out, and there	*/
		/* wasn't anyone we might even consider.  Obviously	*/
		/* memory has become unavailable due to locking of	*/
		/* processes or some other means, so to avoid having	*/
		/* no one at all be able to run, we set this guy's	*/
		/* outage down.						*/
		inp->p_time = -8;
	}

	/*
	 * Swap found user out if sleeping at bad pri,
	 * or if he has spent at least 2 seconds in memory and
	 * the swapped-out process has spent at least 2 seconds out.
	 * Otherwise wait a bit and try again.
	 */
	if (maxbad>0 || (outage>=2 && inage>=2)) {
		p->p_flag &= ~SLOAD;
		xswap(p, 1, 0);
		goto loop;
	}
	spl7();
	runin++;
	sleep((caddr_t)&runin, PSWP);
	goto loop;
}

/*
 * Swap a process in.
 * Allocate data and possible text separately.
 * It would be better to do largest first.
 */
swapin(p)
register struct proc *p;
{
	register struct text *xp;
	register ushort a;
	register x;

	if ((a = malloc(coremap, p->p_size)) == NULL)
		return(0);
	if (xp = p->p_textp) {
		xlock(xp);
		if (xp->x_ccount == 0) {
			if ((x = malloc(coremap, xp->x_size)) == NULL) {
				xunlock(xp);
				mfree(coremap, p->p_size, a);
				return(0);
			}
			xp->x_caddr = x;
			if ((xp->x_flag&XLOAD) == 0)
				swap(p,xp->x_daddr,x,xp->x_size,B_READ);
		}
		xp->x_ccount++;
		xunlock(xp);
	}
	swap(p, p->p_addr, a, p->p_size, B_READ);
	mfree(swapmap, ctod(p->p_size), p->p_addr);
	p->p_addr = a;
	adjustldt(p, a); 
	if ( p->p_textp )
		adjusttext(p, xp->x_caddr);
	p->p_flag |= SLOAD;
	p->p_time = 0;
	return(1);
}

/*
 * put the current process on
 * the Q of running processes and
 * call the scheduler.
 */
qswtch()
{
	setrq(u.u_procp);
	swtch();
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */
swtch()
{
        register n;
        register struct proc *p, *q, *pp, *pq;

        /*
         * If not the idle process, resume the idle process.
         */
        sysinfo.pswitch++;
 
	if (u.u_procp != &proc[0]) {

		/* Everyone except proc0 just gives up control */
		dispatch(&proc[0]);
		return;
	}
loop:
        spl7();
        runrun = 0;
        pp = NULL;
        q = NULL;
        n = 128;
        /*
         * Search for highest-priority runnable process
         */
        if (p = runq) do {
                if ((p->p_flag&SLOAD) && p->p_pri <= n) {
                        pp = p;
                        pq = q;
                        n = p->p_pri;
                }
                q = p;
        } while (p = p->p_link);
        /*
         * If no process is runnable, idle.
         */
        p = pp;
        if (p == NULL) {
                curpri = PIDLE;
                curproc = &proc[0];
                idle();
                goto loop;
        }
        q = pq;
        if (q == NULL)
                runq = p->p_link;
        else
                q->p_link = p->p_link;
        curpri = n;
        curproc = p;
        spl0();

	/*
	 * If we want to run proc0 we just return (since we already are proc0)
	 * otherwise we give the desired process control.
	 */
	if (p == &proc[0])
		return;
	else
		dispatch(p);

        p = curproc;
        switch(p->p_stat) {
        case SZOMB:
		/*
		 * in the case that the current process just exited
		 * we free its memory
		 */
                mfree(coremap, p->p_size, p->p_addr);
		zapprocslots(p);
                break;
        }
	goto loop;
}

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process, 0 in the old.
 */
newproc(i)
{
	register int (**forkptr)();
	extern int (*dev_fork[])();
	register struct proc *rpp, *rip;
	register n;
	register a;
	struct proc *pend;
	static mpid;
	register s;

	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */
	rpp = NULL;
retry:
	mpid++;
	if (mpid >= MAXPID) {
		mpid = 0;
		goto retry;
	}
	rip = &proc[0];
	n = (struct proc *)v.ve_proc - rip;
	a = 0;
	do {
		if (rip->p_stat == NULL) {
			if (rpp == NULL)
				rpp = rip;
			continue;
		}
		if (rip->p_pid==mpid)
			goto retry;
		if (rip->p_uid == u.u_ruid)
			a++;
		pend = rip;
	} while(rip++, --n);
	if (rpp==NULL) {
		if ((struct proc *)v.ve_proc >= &proc[v.v_proc]) {
			if (i) {
				syserr.procovf++;
				u.u_error = EAGAIN;
				return(-1);
			} else
				panic("no procs");
		}
		rpp = (struct proc *)v.ve_proc;
	}
	if (rpp > pend)
		pend = rpp;
	pend++;
	v.ve_proc = (char *)pend;
	if (u.u_uid && u.u_ruid) {
		if (rpp == &proc[v.v_proc-1] || a > v.v_maxup) {
			u.u_error = EAGAIN;
			return(-1);
		}
	}
	/*
	 * make proc entry for new proc
	 */

	rip = u.u_procp;
	rpp->p_stat = SRUN;
	rpp->p_clktim = 0;
	rpp->p_flag = SLOAD;
	rpp->p_uid = rip->p_uid;
	rpp->p_suid = rip->p_suid;
	rpp->p_pgrp = rip->p_pgrp;
	rpp->p_nice = rip->p_nice;
	rpp->p_textp = rip->p_textp;
	rpp->p_pid = mpid;
	rpp->p_ppid = rip->p_pid;
	rpp->p_time = 0;
	rpp->p_cpu = rip->p_cpu;
	rpp->p_pri = PUSER + rip->p_nice - NZERO;
	rpp->p_addr = rip->p_addr;
	rpp->p_ssize = rip->p_ssize;
	rpp->p_lsize = rip->p_lsize;

	/*
	 * make duplicate entries
	 * where needed
	 */

	for(n=0; n<NOFILE; n++)
		if (u.u_ofile[n] != NULL)
			u.u_ofile[n]->f_count++;
	if (rpp->p_textp != NULL) {
		rpp->p_textp->x_count++;
		rpp->p_textp->x_ccount++;
		rpp->p_flag |= STEXT;
	}
	u.u_cdir->i_count++;
	if (u.u_rdir)
		u.u_rdir->i_count++;

	pslot[rpp - proc] = pslot[rip - proc];

	for (forkptr = &dev_fork[0]; *forkptr; forkptr++)
		(**forkptr)(rpp, rip);

	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 */
	u.u_procp = rpp;
	curproc = rpp;
	rpp->p_size = rip->p_size;

	/*
	 * save the floating point state for this process
	 */
	 fpsave( FPCHECK );

	/*
	 * When the resume is executed for the new process,
	 * here's where it will resume.
	 */
	if (save(u.u_ssav)) {
		return(1);
	}
	/*
	 * If there is not enough memory for the
	 * new process, swap out the current process to generate the
	 * copy.
	 */
	if (procdup(rpp) == NULL) {
		rip->p_stat = SIDL;
		/*
		** set up child's proc to be available and present
		*/
		unzapslots( rpp );
		xswap(rpp, 0, 0);
		/* reloadptr() reloads the parent proc's tss gate selector
		   into the task register */
		reloadptr((&pslot[rip - proc].pa_tss_d - gdt) << 3);
		rip->p_stat = SRUN;
	}
	u.u_procp = rip;
	curproc = rip;
	setrq(rpp);
	rpp->p_flag |= SSWAP;
	u.u_rval1 = rpp->p_pid;		/* parent returns pid of child */
	return(0);
}

/*
 * Change the size of the data+stack regions of the process.
 * If the size is shrinking, it's easy-- just release the extra memory.
 * If it's growing, and there is memory, just allocate it
 * and copy the image, taking care to reset registers to account
 * for the fact that the system's stack has moved.
 * If there is no memory, arrange for the process to be swapped
 * out after adjusting the size requirement-- when it comes
 * in, enough memory will be allocated.
 *
 * After the expansion, the caller will take care of copying
 * the user's stack towards or away from the data area.
 */
expand(newsize)
ushort newsize;
{
	register i;
	register ushort n;
	register struct proc *p;
	register ushort a1, a2;
	/*
	** we can't have x automatic because we change it after the 
	** process image is copied,  we can make x static because
	** we only change it within the mutual exculsion region.
	*/
	static uint x;

	p = u.u_procp;
	n = p->p_size;
	p->p_size = newsize;
	a1 = p->p_addr;
	if (n >= newsize) {
		mfree(coremap, n-newsize, a1+newsize);
		return;
	}
	if ((a2 = malloc(coremap, newsize)) == NULL) {
		xque(p, SXBRK, n);
		return;
	}
	for(i=0; i<n; i++)
		copyseg(a1+i, a2+i);
	/*
	 * We've got to be very careful here because we're moving
	 * the running udot & process around underneath us
	 * We will be okay as long as:
	 *	(a) we don't load any register (cs, ds, es, ss, ldtr, tr)
	 *	    with a process selector (any in the pslot or the LDT)
	 *	    before we do the reloadregs
	 *	(b) no interrupts come in (implied by (a))
	 *	(c) we don't expect a change to the process image made after
	 *	    the copy to remain after the reloadreg() as the changed
	 *	    stuff isn't copied by reloadregs.  this is why x is a
	 *	    static rather than an automatic variable.
	 */
	x = spl7();
	p->p_addr = a2;
	adjustldt(p, a2);
	reloadregs();
	splx(x);
	mfree(coremap, n, a1);
}

/*
 * dispatch adjusts the TSS if p is a new proc
 * and jumps to the TSS.
 */
dispatch(p)
struct proc *p;
{
	struct user *up;
	struct seg_desc *sdp;
	long a;

	if (p->p_flag&SSWAP) {
		/*
		 * build a descriptor to allow us to look at the
		 * new processes udot and then copy u_ssav into
		 * the tss so we restart correctly
		 */
		sdp = gdt + SWTCH_SCRATCH_SEL;
		a = ctob((long)p->p_addr);
		sdp->sd_hibase = lobyte(hiword(a));
		sdp->sd_lowbase = loword(a);
		sdp->sd_limit = ctob(USIZE) - 1;
		sdp->sd_access = ACC_KDATA;
		up = (struct user *)gstokv(SWTCH_SCRATCH_SEL);
		up->u_tss.ts_ax = 1;
		up->u_tss.ts_sp = up->u_ssav[0];
		up->u_tss.ts_bp = up->u_ssav[1];
		up->u_tss.ts_ss = up->u_ssav[2];
		up->u_tss.ts_ds = up->u_ssav[3];
		up->u_tss.ts_ip = up->u_ssav[4];
		up->u_tss.ts_cs = up->u_ssav[5];
		up->u_tss.ts_flags = up->u_ssav[6];
		up->u_tss.ts_ldt = ((&pslot[p - proc].pa_ldt_d - gdt) << 3)
		    | GDT_TI | KER_DPL;
		up->u_ldtadv =
		    (struct seg_desc *)gstokv(&pslot[p - proc].pa_ldt_ad - gdt);
		p->p_flag &= ~SSWAP;
	}

	twitch( 0, ((&pslot[p-proc].pa_tss_d - gdt)<<3)|GDT_TI|KER_DPL );
}
