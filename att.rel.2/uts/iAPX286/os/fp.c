/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)fp.c	1.13 */
/*
** routines that deal with floating point
*/

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

int	finitstate;

/*
** fpinit
**	initialize the floating point unit for this user
*/
fpinit()
{

	switch ( fp_kind )
	{
		case FP_HW:

			asm( "	finit" );
			/*
			** must allow invalid operation, zero divide, and
			** overflow interrupt conditions and change to use
			** long real precision
			*/
			asm( "	mov	$<s>finitstate,%ax" );
			asm( "	mov	%ax,%ds" );
			asm( "	fstcw	finitstate" );

			finitstate &= ~( FPINV | FPZDIV | FPOVR | FPPC);
			finitstate |= FPSIG53;

			asm( "	fldcw	finitstate" );
			break;

		case FP_SW:
			emul_init();
			break;

		default:
			break;
	}
}


/*
** fpsave
**	save the floating point state into fp_proc's user
**	structure.
**	check flag:
**		FPCHECK = check for validity of doing a save
**		FPNOCHECK = don't check for validity
*/
fpsave( check )
{
	struct user	*up;
	struct seg_desc	*sdp;
	long		a;

	if ( check )	/* check == FPCHECK */
	{
		if ( ( fp_proc == 0 ) || ( fp_proc != u.u_procp ) )
			return;
	}

	/*
	** build a descriptor to allow us to look at the
	** fp_proc's udot and then pass the address of the
	** save area to savefp()
	*/
	sdp = gdt + FPSEL;
	a = ctob( (long)fp_proc->p_addr );
	sdp->sd_hibase = lobyte( hiword( a ) );
	sdp->sd_lowbase = loword( a );
	sdp->sd_limit = ctob( USIZE ) - 1;
	sdp->sd_access = ACC_KDATA;
	up = (struct user *)gstokv( FPSEL );

	switch ( fp_kind )
	{
		case FP_HW:	/* 80287 present */
			savefp( &( up->u_fpstate ) );	/* save state	*/
			/* FALL THROUGH */

		case FP_SW:
			/*
			** say that the saved state is valid
			*/
			up->u_fpvalid++;
			fpclex();		/* clear the exceptions	*/
			break;

		default:
			break;
	}

	/*
	** Now, signify that nobody owns the fp unit
	*/
	fp_proc = 0;
}


/*
** fprestore
**	restore the floating point state from the current
**	user structure
*/
fprestore()
{
	switch ( fp_kind )
	{
		case FP_HW:
			restorefp( &( u.u_fpstate ) );
			/* FALL THROUGH */

		case FP_SW:
			u.u_fpvalid = 0;
			break;

		default:
			break;
	}
}

/*
** fpclex
**	clear the floating point exceptions on the 80287
*/
fpclex()
{
	switch ( fp_kind )
	{
		case FP_HW:
			asm( "	fclex" );
			/* FALL THROUGH */

		case FP_SW:
			break;
		
		default:
			break;
	}
}
