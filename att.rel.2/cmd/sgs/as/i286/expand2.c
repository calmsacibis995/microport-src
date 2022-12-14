/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)expand2.c	1.3 - 85/08/08 */

#include "systems.h"
#include "symbols.h"
#include "expand.h"
#include "expand2.h"

/*
 *	This array contains the upper and lower bounds of the span
 *	permitted for a short form of a span dependent instruction
 *	(sdi).  This array must be initialized in the same units
 *	as dot,	 e.g. bytes. Also don't forget that the structure
 *	expects long values.
 */

rangetag range[NITYPES] = {
		{-128L,	127L},		/* CJMP	*/
		{-128L,	127L},		/* UJMP	*/
		{-128L,	127L},		/* LOOPI	*/
};

/*
 *	This array contains the size of the short form of a sdi.
 *	The size must be in the same units that are used by dot,	
 *	e.g. bytes.
 */

BYTE pcincr[NITYPES] = {
	2,	
	2,	
	2,	
};

/*
 *	This array contains the difference between the size of
 *	the long form of a sdi and the short form. Again the difference
 *	must be in the same units as dot.
 */

BYTE idelta[NITYPES] = {
	3,	
	1,
	5
};
