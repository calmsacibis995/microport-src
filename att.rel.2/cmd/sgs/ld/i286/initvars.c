/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)initvars.c	1.3 - 85/08/09 */
/*
 * This routine uses the magic number of the first
 * object file to determine what kind of load is being
 * linked together.
 */
#include "system.h"
#include "structs.h"
#include "extrns.h"
#include "params.h"

void
initvars(firstword)
unsigned short firstword;
{
	
#if iAPX286
	if ( firstword == (unsigned short) I286SMAGIC ) {
		dynamagic = (unsigned short) I286SMAGIC;
		model = M_SMALL;
#else
	if ( (firstword == (unsigned short) IAPX16) ||
	     (firstword == (unsigned short) IAPX16TV) ) {
		
		/*  SET VARIABLES TO 16-BIT VALUES  */

		dynamagic = (unsigned short) IAPX16;
#endif
		
		/*
		 * Maximum size of a section
		 */
		MAXSCNSIZE = 0x10000L;
		MAXSCNSZ = 0x0fff0L;
		
		
		/*
		 * Default size of configured memory
		 */
#if iAPX286
		MEMSIZE = 0x20000000L;
#else
		MEMORG = 0x0L;
		MEMSIZE = 0x200000L;
		
		/*
		 * Size of a region. If IAPX16MAGIC(dynamagic) returns
		 * zero, the link editor will NOT
		 * permit the use of REGIONS, nor partition the address space
		 */
		
		REGSIZE = 0x10000L;
#endif
		
				
	}
#if iAPX286
	else { 
	  if ( firstword == (unsigned short) I286LMAGIC ) {
		dynamagic = (unsigned short) I286LMAGIC;
		model = M_LARGE;
		/*
		 * This is a crazy section size, but it preserves
		 * the 3 protection bits used for finding the next
		 * plausible address.  This could possibly be changed
		 * back to 0x10000L if the alignment algorithm were
		 * changed in alloc.c (in the places where an address
		 * is shifted left 16 bits and has 8 added to it).
		 */
		MAXSCNSIZE = 0xffffL;	
		MAXSCNSZ = MAXSCNSIZE;
		MEMSIZE = 0x20000000L;
#else
	else { if ( (firstword == (unsigned short) IAPX20) ||
		  (firstword == (unsigned short) IAPX20TV) ) {
			
		/*  SET VARIABLES TO 20-BIT VALUES  */
		

		dynamagic = (unsigned short) IAPX20;
		
		/*
		 * Maximum size of a section
		 */
		MAXSCNSIZE = 0x1000000L;
		MAXSCNSZ = MAXSCNSIZE;
		
		
		/*
		 * Default size of configured memory
		 */
		MEMORG = 0x000L;		/* ewb */
		MEMSIZE = 0x2000000L;
		
		REGSIZE = 0x1000000L;
		
#endif
					
		}
	} /* end else-if */
	magic = firstword;
	memorg = MEMORG;
}


