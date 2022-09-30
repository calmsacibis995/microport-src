/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)messages.h	1.3 - 85/08/09 */

#define	NUMMSGS	130

#ifndef WERROR
#    define	WERROR	werror
#endif

#ifndef UERROR
#    define	UERROR	uerror
#endif

#ifndef MESSAGE
#    define	MESSAGE(x)	msgtext[ x ]
#endif

extern char	*msgtext[ ];
