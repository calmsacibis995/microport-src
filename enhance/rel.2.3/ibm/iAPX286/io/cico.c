static char *uportid = "@(#)cico.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)cico.c	1.4 */
 /*
 *
 * polled video screen and keyboard 
 *
 *		Copyright 1985 Microport Systems
 *		All Rights Reserved.
 *
 *	Modification History:
 * M000:	uport!dwight Mon Feb 17 09:29:35 PST 1986
 *	Modified for the IBM AT. The putchar() function has been
 *	moved to keybrd.c. 
 *
 *	getchar() is currently hardwired to return a 'y', for
 *	the dump() code.
 *
 */

#define CONMINDEV 1		 /* which console device */
getchar(){
return('y');
}
