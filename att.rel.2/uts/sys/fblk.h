/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)fblk.h	1.3 */
struct	fblk
{
	int	df_nfree;
	daddr_t	df_free[NICFREE];
};
