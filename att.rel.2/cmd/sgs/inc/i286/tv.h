/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
*/

struct tventry {
	unsigned short	tv_ip;		/* offset  */
	unsigned short	tv_cs;		/* segment */
	};

#define TVENTRY struct tventry
#define TVENTSZ sizeof(TVENTRY)
