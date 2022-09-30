/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include "curses.ext"

/*
 * _line_alloc returns a pointer to a new line structure.
 */
struct line *
_line_alloc ()
{
	register struct line   *rv = SP->freelist;
	char *calloc();

#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "mem: _line_alloc (), prev SP->freelist %lx\n", SP->freelist);
#else
	if(outf) fprintf(outf, "mem: _line_alloc (), prev SP->freelist %x\n", SP->freelist);
#endif
#endif
	if (rv) {
		SP->freelist = rv -> next;
	} else {
#ifdef NONSTANDARD
		_ec_quit("No lines available in line_alloc", "");
#else
		rv = (struct line *) calloc (1, sizeof *rv);
		rv -> body = (chtype *) calloc (columns, sizeof (chtype));
#endif
	}
	rv -> length = 0;
	rv -> hash = 0;
	return rv;
}
