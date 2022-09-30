/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)iargc_.c	1.3 85/09/16 */
/*	@(#)iargc_.c	1.2	*/
long int iargc_()
{
extern int xargc;
return ( xargc - 1 );
}
