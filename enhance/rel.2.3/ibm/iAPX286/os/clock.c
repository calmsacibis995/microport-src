static char *uportid = "@(#)clock.c	Microport Rev Id 2.3  6/16/87";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)clock.c	1.12 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/callo.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/psl.h"
#include "sys/var.h"
#include "sys/mmu.h"
#include "sys/reg.h"
#include "sys/trap.h"
#ifdef ATMERGE
#include "sys/realmode.h"
#endif /* ATMERGE */

/*
 * Modification History:
 * The upgrade to the IBM AT requires a modification to track the real time 
 * clock, and correct system time and timeouts for lost ticks.
 */

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock ( not on 286 )
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	alarm clock signals
 *	jab the scheduler
 */

#define	PRF_ON	1
extern	prfstat;

time_t	time, lbolt;
unsigned int inclock;	/* flag to check if we are in clock */

short hz = HZ;		/* patchable time variable */

#ifdef NIBMAT
int	clkprint = 0;
static int chsecs = 0;
static time_t cmossecs = 0;
#endif NIBMAT

clock( cs, pc )
unsigned int	cs;
caddr_t		pc;
{
	register struct proc *pp;
	register a, oticks;
	static short lticks, lostticks;
	static rqlen, sqlen;
	extern caddr_t waitloc;
	unsigned int	x;
	time_t	tempsecs, cursecs;
#ifdef ATMERGE 
	extern struct sw_data usw_data;
	extern struct proc *dosprocp;
#endif ATMERGE 

	/*
	** signify that we are in the clock routine
	** try to catch lost ticks.
	*/
	oticks = lticks;
	--lticks;
	if ( ++inclock > 1 ) {
		--inclock;
#ifndef	PICFIX1
		eoi( CLKVEC );
#endif /* ! PICFIX1 */ 
		return;
		}

	/*
	 * if panic stop clock
	 */
	if (panicstr) {
		clkreld(0);
#ifndef	PICFIX1
		eoi( CLKVEC );
#endif /* ! PICFIX1 */ 
		inclock = 0;
		return;
	}

#ifndef	PICFIX1
	/*
	** acknowledge the interrupt so we can get more immediately
	*/
	eoi( CLKVEC );
#endif /* ! PICFIX1 */ 
#ifdef	PICFIX3
	asm("	sti");
#endif	/* PICFIX3 */

	/*
	 * if any callout active, update first non-zero time
	 */

	if (callout[0].c_func != NULL) {
		register struct callo *cp;
		cp = &callout[0];
		while(cp->c_time<=0 && cp->c_func!=NULL)
			cp++;
		/* reclaim lost ticks */
		cp->c_time -= lostticks + (oticks - lticks); /* */
/*		cp->c_time--; /* */
		oticks = lticks;
	}

	/*
	** do callouts 
	*/
	/*
	** the call to timein is used instead of the
	** original call to timepoke() because there
	** is no scheduling interrupt in this implementation
	*/
	if (callout[0].c_time <= 0)
		timein();

	if (prfstat & PRF_ON)
		prfintr( pc, cs );
	if ( USERMODE( cs ) ) {
		a = CPU_USER;
		u.u_utime++;
		if (u.u_prof.pr_scale)
			addupc( pc, &u.u_prof, 1 );
	} else {
		if ( pc == (caddr_t)&waitloc ) {
			if (syswait.iowait+syswait.swap+syswait.physio) {
				a = CPU_WAIT;
				if (syswait.iowait)
					sysinfo.wait[W_IO]++;
				if (syswait.swap)
					sysinfo.wait[W_SWAP]++;
				if (syswait.physio)
					sysinfo.wait[W_PIO]++;
			} else
				a = CPU_IDLE;
		} else
			a = CPU_KERNAL;
		u.u_stime++;
	}
	sysinfo.cpu[a]++;
	pp = u.u_procp;
	if (pp->p_stat==SRUN) {
		u.u_mem += (unsigned)pp->p_size;
		if (pp->p_textp) {
			a = pp->p_textp->x_ccount;
			if (a==0)
				a++;
			u.u_mem += pp->p_textp->x_size/a;
		}
	}
	if (pp->p_cpu < 80)
		pp->p_cpu++;
	lbolt++;	/* time in ticks */
	if (lticks <= 0) {
	    /*
		if ( inclock > 1 )
		{
			--inclock;
			return;
		}
	    */
		lticks += hz;
		time++;
#ifdef NIBMAT
		if (--chsecs <= 0) {
		    cursecs = getsecs();
		    tempsecs = cursecs - cmossecs;
		    time += (time_t) tempsecs;
		    if (clkprint)
			printf ("+%d+", tempsecs);
		    cmossecs = cursecs;
		    chsecs = 60;
		}
#endif NIBMAT
		runrun++;
		rqlen = 0;
		sqlen = 0;
		for(pp = &proc[0]; pp < (struct proc *)v.ve_proc; pp++)
		if (pp->p_stat) {
			if (pp->p_time != 127)
				pp->p_time++;
			if (pp->p_clktim)
				if (--pp->p_clktim == 0)
					psignal(pp, SIGALRM);
			pp->p_cpu >>= 1;
			if (pp->p_pri >= (PUSER-NZERO)) {
				pp->p_pri = (pp->p_cpu>>1) + PUSER +
					pp->p_nice - NZERO;
			}
			if (pp->p_stat == SRUN)
				if (pp->p_flag & SLOAD)
					rqlen++;
				else
					sqlen++;
		}
		if (rqlen) {
			sysinfo.runque += rqlen;
			sysinfo.runocc++;
		}
		if (sqlen) {
			sysinfo.swpque += sqlen;
			sysinfo.swpocc++;
		}
		if (runin!=0) {
			runin = 0;
			setrun(&proc[0]);
		}
	}
	/* 
	 * track lost ticks
	 * should also track lost ticks discovered on a '' delta in RT clock
	 */
	if (lticks > oticks)
		oticks += hz;
	lostticks = oticks - lticks;
	/*
	** now we can enable other kinds of interrupts
	*/
	--inclock;
}

