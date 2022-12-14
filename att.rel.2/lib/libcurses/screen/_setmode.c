/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include "curses.ext"

extern	int	_outch();

_setmode ()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_setmode().\n");
#endif
	if (SP->virt_irm == SP->phys_irm)
		return;
	tputs(SP->virt_irm==1 ? enter_insert_mode : exit_insert_mode, 0, _outch);
	SP->phys_irm = SP->virt_irm;
}
