/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

# include	"curses.ext"

/*
 * Turn off selected attributes.
 */
wattroff(win, attrs)
register WINDOW	*win;
int attrs;
{
#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "WATTRON(%lx, %o)\n", win, attrs);
#else
	if(outf) fprintf(outf, "WATTRON(%x, %o)\n", win, attrs);
#endif
#endif

	win->_attrs &= ~attrs;
	return 1;
}
