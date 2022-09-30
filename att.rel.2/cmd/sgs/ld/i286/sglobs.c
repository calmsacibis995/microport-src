/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)sglobs.c	1.3 - 85/08/09 */

#include <stdio.h>

#include "system.h"
#include "structs.h"
#include "tv.h"
#include "ldtv.h"

#if iAPX286
#include "params.h" 	/* include because of STKSIZ */

char	Xflag = 1;      /* do generate optional header with "old" a.out format */
short	kflag = STKSIZ,	/* what stack size is desired */
	Kflag,		/* 1 if actual byte counts wanted in the unix header */
	Rflag,		/* Real Adress mode if 1.  Is this really used?	*/
	model;		/* small, large, huge? */
#else
char	Xflag = 0;      /* don't generate optional header with "old" a.out format */
#endif

/*eject*/
/*
 * Structure of information needed about the transfer vector (and the
 * .tv section).  Part of this structure is added to outsclst, so that
 * the list must not be freed before the last use of tvspec.
 */

#if TRVEC
OUTSECT tvoutsc;

TVINFO	tvspec = {
	&tvoutsc,	/* tvosptr */
	"",		/* tvfnfill: fill name for tv slots	*/
	{ -1,		/* tvipfill: fill value for tv slots	*/
	  -1 },		/* tvcsfill: fill value for tv slots	*/
	NULL,		/* tvinflnm: file containing tv specs	*/
	0,		/* tvinlnno: line nbr of tv directive	*/
	0,		/* tvlength: tv area length		*/
	-1L,		/* tvbndadr: tv area bond address	*/
	0, 0		/* tvrange				*/
	};
#endif

/*
 *   Normally these parameters are found in a seperate file (params.h)
 *   and are #define constants, but for the iapx loader they must be
 *   global variables.  Hence, they are being defined here.
 *
 */

/*
 * Maximum size of a section
 */
long MAXSCNSIZE;
long MAXSCNSZ;


/*
 * Default size of configured memory
 */
#if !iAPX286
long MEMORG;
#endif
long MEMSIZE;

/*
 * To contain the size of a region:
 */
long REGSIZE;

unsigned short magic = (unsigned short) 0;
unsigned short dynamagic = 0;
