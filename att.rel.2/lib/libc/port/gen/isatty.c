/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*LINTLIBRARY*/
/*
 * Returns 1 iff file is a tty
 */
#include <sys/termio.h>

extern int ioctl();

int
isatty(f)
int	f;
{
	struct termio tty;

	if(ioctl(f, TCGETA, &tty) < 0)
		return(0);
	return(1);
}
