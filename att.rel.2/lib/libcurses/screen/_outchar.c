/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
 * Handy functions to put out a string with padding.
 * These make two assumptions:
 *	(1) Output is via stdio to stdout through putchar.
 *	(2) There is no count of affected lines.  Thus, this
 *	    routine is only valid for certain capabilities,
 *	    i.e. those that don't have *'s in the documentation.
 */
#include <stdio.h>

/*
 * Routine to act like putchar for passing to tputs.
 */
_outchar(ch)
char ch;
{
	putchar(ch);
}
