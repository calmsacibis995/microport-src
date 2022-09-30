/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

struct screen *
set_term(new)
struct screen *new;
{
	register struct screen *rv = SP;

#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "setterm: old %lx, new %lx\n", rv, new);
#else
	if(outf) fprintf(outf, "setterm: old %x, new %x\n", rv, new);
#endif
#endif

#ifndef		NONSTANDARD
	SP = new;
#endif		NONSTANDARD

	cur_term = SP->tcap;
	LINES = lines;
	COLS = columns;
	stdscr = SP->std_scr;
	curscr = SP->cur_scr;
	return rv;
}
