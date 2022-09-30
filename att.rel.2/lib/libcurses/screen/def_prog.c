/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include	"curses.h"
#include	"term.h"

extern	struct term *cur_term;

def_prog_mode()
{
#ifdef USG
	ioctl(cur_term -> Filedes, TCGETA, &(cur_term->Nttyb));
#else
	ioctl(cur_term -> Filedes, TIOCGETP, &(cur_term->Nttyb));
#endif
}
