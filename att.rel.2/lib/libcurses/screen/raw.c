/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

raw()
{
#ifdef USG
	/* Disable interrupt characters */
	(cur_term->Nttyb).c_cc[VINTR] = 0377;
	(cur_term->Nttyb).c_cc[VQUIT] = 0377;
	/* Allow 8 bit input/output */
	(cur_term->Nttyb).c_iflag &= ~ISTRIP;
	(cur_term->Nttyb).c_cflag &= ~CSIZE;
	(cur_term->Nttyb).c_cflag |= CS8;
	(cur_term->Nttyb).c_cflag &= ~PARENB;
	crmode();
#else
	(cur_term->Nttyb).sg_flags|=RAW;
#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "raw(), file %lx, SP %lx, flags %x\n", SP->term_file, SP, cur_term->Nttyb.sg_flags);
#else
	if(outf) fprintf(outf, "raw(), file %x, SP %x, flags %x\n", SP->term_file, SP, cur_term->Nttyb.sg_flags);
#endif
#endif
	SP->fl_rawmode=TRUE;
#endif
	reset_prog_mode();
}