/*
 * timeout is called to arrange that fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout structure.
 * The time in each structure entry is the number of HZ's more
 * than the previous entry. In this way, decrementing the
 * first entry has the effect of updating all entries.
 *
 * The panic is there because there is nothing
 * intelligent to be done if an entry won't fit.
 */
timeout(fun, arg, tim)
int (*fun)();
caddr_t arg;
{
	register struct callo *p1, *p2;
	register int t;
	int s;

	t = tim;
	p1 = &callout[0];
	s = spl7();
	if (callout[v.v_call-2].c_func)
		panic("Timeout table overflow");
	while(p1->c_func != 0 && p1->c_time <= t) {
		t -= p1->c_time;
		p1++;
	}
	p1->c_time -= t;
	p2 = p1;
	while(p2->c_func != 0)
		p2++;
	while(p2 >= p1) {
		(p2+1)->c_time = p2->c_time;
		(p2+1)->c_func = p2->c_func;
		(p2+1)->c_arg = p2->c_arg;
		p2--;
	}
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	splx(s);
}

timein()
{
	register struct callo *p1, *p2;
	register s;

	if (callout[0].c_func == NULL)
		return;
/*	s = spl7(); /* */
	asm("	cli	");
	if (callout[0].c_time <= 0) {
		p1 = &callout[0];
		while(p1->c_func != 0 && p1->c_time <= 0) {
/*			splx(s); /* */
			asm("	sti	");
			(*p1->c_func)(p1->c_arg);
/*			s = spl7(); /* */
			asm("	cli	");
			p1++;
		}
		p2 = &callout[0];
		while(p2->c_func = p1->c_func) {
			p2->c_time = p1->c_time;
			p2->c_arg = p1->c_arg;
			p1++;
			p2++;
		}
	}
/*	splx(s); /* */
	asm("	sti	");
}

#define	PDELAY	(PZERO-1)
delay(ticks)
{
	extern wakeup();

	if (ticks<=0)
		return;
	timeout(wakeup, (caddr_t)u.u_procp+1, ticks);
	sleep((caddr_t)u.u_procp+1, PDELAY);
}

#ifdef NIBMAT
/* 
 * getsecs - return seconds field of hardware real-time clock
 */

/* time fields are kept in BCD format */
#define	bcdtoi(x) (((x) & 0xf) + (((x) >> 4) * 10))

static int
getsecs() {
    int x, ret;

    outb (0x70, 0x0);
    ret = inb (0x71);
    return bcdtoi (ret);
}
#endif NIBMAT
