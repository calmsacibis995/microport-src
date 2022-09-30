/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)lock.c	1.4 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/proc.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/text.h"
#include "sys/lock.h"

lock()
{
	struct a {
		int oper;
	};

	if (!suser())
		return;
	switch(((struct a *)u.u_ap)->oper) {
	case TXTLOCK:
		if ((u.u_lock&(PROCLOCK|TXTLOCK)) || textlock() == 0)
			goto bad;
		break;
	case PROCLOCK:
		if (u.u_lock&(PROCLOCK|TXTLOCK))
			goto bad;
		textlock();
		proclock();
		break;
	case DATLOCK:
		if (u.u_lock&(PROCLOCK|DATLOCK))
			goto bad;
		u.u_lock |= DATLOCK;	/* NOP for VAX */
		break;
	case UNLOCK:
		if (punlock() == 0)
			goto bad;
		break;

	default:
bad:
		u.u_error = EINVAL;
	}
}

textlock()
{
	struct text *xp;

	if ((xp=u.u_procp->p_textp) == NULL)
		return(0);
	u.u_lock |= TXTLOCK;
	return(1);
}
		
tunlock()
{
	struct text *xp;

	if ((xp=u.u_procp->p_textp) == NULL || (u.u_lock&TXTLOCK) == 0)
		return(0);
	u.u_lock &= ~TXTLOCK;
	return(1);
}

proclock()
{
	u.u_procp->p_flag |= SSYS;
	u.u_lock |= PROCLOCK;
}

punlock()
{
	if ((u.u_lock&(PROCLOCK|TXTLOCK|DATLOCK)) == 0)
		return(0);
	u.u_procp->p_flag &= ~SSYS;
	u.u_lock &= ~PROCLOCK;
	u.u_lock &= ~DATLOCK;
	tunlock();
	return(1);
}
