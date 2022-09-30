/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

/*
 * TRUE => OK to scroll screen up when you run off the bottom.
 */
scrollok(win,bf)
WINDOW *win;
int bf;
{
	/* Should consider using scroll/page mode of some terminals. */
	win->_scroll = bf;
}
