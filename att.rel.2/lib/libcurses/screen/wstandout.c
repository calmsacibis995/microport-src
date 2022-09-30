/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

# include	"curses.ext"

/*
 * enter standout mode
 */
wstandout(win)
register WINDOW	*win;
{
#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "WSTANDOUT(%lx)\n", win);
#else
	if(outf) fprintf(outf, "WSTANDOUT(%x)\n", win);
#endif
#endif

	win->_attrs |= A_STANDOUT;
	return 1;
}
