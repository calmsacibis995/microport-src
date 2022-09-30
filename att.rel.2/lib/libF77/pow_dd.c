/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)pow_dd.c	1.3 85/09/16 */
/*	@(#)pow_dd.c	1.2	*/
double pow_dd(ap, bp)
double *ap, *bp;
{
double pow();

return(pow(*ap, *bp) );
}
