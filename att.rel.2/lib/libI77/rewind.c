/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286.c @(#)rewind.c	1.3 85/09/16 */
/*	@(#)rewind.c	1.2	*/
/*	3.0 SID #	1.2	*/
#include "fio.h"
f_rew(a) alist *a;
{
	unit *b;
	if(a->aunit>=MXUNIT || a->aunit<0) err(a->aerr,101,"rewind");
	b = &units[a->aunit];
	if(b->ufd == NULL) return(0);
	if(!b->useek) err(a->aerr,106,"rewind")
	if(b->uwrt)
	{	(void) nowreading(b);
		(void) t_runc(b);
	}
	rewind(b->ufd);
	b->uend=0;
	return(0);
}
