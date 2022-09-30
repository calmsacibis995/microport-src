/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
	/*	 extn.c: 1.3 10/4/82	*/

/*	for iAPX286 sdb
 *	External variables used by iAPX286 disassembler.
 *	(Initialized arrays are in tbls.c)
 */

#include "dis.h"

long loc;		/* byte location in section being disassembled	*/
			/* IMPORTANT: loc is incremented only by the	*/
			/* disassembler dis_dot()			*/

char mneu[NLINE];	/* array to store mneumonic code for output	*/

int argument [VARNO];	/* arguments of current instruction		*/
