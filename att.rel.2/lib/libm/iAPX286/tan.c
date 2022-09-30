/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*LINTLIBRARY*/
/*
 *	tan returns the tangent of its double-precision argument.
 *	Returns ERANGE error and value 0 if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <values.h>
#include <errno.h>

extern double xtan();

double
tan(x)
register double x;
{
	struct exception exc;

	exc.name = "tan";
	exc.arg1 = x;
	if (x < 0) {
		x = -x;
	}
	if (x > X_TLOSS/2) {
		exc.type = TLOSS;
		exc.retval = 0.0;
		if (!matherr(&exc)) {
			(void) write(2, "tan: TLOSS error\n", 17);
			errno = ERANGE;
		}
		return (exc.retval);
	}

	exc.retval = xtan(x);
	if(exc.arg1 > X_PLOSS/2 || exc.arg1 < -X_PLOSS/2) {
		exc.type = PLOSS;
		if (!matherr(&exc))
			errno = ERANGE;
	}
	return (exc.retval);
}
