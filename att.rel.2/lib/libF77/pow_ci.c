/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)pow_ci.c	1.3 85/09/16 */
/*	@(#)pow_ci.c	1.2	*/
#include "complex"

pow_ci(p, a, b) 	/* p = a**b  */
complex *p, *a;
long int *b;
{
dcomplex p1, a1;

a1.dreal = a->real;
a1.dimag = a->imag;

pow_zi(&p1, &a1, b);

p->real = p1.dreal;
p->imag = p1.dimag;
}
