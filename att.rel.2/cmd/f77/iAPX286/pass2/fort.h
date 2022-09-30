/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*	iAPX286 @(#)fort.h	1.3 85/09/06 */
/*	machine dependent file  */

label( n ){
	printf( ".%d:\n", n );
	}

tlabel(){
	cerror("code generator asked to make label via tlabel\n");
	}
