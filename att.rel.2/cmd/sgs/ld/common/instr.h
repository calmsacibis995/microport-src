/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)instr.h	1.3 - 85/08/09 */
/*
 */

#if PERF_DATA
extern long	nwalks, nfwalks, ncolisns, maxchain;

extern long	allocspc;

extern long ttime;
extern struct tbuffer {
	long proc_user_time;
	long proc_system_time;
	long child_user_time;
	long child_system_time;
	} ptimes;
#endif
