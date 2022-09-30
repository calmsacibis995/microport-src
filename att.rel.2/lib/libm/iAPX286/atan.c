/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	atan returns the arctangent of its double-precision argument,
 *	in the range [-pi/2, pi/2].
 *	There are no error returns.
 *
 *	atan2(y, x) returns the arctangent of y/x,
 *	in the range [-pi, pi].
 *	atan2 discovers what quadrant the angle is in and calls the
 *	287 chip calculation routine.
 *
 */

#include <math.h>
#include <values.h>
#include <errno.h>

#define NEG	1

extern double xatan();

double
atan(x)
register double x;
{
	return(atan2(x,1.0));
}

double
atan2(y, x)
register double y, x;
{
	double ay, ax, z;

	if(y+x==y)
		if(y >= 0.0)
			return(M_PI_2);
		else
			return(-M_PI_2);
	ay = (y > 0.0 ? y : -y);
	ax = (x > 0.0 ? x : -x);
	if(ay < ax) {
		if (y == 0.0) {
			if (x > 0.0)
				return(0.0);
			return(M_PI);
		}
		z = xatan(ay,ax);
	} else {
		if (x == 0.0) {
			if (y > 0.0)
				return(M_PI_2);
			return(-M_PI_2);
		}
		z = M_PI_2 - xatan(ax,ay);
	}
	if (x < 0.0)
		z = M_PI - z;
	if (y < 0.0)
		z = -z;
	return(z);
}
