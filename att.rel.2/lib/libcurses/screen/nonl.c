/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

nonl()	
{
#ifdef USG
	(cur_term->Nttyb).c_iflag &= ~ICRNL;
	(cur_term->Nttyb).c_oflag &= ~ONLCR;
# ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "nonl(), file %lx, SP %lx, flags %x,%x\n", SP->term_file, SP, cur_term->Nttyb.c_iflag, cur_term->Nttyb.c_oflag);
#else
	if(outf) fprintf(outf, "nonl(), file %x, SP %x, flags %x,%x\n", SP->term_file, SP, cur_term->Nttyb.c_iflag, cur_term->Nttyb.c_oflag);
#endif
# endif
#else
	(cur_term->Nttyb).sg_flags &= ~CRMOD;
# ifdef DEBUG
	if(outf) fprintf(outf, "nonl(), file %x, SP %x, flags %x\n", SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
	reset_prog_mode();
}
