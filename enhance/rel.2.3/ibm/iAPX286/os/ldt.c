static char *uportid = "@(#)ldt.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)ldt.c	1.14 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/seg.h"
#include "sys/mmu.h"
#include "sys/fp.h"

/*
 * Adjust the physical memory pointers for a process in the GDT & its LDT.
 */
adjustldt(p, new)
struct proc *p;
int new;
{
	struct seg_desc *sdp, *ldt, *ep;
	long a;
	unsigned oldl;

	/*
	 * first we get access to the pslot in the GDT.
	 * we update addresses and turn on present bits 
	 * in case we're swapping in.
	 * we also fix the LDT size to save sbreak pain.
	 */
	sdp = &pslot[p-proc].pa_tss_d;

	a = ctob((long)new + USIZE/2);		/* TSS descriptor */
	sdp->sd_hibase = lobyte(hiword(a));
	sdp->sd_lowbase = loword(a);
	/*
	 * We force the access to an available task because we need
	 * to turn off the busy bit for expand and it doesn't hurt
	 * us in any other case
	 */
	sdp->sd_access = DSC_PRESENT | CD_AVAIL_TSK;

	sdp++;
	sdp->sd_hibase = lobyte(hiword(a));	/* TSS alias descriptor */
	sdp->sd_lowbase = loword(a);
	sdp->sd_access |= DSC_PRESENT;

	sdp++;
	a = ctob((long)new + USIZE);		/* LDT descriptor */
	sdp->sd_hibase = lobyte(hiword(a));
	sdp->sd_lowbase = loword(a);
	oldl = btoc(sdp->sd_limit);
	sdp->sd_limit = ctob((long)p->p_lsize) - 1;
	sdp->sd_access |= DSC_PRESENT;

	sdp++;
	sdp->sd_hibase = lobyte(hiword(a));	/* LDT alias descriptor */
	sdp->sd_lowbase = loword(a);
	sdp->sd_limit = ctob((long)p->p_lsize) - 1;
	sdp->sd_access |= DSC_PRESENT;

	/*
	 * now we get at the process's LDT.
	 * p_smbeg is a selector index
	 * so it performs a necessary limit function.
	 * note that we just fix addresses.
	 */
	ldt = (struct seg_desc *)gstokv(sdp - gdt);
	if (p->p_smbeg)
		ep = ldt + p->p_smbeg;
	else
		ep = ldt + (ctob((long)oldl) >> 3);

	/*
	** if we are using the 287 emulator, and this process
	** was doing floating point, update the 287 EMUL selector
	*/
	if (fp_kind == FP_SW) {
		ldt += EMUL_SEL;
		if (ldt->sd_access & DSC_PRESENT) {
			a = ctob((long)new) + (unsigned int)&u.u_fpstate;
			ldt->sd_hibase = lobyte(hiword(a));
			ldt->sd_lowbase = loword(a);
		}
		ldt++;
	}
	else
		ldt += UPAGE_SEL;

	a = ctob((long)new);			/* upage */
	ldt->sd_hibase = lobyte(hiword(a));
	ldt->sd_lowbase = loword(a);

	ldt++;					/* stack */
	if (p->p_ssize) {	/* only fix if seperate stack */
		a = ctob((long)new + p->p_size - stoc(1));
		ldt->sd_hibase = lobyte(hiword(a));
		ldt->sd_lowbase = loword(a);
	}

	ldt++;					/* code */
	a = ctob((long)new + USIZE + p->p_lsize);
	while (ldt < ep
	    && (ldt->sd_access & (ACC_UCODE)) == (ACC_UCODE)) {
		if (!p->p_textp) {	/* only fix if not shared text */
			ldt->sd_hibase = lobyte(hiword(a));
			ldt->sd_lowbase = loword(a);
			a += ldt->sd_limit + 1L;
		}
		ldt++;
	}

	if (p->p_ssize) {			/* data */
		/*
		 * since the stack is separate we need to check for
		 * multiple data.  we mask with code to make sure it's
		 * really data.
		 */
		while (ldt < ep
		    && (ldt->sd_access & (ACC_UCODE)) == (ACC_UDATA)) {
			ldt->sd_hibase = lobyte(hiword(a));
			ldt->sd_lowbase = loword(a);
			a += ldt->sd_limit + 1L;
			ldt++;
		}
	} else {
		/* combined stack/data can only be one entry */
		ldt->sd_hibase = lobyte(hiword(a));
		ldt->sd_lowbase = loword(a);
	}
}

/*
 * Adjust the physical memory pointers for shared text in the LDT.
 */
adjusttext(p, base)
struct proc *p;
int base;
{
	struct seg_desc *ldt, *ep;
	long a;

	if (!p->p_textp)
		return;
	/*
	 * this is a lot like adjustldt except we only need to fix
	 * the text segments.  it is separate because we usually
	 * do the adjustments at different times.
	 */
	ldt = (struct seg_desc *)gstokv(&pslot[p-proc].pa_ldt_ad - gdt);
	if (p->p_smbeg)
		ep = ldt + p->p_smbeg;
	else
		ep = ldt + (ctob((long)p->p_lsize) >> 3);
	ldt += CODE1_SEL;
	a = ctob((long)base);
	while (ldt < ep
	    && (ldt->sd_access & (DSC_SEG|SD_CODE)) == (DSC_SEG|SD_CODE)) {
		ldt->sd_hibase = lobyte(hiword(a));
		ldt->sd_lowbase = loword(a);
		a += ldt->sd_limit + 1L;
		ldt++;
	}
}

/*
 * turn off the present bits in the GDT pslot for this process so
 * no one will use the process's descriptors.
 */
zapprocslots(p)
struct proc *p;
{
	register int n;

	n = p - proc;
	pslot[n].pa_tss_d.sd_access &= ~DSC_PRESENT;
	pslot[n].pa_tss_ad.sd_access &= ~DSC_PRESENT;
	pslot[n].pa_ldt_d.sd_access &= ~DSC_PRESENT;
	pslot[n].pa_ldt_ad.sd_access &= ~DSC_PRESENT;
}

/*
** turn on all the present bits in the GDT pslot for this
** process, and reset the busy bit in the tss for this process
*/
unzapslots( p )
struct proc	*p;
{
	register int n;

	n = p - proc;
	pslot[ n ].pa_tss_d.sd_access = DSC_PRESENT | CD_AVAIL_TSK;
	pslot[ n ].pa_tss_ad.sd_access |= DSC_PRESENT;
	pslot[ n ].pa_ldt_d.sd_access |= DSC_PRESENT;
	pslot[ n ].pa_ldt_ad.sd_access |= DSC_PRESENT;
}
