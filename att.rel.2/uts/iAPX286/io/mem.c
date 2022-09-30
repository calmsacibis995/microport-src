/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)mem.c	1.10 */
/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/NULL
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/mmu.h"
#include "sys/seg.h"

extern	physmem;

int	*nofault;	/* pointer to nofault save area	*/

mmread( dev )
{
	register unsigned	n;
	paddr_t			physaddr();


	switch ( dev )
	{
		case 0:
		{
			struct	seg_desc	*mmp;
			unsigned long		a;
			char			*addr;

			if ( btoc( u.u_offset ) > physmem )
			{
				u.u_error = ENXIO;
				break;
			}
			mmp = gdt + MM_SEL;
			a = u.u_offset;
			mmp->sd_hibase = lobyte( hiword( a ) );
			mmp->sd_lowbase = loword( a );
			mmp->sd_limit = u.u_count - 1;
			mmp->sd_access = ACC_KDATA;
			addr = (char *)gstokv( MM_SEL );
			if ( copyout( addr, u.u_base, u.u_count ) )
			{
				u.u_error = ENXIO;
			}
			break;
		}

		case 1:
		{
			label_t	fltsav;		/* nofault save area	*/
			int	*oldnofault;	/* old nofault value	*/

			/*
			** the user supplied us with both addresses,
			** so this could fault. So, we implement nofault
			** so it can't hurt us
			*/
			if ( setjmp( fltsav ) )
			{
				nofault = oldnofault;
			}
			else
			{
				oldnofault = nofault;
				nofault = fltsav;
				if ( copyout( u.u_offset, u.u_base,u.u_count ) )
				{
					u.u_error = ENXIO;
				}
			}
			nofault = oldnofault;
			break;
		}

		case 2:
			return;
	}
	u.u_offset += u.u_count;
	u.u_base += u.u_count;
	u.u_count = 0;
}


mmwrite( dev )
{
	register unsigned	n;
	register		c;
	paddr_t			physaddr();


	switch ( dev )
	{
		case 0:
		{
			struct	seg_desc	*mmp;
			unsigned long		a;
			char			*addr;

			if ( btoc( u.u_offset ) > physmem )
			{
				u.u_error = ENXIO;
				break;
			}
			mmp = gdt + MM_SEL;
			a = u.u_offset;
			mmp->sd_hibase = lobyte( hiword( a ) );
			mmp->sd_lowbase = loword( a );
			mmp->sd_limit = u.u_count - 1;
			mmp->sd_access = ACC_KDATA;
			addr = (char *)gstokv( MM_SEL );
			if ( copyin( u.u_base, addr, u.u_count ) )
			{
				u.u_error = ENXIO;
			}
			break;
		}

		case 1:
		{
			label_t	fltsav;		/* nofault save area	*/
			int	*oldnofault;	/* old nofault value	*/

			/*
			** the user supplied us with both addresses,
			** so this could fault. So, we implement nofault
			** so it can't hurt us
			*/
			if ( setjmp( fltsav ) )
			{
				nofault = oldnofault;
			}
			else
			{
				oldnofault = nofault;
				nofault = fltsav;
				if ( copyin( u.u_base, u.u_offset, u.u_count ) )
				{
					u.u_error = ENXIO;
				}
			}
			nofault = oldnofault;
		}
			/* fall through */
		case 2:
			break;
	}
	u.u_offset += u.u_count;
	u.u_base += u.u_count;
	u.u_count = 0;
}
