/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)prof.c	1.6 */
/*
 *	VAX/UNIX Operating System Profiler
 *
 *	Sorted Kernel text addresses are written into driver.  At each
 *	clock interrupt a binary search locates the counter for the
 *	interval containing the captured PC and increments it.
 *	The last counter is used to hold the User mode counts.
 */

#include	"sys/signal.h"
#include	"sys/errno.h"
#include	"sys/mmu.h"
#include	"sys/param.h"
#include	"sys/psl.h"
#include	"sys/types.h"
#include	"sys/dir.h"
#include	"sys/user.h"
#include	"sys/buf.h"
#include	"sys/ioctl.h"

#define PRFMAX  2048		/* maximum number of text addresses */
# define PRF_ON    1		/* profiler collecting samples */
# define PRF_VAL   2		/* profiler contains valid text symbols */
# define BPL	   4		/* bytes per long */
# define BPP	   4		/* bytes per pointer (large model) */
# define L2BPL	   2		/* log2(BPL) */

#if iAPX286
/* prfstat is declared in space.h on the 286 so this driver can be included or
	omitted from the configuration and have clock.c find prfstat in the
	same segment at all times when the kernel is superoptimized. */
extern unsigned  prfstat;	/* state of profiler */
#else
unsigned  prfstat;		/* state of profiler */
#endif
unsigned  prfmax;		/* number of loaded text symbols */
unsigned long  prfctr[PRFMAX + 1];	/* counters for symbols; last used for User */
caddr_t   prfsym[PRFMAX];	/* text symbols */

prfread()
{
	unsigned  min();

	if((prfstat & PRF_VAL) == 0) {
		u.u_error = ENXIO;
		return;
	}
	iomove((caddr_t) prfsym, min(u.u_count, prfmax * BPP),
	    B_READ);
	iomove((caddr_t) prfctr, min(u.u_count, (prfmax + 1) * BPL),
	    B_READ);
}

prfwrite()
{
	unsigned  long	*ip;
	caddr_t		*pp;

	if(u.u_count > sizeof prfsym)
		u.u_error = ENOSPC;
	else if(u.u_count & (BPL - 1) || u.u_count < 3 * BPL)
		u.u_error = E2BIG;
	else if(prfstat & PRF_ON)
		u.u_error = EBUSY;
	if(u.u_error)
		return;
	for(ip = prfctr; ip != &prfctr[PRFMAX + 1]; )
		*ip++ = 0;
	prfmax = u.u_count >> L2BPL;
	iomove((caddr_t) prfsym, u.u_count, B_WRITE);
	for(pp = &prfsym[1]; pp != &prfsym[prfmax]; pp++)
		if(*pp < pp[-1]) {
			u.u_error = EINVAL;
			break;
		}
	if(u.u_error)
		prfstat = 0;
	else
		prfstat = PRF_VAL;
}

prfioctl(dev, cmd, arg, mode)
union ioctl_arg	arg;
{
	switch(cmd) {
	case 1:
		u.u_r.r_reg.r_val1 = prfstat;
		break;
	case 2:
		u.u_r.r_reg.r_val1 = prfmax;
		break;
	case 3:
		if(prfstat & PRF_VAL) {
			prfstat = PRF_VAL | arg.iarg & PRF_ON;
			break;
		}
	default:
		u.u_error = EINVAL;
	}
}

prfintr(pc, cs)
caddr_t	pc;
{
	register  int  h, l, m;

	if(USERMODE(cs))
		prfctr[prfmax]++;
	else {
		l = 0;
		h = prfmax;
		while((m = (l + h) / 2) != l)
			if(pc >= prfsym[m])
				l = m;
			else
				h = m;
		prfctr[m]++;
	}
}
