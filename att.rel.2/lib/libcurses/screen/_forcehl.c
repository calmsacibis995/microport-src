/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include "curses.ext"

/*
 * Output the string to get us in the right highlight mode,
 * no matter what mode we are currently in.
 */
_forcehl()
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_forcehl().\n");
#endif
	SP->phys_gr = -1;
	_sethl();
}
