/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include "curses.ext"

/*
 * Set the virtual insert/replacement mode to new.
 */
_insmode (new)
int new;
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_insmode(%d).\n", new);
#endif
	SP->virt_irm = new;
}
