/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*LINTLIBRARY*/
/*
 *	exp returns the exponential function of its double-precision argument.
 *	Returns ERANGE error and value 0 if argument too small,
 *	   value HUGE if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
*/

#include <math.h>			/* while testing only */
#include <values.h>
#include <errno.h>

extern double xexp(), xldexp();

double
exp(x)
register double x;
{
	register int n;
	register double y,z;
	struct exception exc;

	exc.arg1 = x;
	if (x < 0)
		x = -x;
	if (x < X_EPS) /* use first term of Taylor series expansion */
		return (1.0 + exc.arg1);
	exc.name = "exp";
	if (exc.arg1 <= LN_MINDOUBLE) {
		if (exc.arg1 == LN_MINDOUBLE) /* protect against roundoff */
			return (MINDOUBLE); /* causing ldexp to underflow */
		exc.type = UNDERFLOW;
		exc.retval = 0.0;
		if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
	}
	if (exc.arg1 >= LN_MAXDOUBLE) {
		if (exc.arg1 == LN_MAXDOUBLE) /* protect against roundoff */
			return (MAXDOUBLE); /* causing ldexp to overflow */
		exc.type = OVERFLOW;
		exc.retval = HUGE;
		if (!matherr(&exc)) 
			errno = ERANGE;
		return (exc.retval);
	}

	y = (x = exc.arg1) * M_LOG2E;
	x = (y - (double)(n = (int)floor(y)))-0.5;
	if(x < 0)
		x = xexp(y - (double)n);
	else
		x = xexp(x) * M_SQRT2;

	return(xldexp(x,n));
}
