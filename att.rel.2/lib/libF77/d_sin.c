/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)d_sin.c	1.3 85/09/16 */
/*	@(#)d_sin.c	1.2	*/
double d_sin(x)
double *x;
{
double sin();
return( sin(*x) );
}
