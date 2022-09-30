/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#include "curses.ext"

/* '_line_free' returns a line object to the free list */
_line_free (p)
register struct line   *p;
{
	register int i, sl, n=0;
	register struct line **q;

	if (p == NULL)
		return;
#ifdef DEBUG
#if iAPX286 & LARGE_M
	if(outf) fprintf(outf, "mem: Releaseline (%lx), prev SP->freelist %lx", p, SP->freelist);
#else
	if(outf) fprintf(outf, "mem: Releaseline (%x), prev SP->freelist %x", p, SP->freelist);
#endif
#endif
	sl = lines;
	for (i=sl,q = &SP->cur_body[sl]; i>0; i--,q--)
		if (p == *q)
			n++;
	for (i=sl,q = &SP->std_body[sl]; i>0; i--,q--)
		if (p == *q)
			n++;
#ifdef DEBUG
	if(outf) fprintf(outf, ", count %d\n", n);
#endif
	if (n > 1)
		return;
	p -> next = SP->freelist;
	p -> hash = -888;
	SP->freelist = p;
}
