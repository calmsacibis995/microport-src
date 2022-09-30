/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*LINTLIBRARY*/
/*
 *	log returns the natural logarithm of its double-precision argument.
 *	log10 returns the base-10 logarithm of its double-precision argument.
 *	Returns EDOM error and value -HUGE if argument <= 0.
 *	Algorithm and coefficients from Cody and Waite (1980).
 *	Calls frexp.
 */

#include <math.h>
#include <errno.h>
#define C1	0.693359375
#define C2	-2.121944400546905827679e-4

extern double log_error();
extern double xlog(), xlog10();


double
log(x)
register double x;
{
	register double y;

	if (x <= 0)
		return (log_error(x, "log", 3));

	return(xlog(x));
}

double
log10(x)
register double x;
{
	register double y;

	if (x <= 0)
		return (log_error(x, "log10", 5));

	return(xlog10(x));
}

static double
log_error(x, f_name, name_len)
double x;
char *f_name;
unsigned int name_len;
{
	register char *err_type;
	unsigned int mess_len;
	struct exception exc;

	exc.name = f_name;
	exc.retval = -HUGE;
	exc.arg1 = x;
	if (x) {
		exc.type = DOMAIN;
		err_type = ": DOMAIN error\n";
		mess_len = 15;
	} else {
		exc.type = SING;
		err_type = ": SING error\n";
		mess_len = 13;
	}
	if (!matherr(&exc)) {
		(void) write(2, f_name, name_len);
		(void) write(2, err_type, mess_len);
		errno = EDOM;
	}
	return (exc.retval);
}
