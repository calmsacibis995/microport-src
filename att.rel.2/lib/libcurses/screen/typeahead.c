/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

/*
 * Set the file descriptor for typeahead checks to fd.  fd can be -1
 * to disable the checking.
 */
typeahead(fd)
int fd;
{
	SP->check_fd = fd;
}
