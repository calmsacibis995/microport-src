/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
# include	"curses.ext"

/*
 *	This routine erases everything on the _window.
 *
 */
werase(win)
reg WINDOW	*win; {

	reg int		y;
	reg chtype	*sp, *end, *start, *maxx;
	reg int		minx;

# ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "WERASE(%0.2lo), _maxx %d\n", win, win->_maxx);
#else
	if(outf) fprintf(outf, "WERASE(%0.2o), _maxx %d\n", win, win->_maxx);
#endif
# endif
	for (y = 0; y < win->_maxy; y++) {
		minx = _NOCHANGE;
		maxx = NULL;
		start = win->_y[y];
		end = &start[win->_maxx];
		for (sp = start; sp < end; sp++) {
#ifdef DEBUG
			if (y == 23) if(outf) fprintf(outf,
				"sp %x, *sp %c %o\n", sp, *sp, *sp);
#endif
			if (*sp != ' ') {
				maxx = sp;
				if (minx == _NOCHANGE)
					minx = sp - start;
				*sp = ' ';
			}
		}
		if (minx != _NOCHANGE) {
			if (win->_firstch[y] > minx
			     || win->_firstch[y] == _NOCHANGE)
				win->_firstch[y] = minx;
			if (win->_lastch[y] < maxx - win->_y[y])
				win->_lastch[y] = maxx - win->_y[y];
		}
# ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "WERASE: minx %d maxx %d _firstch[%d] %d, start %lx, end %lx\n",
#else
	if(outf) fprintf(outf, "WERASE: minx %d maxx %d _firstch[%d] %d, start %x, end %x\n",
#endif
		minx, maxx ? maxx-start : NULL, y, win->_firstch[y], start, end);
# endif
	}
	win->_curx = win->_cury = 0;
}
