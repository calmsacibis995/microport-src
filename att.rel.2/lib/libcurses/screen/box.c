/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
# include	"curses.ext"

/*
 *	This routine draws a box around the given window with "vert"
 * as the vertical delimiting char, and "hor", as the horizontal one.
 *
 */

/* Defaults - might someday be terminal dependent using graphics chars */
#define DEFVERT '|'
#define DEFHOR  '-'

box(win, vert, hor)
register WINDOW	*win;
chtype	vert, hor;
{
	register int	i;
	register int	endy, endx;
	register chtype	*fp, *lp;

	if (vert == 0)
		vert = DEFVERT;
	if (hor == 0)
		hor = DEFHOR;
	endx = win->_maxx;
	endy = win->_maxy -  1;
	fp = win->_y[0];
	lp = win->_y[endy];
	for (i = 0; i < endx; i++)
		fp[i] = lp[i] = hor;
	endx--;
	for (i = 0; i <= endy; i++)
		win->_y[i][0] = (win->_y[i][endx] = vert);
	touchwin(win);
}
