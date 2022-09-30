static char *uportid = "@(#)dtlib.c	Microport Rev Id  1.3.2 6/10/86";

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
**	Turn year, month, day, hour, minute, second codes
**  into UNIX-style number of seconds since Jan 1, 1970.
**  Code stolen from date.c by Lance Norskog, 4-26-86.
**  In order to properly set the day of week field of CMOS.
*/

#include	<sys/types.h>
#include	"utmp.h"
#include        <stdio.h>
#include	<time.h>

#define	dysize(A) (((A)%4)? 365: 366)

int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*
 * year is 86, not 1986.
 * month is 1-12
 * day is 1-31
 * hour is 0-23
 * minute is 0-59
 * second is 0-59
 */


time_t
dtform(year, month, day, hour, minute, second)
{

	int	i;
	long nt;
	time_t	timbuf;

	tzset();

	if (year<0) {
		(void) time(&nt);
		year = localtime(&nt)->tm_year;
	}
	if(month<1 || month>12)
		return((time_t) -1);
	if(day<1 || day>31)
		return((time_t) -1);
	if(hour == 24) {
		hour = 0;
		day++;
	}
	if (hour<0 || hour>23)
		return((time_t) -1);
	if(minute<0 || minute>59)
		return((time_t) -1);
	if(second<0 || second>59)
		return((time_t) -1);
	timbuf = 0;
	year += 1900;
	for(i=1970; i<year; i++)
		timbuf += dysize(i);
	/* Leap year */
	if (dysize(year)==366 && month >= 3)
		timbuf += 1;
	while(--month)
		timbuf += dmsize[month-1];
	timbuf += (day-1);
	timbuf *= 24;
	timbuf += hour;
	timbuf *= 60;
	timbuf += minute;
	timbuf *= 60;
	timbuf += second;

	/* convert to Greenwich time, on assumption of Standard time. */
	timbuf += timezone;

	/* Now fix up to local daylight time. */
	if (localtime(&timbuf)->tm_isdst)
		timbuf += -1*60*60;
	return timbuf;
}



