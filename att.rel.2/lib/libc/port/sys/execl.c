/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*
 *	execl(name, arg0, arg1, ..., argn, 0)
 *	environment automatically passed.
 */

execl(name, args)
char *name, *args;
{
	extern char **environ;

	return (execve(name, &args, environ));
}
