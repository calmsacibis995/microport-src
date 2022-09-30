/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*LINTLIBRARY*/
/*
 *	fabs returns the absolute value of its double-precision argument.
 */

double
fabs(x)
register double x;
{
	return (x < 0 ? -x : x);
}
