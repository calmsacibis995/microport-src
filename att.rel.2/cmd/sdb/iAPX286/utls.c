/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
	/*	 utls.c: 1.3 5/17/83	*/

/* VAX disasembler utility routines.
 *
 * printline() - prints disassembled line, as stored is mneu[].
 */

printline()
{
	extern char mneu[];
	printf("%s", mneu);		/* to print */
}
