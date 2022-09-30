/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

/*
 * TRUE => OK to use insert/delete line.
 */
idlok(win,bf)
WINDOW *win;
int bf;
{
	win->_use_idl = bf;
}
