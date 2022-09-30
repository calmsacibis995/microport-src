/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#if vax
char **execargs = (char**)(0x7ffffffc);
#endif

#if pdp11
char **execargs = (char**)(-2);
#endif

#if u3b || u3b5
/* empty till we can figure out what to do for the shell */
#endif
