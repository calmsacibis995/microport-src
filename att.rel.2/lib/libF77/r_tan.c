/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)r_tan.c	1.3 85/09/16 */
/*	@(#)r_tan.c	1.2	*/
double r_tan(x)
float *x;
{
double tan();
return( tan(*x) );
}
