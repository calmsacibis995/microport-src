/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

flushinp()
{
#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "flushinp(), file %lx, SP %lx\n", SP->term_file, SP);
#else
	if(outf) fprintf(outf, "flushinp(), file %x, SP %x\n", SP->term_file, SP);
#endif
#endif
#ifdef USG
	ioctl(cur_term -> Filedes, TCFLSH, 0);
#else
	/* for insurance against someone using their own buffer: */
	ioctl(cur_term -> Filedes, TIOCGETP, &(cur_term->Nttyb));

	/*
	 * SETP waits on output and flushes input as side effect.
	 * Really want an ioctl like TCFLSH but Berkeley doesn't have one.
	 */
	ioctl(cur_term -> Filedes, TIOCSETP, &(cur_term->Nttyb));
#endif
	/*
	 * Have to doupdate() because, if we've stopped output due to
	 * typeahead, now that typeahead is gone, so we'd better catch up.
	 */
	doupdate();
}
