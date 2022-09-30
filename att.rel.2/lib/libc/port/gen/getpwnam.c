/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*LINTLIBRARY*/
#include <pwd.h>
#include <stdio.h>	/* for definition of L_cuserid */

extern struct passwd *getpwent();
extern void setpwent(), endpwent();
extern int strncmp();

struct passwd *
getpwnam(name)
char	*name;
{
	register struct passwd *p;

	setpwent();
	while ((p = getpwent()) && strncmp(name, p->pw_name, L_cuserid - 1))
		;
	endpwent();
	return (p);
}
