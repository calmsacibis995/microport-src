/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*	@(#)tmless.c	1.2 of 3/31/82	*/
/*
 *	return 1 if t1 earlier than t2 (times in localtime format)
 *	assumed that t1 and t2 are in same day
 */
#include <time.h>

tmless(t1, t2)
register struct tm *t1, *t2;
{
	if (t1->tm_hour != t2->tm_hour)
		return(t1->tm_hour < t2->tm_hour);
	if (t1->tm_min != t2->tm_min)
		return(t1->tm_min < t2->tm_min);
	return(t1->tm_sec < t2->tm_sec);
}
