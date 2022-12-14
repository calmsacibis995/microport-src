/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
 *	isrequest(s, dest, seqno) - predicate which returns true if
 *		s is a syntactically correct request id as returned by lp(1).
 *		No check is made to see if it is in the output queue.
 *		if s is a request id, then
 *			dest is set to the destination
 *			seqno is set to the sequence number
 */

#include	"lp.h"


isrequest(s, dest, seqno)
char *s;
char *dest;
int *seqno;
{
	char *strchr(), *p, *strncpy();

	if((p = strchr(s, '-')) == NULL || p == s || p - s > DESTMAX ||
	    (*seqno = atoi(p + 1)) <= 0 || *seqno >= SEQMAX)
		return(FALSE);
	strncpy(dest, s, p - s);
	*(dest + (p - s)) = '\0';
	return(isdest(dest));
}
