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
extern int strcmp();

struct group *
getgrnam(name)
register char *name;
{
	register struct group *p;

	setgrent();
	while((p = getgrent()) && strcmp(p->gr_name, name))
		;
	endgrent();
	return(p);
}
