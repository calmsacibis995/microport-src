/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

clearok(win,bf)	
WINDOW *win;
int bf;
{
#ifdef DEBUG
	if (outf) {
		if (win == stdscr)
			fprintf(outf, "it's stdscr: ");
		if (win == curscr)
			fprintf(outf, "it's curscr: ");
	}
#if iAPX286 & LARGE_M
	if (outf) fprintf(outf, "clearok(%lx, %d)\n", win, bf);
#else
	if (outf) fprintf(outf, "clearok(%x, %d)\n", win, bf);
#endif
#endif
	if (win==curscr)
		SP->doclear = 1;
	else
		win->_clear = bf;
}
