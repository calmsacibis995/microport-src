/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

# include	"curses.ext"

/*
 * exit standout mode
 */
wstandend(win)
register WINDOW	*win;
{
#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "WSTANDEND(%x)\n", win);
#else
	if(outf) fprintf(outf, "WSTANDEND(%x)\n", win);
#endif
#endif

	win->_attrs = 0;
	return 1;
}
