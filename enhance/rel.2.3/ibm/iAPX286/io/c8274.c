static char *uportid = "@(#)c8274.c	Microport Rev Id 1.3.3  6/18/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)c8274.c	1.3 */
#include "sys/mpsc.h"
/*
 * 8274 specific configuration table:
 *	This file declares the port and interrupt level table
 *	which the 8274 device driver uses.
 * NOTE: when adding additional entries:
 *	1) make sure the last row is all 0s
 *	2) channel B follows channel A for each 8274. ie
 *	   even numbered entries (0,2, ...) are for channel A
 *	   and odd entries (1,3,5, ... ) are for channel B.
 */
struct mpsc C8274[] =
/*	8274	8274	8254	8254	timer	interrupt */
/*	ctrl	data	ctrl	data	number	level	  */
{
	0xDC,	0xD8,	0xD6,	0xD4,	0x02,	0x06,	/* Ch A */
	0xDE,	0xDA,	0xD6,	0xD2,	0x01,	0x06,	/* Ch B */
	0,	0,	0,	0,	0,	0 };
