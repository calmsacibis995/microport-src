/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)lddef.h	1.3 - 85/08/09 */
#ifndef LDLIST

struct ldlist {
	LDFILE		ld_item;
	struct ldlist	*ld_next;
};

#define	LDLIST	struct ldlist
#define	LDLSZ	sizeof(LDLIST)

#endif
