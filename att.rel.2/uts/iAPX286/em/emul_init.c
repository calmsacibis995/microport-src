/* @(#)emul_init.c	1.1 - 85/09/06 */

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

emul_init(cwoff,cwon)
int cwoff,cwon;                         /* initial value of control word */
{
	unsigned long	phys;		/* physical address	*/
	unsigned int	lim;		/* segment limit	*/
	struct seg_desc	*uldtp,		/* upage seg ptr	*/
			*eldtp;		/* emul seg ptr		*/

	uldtp = &u.u_ldtadv[ UPAGE_SEL ];
	lim = uldtp->sd_limit;
	phys = ( (unsigned long)uldtp->sd_hibase << 16 ) | uldtp->sd_lowbase;

	phys += (unsigned int)&u.u_fpstate;
	lim -= (unsigned int)&u.u_fpstate;

	eldtp = &u.u_ldtadv[ EMUL_SEL ];
	eldtp->sd_limit = lim;
	eldtp->sd_lowbase = loword( phys );
	eldtp->sd_hibase = lobyte( hiword( phys ) );
	eldtp->sd_access = ACC_KDATA;
	spec_init(cwoff,cwon);                        /* calls init in emul */
	return;
}
