/* @(#)emul_entry.c	1.1 - 85/09/06 */

/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/systm.h"
#include	"sys/dir.h"
#include	"sys/signal.h"
#include	"sys/user.h"
#include	"sys/errno.h"
#include	"sys/proc.h"
#include	"sys/reg.h"
#include	"sys/psl.h"
#include	"sys/trap.h"
#include	"sys/seg.h"
#include	"sys/sysinfo.h"
#include	"sys/mmu.h"
#include	"sys/fp.h"

/*---------------------------------------------------------------------------*/
	unsigned int	segment;		/* seg to load into ds       */
		 int	*regpoint;		/* pointer to user register  */
/*---------------------------------------------------------------------------*/
emul_entry(addr)
char	*addr;					/* addr points to the 287    */
						/* instruction               */
{

	segment = ((long)lstokv(EMUL_SEL)) >> 16;	/* get emulator ds   */
	regpoint= u.u_ar0;				/* get pointer to    */

	emul(segment,regpoint);				/* call emulator     */

	return;						/* return to user    */
}

/*---------------------------------------------------------------------------*/
emul_signal_segv()
{
	psignal ( u.u_procp , SIGSEGV );
}
/*---------------------------------------------------------------------------*/

emul_signal_ill()
{
	psignal ( u.u_procp , SIGILL );
}
/*---------------------------------------------------------------------------*/

emul_signal_fpe()
{
	psignal ( u.u_procp , SIGFPE );
}
/*---------------------------------------------------------------------------*/
