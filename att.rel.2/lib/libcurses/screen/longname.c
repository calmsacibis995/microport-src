/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

/*
 *	This routine returns the long name of the terminal.
 */
char *
longname()
{
	register char	*cp;
	extern char ttytype[];

	for (cp=ttytype; *cp++; )		/* Go to end of string */
		;
	while (*--cp != '|' && cp>=ttytype)	/* Back up to | or beginning */
		;
	return ++cp;
}
