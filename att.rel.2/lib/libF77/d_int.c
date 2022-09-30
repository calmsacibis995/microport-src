/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)d_int.c	1.3 85/09/16 */
/*	@(#)d_int.c	1.2	*/
double d_int(x)
double *x;
{
double floor();

return( (*x>0) ? floor(*x) : -floor(- *x) );
}
