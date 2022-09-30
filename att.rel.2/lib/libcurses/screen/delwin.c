/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
# include	"curses.ext"

/*
 *	This routine deletes a _window and releases it back to the system.
 *
 */
delwin(win)
reg WINDOW	*win; {

	reg int	i;

	if (!(win->_flags & _SUBWIN))
		for (i = 0; i < win->_maxy && win->_y[i]; i++)
			cfree((char *) win->_y[i]);
	cfree((char *) win->_y);
	cfree((char *) win);
}
