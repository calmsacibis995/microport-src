/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)userio.c	1.7 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/seg.h"
#include "sys/mmu.h"

/*
**  fuptr - Fetch pointer (casting to long pointer) from user space.
*/
long
fuptr(addr, sel)
long addr;
int sel;
{
	register unsigned low, high;

	if (u.u_model & U_MOD_SSTACK) {
		low = fuword(addr);
		high = fuword(addr+2);
	} else {
		low = fuword(addr);
		if (low)
			high = (sel<<3)|LDT_TI|USER_RPL;
		else
			high = 0;
	}
	return(((unsigned long)high << 16) | low);
}

/*
**  suptr - Store pointer (casting to correct model) into user space.
*/
suptr(addr, value)
long addr;
long value;
{
	if (!(u.u_model & U_MOD_SSTACK))
		return(suword(addr, (ushort)value));
	else if (suword(addr, (ushort)value) < 0)
		return(-1);
	else
		return(suword(addr+2, value >> 16));
}

/*
 * find_stack - used by exece and passed to fuptr or in addr of suptr.
 */
find_stack()
{
	register struct seg_desc *sp;	/* ptr to LDT entry */
	register struct seg_desc *ep;

	if (u.u_model & U_MOD_SSTACK)
		return(STACK_SEL);
	else if(u.u_model & U_MOD_MTEXT) {
		ep = (struct seg_desc *)
		    ((char *)u.u_ldtadv + ctob((long)u.u_lsize));
		sp = u.u_ldtadv + CODE1_SEL + 1;
		while (sp < ep && (sp->sd_access & (DSC_SEG|SD_CODE))
		    == (DSC_SEG|SD_CODE))
			sp++;
		return(sp - u.u_ldtadv);
	} else
		return(CODE1_SEL + 1);
}
