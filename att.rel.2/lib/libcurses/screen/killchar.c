/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "curses.ext"

char
killchar()
{
#ifdef USG
	return cur_term->Ottyb.c_cc[VKILL];
#else
	return cur_term->Ottyb.sg_kill;
#endif
}
