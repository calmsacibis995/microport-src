/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*LINTLIBRARY*/
/*
 *	sqrt returns the square root of its double-precision argument
 *	using the 80287 processor or the 80287 emulator.
 *	Returns EDOM error and value 0 if argument negative.
 */

#include <errno.h>
#include <math.h>			/* temporary location	*/
extern double xsqrt();

double
sqrt(x)
register double x;
{

	if (x <= 0) {
		struct exception exc;

		if (!x)
			return (x); /* sqrt(0) == 0 */
		exc.type = DOMAIN;
		exc.name = "sqrt";
		exc.arg1 = x;
		exc.retval = 0.0;
		if (!matherr(&exc)) {
			(void) write(2, "sqrt: DOMAIN error\n", 19);
			errno = EDOM;
		}
		return (exc.retval);
	}

	return(xsqrt(x));
}
