/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*LINTLIBRARY*/
#include <grp.h>

extern struct group *getgrent();
extern void setgrent(), endgrent();

struct group *
getgrgid(gid)
register int gid;
{
	register struct group *p;

	setgrent();
	while((p = getgrent()) && p->gr_gid != gid)
		;
	endgrent();
	return(p);
}
