/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)h_dim.c	1.3 85/09/16 */
/*	@(#)h_dim.c	1.2	*/
short h_dim(a,b)
short *a, *b;
{
return( *a > *b ? *a - *b : 0);
}