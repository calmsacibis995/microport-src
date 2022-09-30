/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)h_abs.c	1.3 85/09/16 */
/*	@(#)h_abs.c	1.2	*/
short h_abs(x)
short *x;
{
if(*x >= 0)
	return(*x);
return(- *x);
}
