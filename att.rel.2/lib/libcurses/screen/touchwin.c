/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
# include	"curses.ext"

/*
 * make it look like the whole window has been changed.
 *
 */
touchwin(win)
reg WINDOW	*win;
{
	reg int		y, maxy, maxx;

#ifdef DEBUG
#if iAPX286 & LARGE_M
	if (outf) fprintf(outf, "touchwin(%lx)\n", win);
#else
	if (outf) fprintf(outf, "touchwin(%x)\n", win);
#endif
#endif
	maxy = win->_maxy;
	maxx = win->_maxx - 1;
	for (y = 0; y < maxy; y++) {
		win->_firstch[y] = 0;
		win->_lastch[y] = maxx;
	}
}
