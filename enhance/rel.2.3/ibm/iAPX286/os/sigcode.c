static char *uportid = "@(#)sigcode.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)sigcode.c	1.8 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/reg.h"
#include "sys/psl.h"

/*
**	Return to interrupted user routine after user interrupt handler has
**	completed processing.
**
**	We get here by the following route:
**	1. Sendsig was called to call a user interrupt handler.  It set up
**		A. floating point state (possibly null),
**		B. cs:ip of interrupted user routine,
**		C. sp of interrupted user routine,
**		D. pusha of interrupted user routine's registers with the
**			sp word reset to point to the saved floating point
**			state if it was present or 0 if it wasn't using
**			floating point,
**		E. ds of interrupted user routine,
**		F. es of interrupted user routine,
**		G. flags of interrupted user routine, and
**		H. duplicate of cs:ip of interrupted user routine,
**	   on the user stack and then called the user interrupt routine with
**	   a return address on its stack that will return to address 0 in the
**	   user's first code segment.  (This is set up in crt0.s to clear the
**	   args to the interrupt routine and call sigcode through a call gate.)
**	2. The user interrup handler did its thing and returned to the
**	   code at the start of crt0.s.
**	3. The code in crt0.s removed the user interrupt routine args from
**	   the stack and did an lcall through a call gate to get here.  Note
**	   that the last two args to sigcode were set up by the transition to
**	   kernel mode by the call gate, not by sendsig.
**
**	Sigcode will do all the work of restoring the floating point state,
**	verify as well as possible that the user hasn't clobberred his stack,
**	and call _sigcode(ml/trap.s) to restore the registers, flags, and
**	return to the user.  If sigcode or _sigcode detect that the stack
**	is not reasonable, sigcode will try to drop a core image and terminate
**	the process with a segmentation violation.
*/
sigcode(ip, cs, flags, es, ds, di, si, bp, fpoffset, bx, dx, cx, ax,
	ip2, cs2, sp, osp, oss)
{
	register int	err;	/* error detected flag */
	extern struct proc	*fp_proc;
	int		r[SS + 1];	/* register work area for core() */

	/* Clear error flag. */
	err = 0;

	/* Overwrite the user's stack pointer in the stack frame with the value
		it had when the user was interrupted. */
	osp = sp;

	/* Set up the end of the stack frame with the flags to be returned to
		the user so an "iret" will return to the user at the desired
		cs:ip with the right flags and ss:sp. */
	asm("	cld");
	sp = flags;

	/* Restore floating point state to the way it was when sendsig
		was called. */
	if(fp_proc == u.u_procp)
		fp_proc = NULL;
	if(fpoffset)

		/* Copy floating point state back into the u area. */
		if(copyin(fpoffset, oss, u.u_fpstate.edata,
			sizeof(u.u_fpstate.edata)))

			/* User must have messed up his own stack. */
			err = 1;
		else
			u.u_fpvalid = 1;
	else

		/* User wasn't using floating point. */
		u.u_fpvalid = 0;

	/* Verify that both copies of "cs:ip" match and are valid user level
		code addresses and that flags haven't been altered. */
	if(err == 0 && ip == ip2 && cs == cs2 && useracc(ip, cs, 1, 0) == 0 &&
		(flags & (PS_IOPL | PS_NT)) == 0 && (flags & PS_IE) == PS_IE
		&& useracc(ip, cs, 1, 1))

		/* Go to assembler routine to restore the user's registers and
			return to the user. */
		_sigcode(&es);

	/* User messed up his stack.  Make a register save area suitable for
		core(), try to drop a core, and exit. */
	r[ES] = es;
	r[DS] = ds;
	r[DI] = di;
	r[SI] = si;
	r[BP] = bp;
	r[BX] = bx;
	r[DX] = dx;
	r[CX] = cx;
	r[AX] = ax;
	r[IP] = ip;
	r[CS] = cs;
	r[FLGS] = flags;
	r[SP] = osp;
	r[SS] = oss;
	r[SCIP] = ip2;
	r[SCCS] = cs2;
	r[ARG0] = 'IR';
	u.u_ar0 = r;
	exit(core() ? (0200 + SIGSEGV) : SIGSEGV);
}
