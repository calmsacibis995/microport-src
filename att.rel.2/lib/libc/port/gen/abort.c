/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
/*LINTLIBRARY*/
/*
 *	abort() - terminate current process with dump via SIGIOT
 */

#include <signal.h>

extern int kill(), getpid();
static pass = 0;		/* counts how many times abort has been called*/

int
abort()
{
	/* increment first to avoid any hassle with interupts */
	if (++pass == 1)  {
		_cleanup();
	}
	return(kill(getpid(), SIGIOT));
}
