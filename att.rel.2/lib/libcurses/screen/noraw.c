/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

noraw()
{
#ifdef USG
	(cur_term->Nttyb).c_cc[VINTR] = (cur_term->Ottyb).c_cc[VINTR];
	(cur_term->Nttyb).c_cc[VQUIT] = (cur_term->Ottyb).c_cc[VQUIT];
	(cur_term->Nttyb).c_iflag |= ISTRIP;
	(cur_term->Nttyb).c_cflag &= ~CSIZE;
	(cur_term->Nttyb).c_cflag |= CS7;
	(cur_term->Nttyb).c_cflag |= PARENB;
	nocrmode();
#else
	(cur_term->Nttyb).sg_flags&=~RAW;
# ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "noraw(), file %lx, SP %lx, flags %x\n", SP->term_file, SP, cur_term->Nttyb.sg_flags);
#else
	if(outf) fprintf(outf, "noraw(), file %x, SP %x, flags %x\n", SP->term_file, SP, cur_term->Nttyb.sg_flags);
#endif
# endif
#endif
	SP->fl_rawmode=FALSE;
	reset_prog_mode();
}
