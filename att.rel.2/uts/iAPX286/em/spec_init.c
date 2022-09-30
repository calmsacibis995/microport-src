/* @(#)spec_init.c	1.1 - 85/09/06 */

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
spec_init(cwoff,cwon)
int cwoff,cwon;                                /* initial control word */
{
	unsigned int	segment;		/* seg to load into ds       */

	segment = ((long)lstokv(EMUL_SEL)) >> 16; /* get emulator ds   */
	emul_sinit(segment,cwoff,cwon);         /* call emulator     */

	return;                                 /* return to user    */
}
