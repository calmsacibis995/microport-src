/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*LINTLIBRARY*/
/*
 *	sin and cos of double-precision argument.
 *	Returns ERANGE error and value 0 if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <values.h>
#include <errno.h>

extern double xsin(), xcos();

double
sin(x)
double x;
{
	extern double sin_cos();

	return (sin_cos(x, 0));
}

double
sin_cos(x, cosflag)
register double x;
int cosflag;
{
	register double y;
	struct exception exc;
	
	if ((exc.arg1 = x) < 0) 	
		x = -x;
	y = x;
	if(cosflag) {
		y += M_PI_2;
		exc.name = "cos";
	} else
		exc.name = "sin";
	if (y > X_TLOSS) {
		exc.type = TLOSS;
		exc.retval = 0.0;
		if (!matherr(&exc)) {
			(void) write(2, exc.name, 3);	
			(void) write(2, ": TLOSS error\n", 14);
			errno = ERANGE;
		}
		return (exc.retval);
	}

	x = exc.arg1;
	exc.retval = y = (cosflag) ? xcos(x) : xsin(x);

	if (x > X_PLOSS || x < -X_PLOSS) {
		exc.type = PLOSS;
		if (!matherr(&exc))
			errno = ERANGE;
	}
	return (y);
}

double
cos(x)
register double x;
{
	return (x ? sin_cos(x, 1) : 1.0);
}
