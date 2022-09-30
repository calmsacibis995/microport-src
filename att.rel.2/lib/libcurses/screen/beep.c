/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include "curses.ext"

extern	int	_outch();

beep()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "beep().\n");
#endif
    if (bell)
	tputs (bell, 0, _outch);
    else
	tputs (flash_screen, 0, _outch);
    __cflush();
}