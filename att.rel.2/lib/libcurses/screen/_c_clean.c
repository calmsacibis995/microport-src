/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include "curses.ext"

extern	int	_outch();

_c_clean ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_c_clean().\n");
#endif
	_hlmode (0);
	_kpmode(0);
	SP->virt_irm = 0;
	_window(0, lines-1, 0, columns-1);
	_syncmodes();
	tputs(exit_ca_mode, 0, _outch);
	tputs(cursor_normal, 0, _outch);
}
