/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
 * This routine is one of the main things
 * in this level of curses that depends on the outside
 * environment.
 */
#include "curses.ext"

int outchcount;

/*
 * Write out one character to the tty.
 */
_outch (c)
chtype c;
{
#ifdef DEBUG
# ifndef LONGDEBUG
	if (outf)
		if (c < ' ')
			fprintf(outf, "^%c", (c+'@')&0177);
		else
			fprintf(outf, "%c", c&0177);
# else LONGDEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "_outch: char '%s' term %lx file %lx=%d\n",
#else
	if(outf) fprintf(outf, "_outch: char '%s' term %x file %x=%d\n",
		unctrl(c&0177), SP, SP->term_file, fileno(SP->term_file));
#endif
# endif LONGDEBUG
#endif DEBUG

	outchcount++;
	if (SP && SP->term_file)
		putc (c&0177, SP->term_file);
	else
		putc (c&0177, stdout);
}
