/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)z_abs.c	1.3 85/09/16 */
/*	@(#)z_abs.c	1.2	*/
#include "complex"

double z_abs(z)
dcomplex *z;
{
double cabs();

return( cabs( z->dreal, z->dimag ) );
}
