/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)c_div.c	1.3 85/09/16 */
/*	@(#)c_div.c	1.2	*/
struct complex { float real, imag; };

c_div(c, a, b)
struct complex *a, *b, *c;
{
double ratio, den;
double abr, abi;

if( (abr = b->real) < 0.)
	abr = - abr;
if( (abi = b->imag) < 0.)
	abi = - abi;
if( abr <= abi )
	{
	if(abi == 0)
		abort(); /* fatal("complex division by zero"); */
	ratio = b->real / b->imag ;
	den = b->imag * (1 + ratio*ratio);
	c->real = (a->real*ratio + a->imag) / den;
	c->imag = (a->imag*ratio - a->real) / den;
	}

else
	{
	ratio = b->imag / b->real ;
	den = b->real * (1 + ratio*ratio);
	c->real = (a->real + a->imag*ratio) / den;
	c->imag = (a->imag - a->real*ratio) / den;
	}

}
