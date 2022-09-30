/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

cbreak()
{
#ifdef USG
	(cur_term->Nttyb).c_lflag &= ~ICANON;
	(cur_term->Nttyb).c_cc[VMIN] = 1;
	(cur_term->Nttyb).c_cc[VTIME] = 1;
# ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "crmode(), file %lx, SP %lx, flags %x\n", SP->term_file, SP, cur_term->Nttyb.c_lflag);
#else
	if(outf) fprintf(outf, "crmode(), file %x, SP %x, flags %x\n", SP->term_file, SP, cur_term->Nttyb.c_lflag);
#endif
# endif
#else
	(cur_term->Nttyb).sg_flags |= CBREAK;
# ifdef DEBUG
	if(outf) fprintf(outf, "crmode(), file %x, SP %x, flags %x\n", SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	SP->fl_rawmode = TRUE;
	reset_prog_mode();
}
