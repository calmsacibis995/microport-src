/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>

extern char *strcpy();
static char res[L_ctermid];

char *
ctermid(s)
register char *s;
{
	return (strcpy(s != NULL ? s : res, "/dev/tty"));
}