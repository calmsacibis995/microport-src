/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

savetty()
{
	SP->save_tty_buf = cur_term->Nttyb;
#ifdef DEBUG
# ifdef USG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "savetty(), file %lx, SP %lx, flags %x,%x,%x,%x\n", SP->term_file, SP, cur_term->Nttyb.c_iflag, cur_term->Nttyb.c_oflag, cur_term->Nttyb.c_cflag, cur_term->Nttyb.c_lflag);
#else
	if(outf) fprintf(outf, "savetty(), file %x, SP %x, flags %x,%x,%x,%x\n", SP->term_file, SP, cur_term->Nttyb.c_iflag, cur_term->Nttyb.c_oflag, cur_term->Nttyb.c_cflag, cur_term->Nttyb.c_lflag);
#endif
# else
	if(outf) fprintf(outf, "savetty(), file %x, SP %x, flags %x\n", SP->term_file, SP, cur_term->Nttyb.sg_flags);
# endif
#endif
}
