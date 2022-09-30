/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)specsyms.c	1.3 - 85/08/09 */

#include <stdio.h>

#include "structs.h"
#include "extrns.h"
#include "tv.h"
#include "ldtv.h"
#include "sgsmacros.h"
#include "sgs.h"
set_spec_syms()
{

/*
 * For non-relocatable link edits, set up the special symbols.
 *
 *	1. _TORIGIN : segment register value for the REGION containing
 *			text
 *	   _DORIGIN : segment register value for the REGION containing
 *			data
 *	   _TVORIG  : segment register value for the REGION containing
 *			the tv
 *	
 *	2. _ETEXT : next available address after the end of the output
 *			section _TEXT
 *	   _EDATA : next available address after the end of the output
 *			section _DATA
 *	   _END   : next available address after the end of the output
 *			section _BSS
 *
 *	3. _SORIGIN : first address at or following _END which is 
 *			aligned to a 16-byte boundary
 *
 *	4. _TV : special symbol with auxiliary entry giving
 *		 tvlength and tvrange.
 */

	register OUTSECT *osptr;
	register long base;
#if iAPX286
	long endtext, enddata, endbss;
#endif
	SYMTAB	*sym;
	AUXENT aux;

	if ( ! aflag )
		return;

#if iAPX286
	/*
	 * the 286 operating system will figure out from the physical
	 * addresses what to load the base register with, so the
	 * ORIGIN symbols won't be created for it.
	 */

	endtext = 0;			/* initialize for the for loop */
	enddata = 0;
	endbss = 0;

	/*
	 * We want to find the biggest starting address for each
	 * section type and add its size to get the end address of
	 * each type.  You can't just add the starting address of
	 * the first .text section, say, to the .text section sizes
	 * because of the funny addressing.  The text section could
	 * take up two segments, one starting at 0x570000 and the
	 * next starting at 0x5f0000, but only be little over 64K bytes
	 * long!
	 */

	for( osptr = (OUTSECT *) outsclst.head; osptr; osptr=osptr->osnext ) {
		if( equal(osptr->oshdr.s_name, _TEXT, 8) ) {
			if ( endtext <= osptr->oshdr.s_paddr )
				endtext = osptr->oshdr.s_paddr + 
							osptr->oshdr.s_size;
			}
		else if( equal(osptr->oshdr.s_name, _DATA, 8) ) {
			if ( enddata <= osptr->oshdr.s_paddr )
				enddata = osptr->oshdr.s_paddr + 
							osptr->oshdr.s_size;
			}
		else if( equal(osptr->oshdr.s_name, _BSS, 8) ) {
			if ( endbss <= osptr->oshdr.s_paddr )
				endbss = osptr->oshdr.s_paddr + 
							osptr->oshdr.s_size;
			}
		}

        creatsym(_ETEXT, endtext);
        creatsym(_EDATA, enddata);
        creatsym(_END, endbss);

#else

if(IAPX16MAGIC(magic)) {

        if( reglist.head == reglist.tail ) {     /* only 1 region */
        	base = (((REGION *) reglist.head)->rgorig - ((REGION *) reglist.head)->rgvaddr) >> 4;
        	creatsym(_DORIGIN, base);
          	creatsym(_SORIGIN, base);
        	creatsym(_TORIGIN, base);
        	}
        else if( iflag  &&  ! tvflag ) {	/* separate i & d */
        	base = (((REGION *) reglist.head)->rgorig - ((REGION *) reglist.head)->rgvaddr) >> 4;
          	creatsym(_TORIGIN, base);
          	base = (((REGION *) reglist.tail)->rgorig - ((REGION *) reglist.tail)->rgvaddr) >> 4;
          	creatsym(_DORIGIN, base);
          	creatsym(_SORIGIN, base);
          	}
          else {
		REGION *rp;
		
		if( (sym = (SYMTAB *) findsym(_START)) != NULL ) {
			if( sym->smscnptr != NULL) {
				osptr = sym->smscnptr->isoutsec;
				base = (osptr->oshdr.s_paddr - osptr->oshdr.s_vaddr) >> 4;
				creatsym(_TORIGIN, base);
				}
			}

		for( rp = (REGION *) reglist.head; rp != NULL; rp = rp->rgnext )
			if( equal(".rdata", rp->rgname, 6) ) {
				base = (rp->rgorig - rp->rgvaddr) >> 4;
				creatsym(_DORIGIN, base);
				creatsym(_SORIGIN, base);
				break;
				}
		}

} /* end if IAPX16MAGIC */

	osptr = (OUTSECT *) outsclst.head;
	while (osptr) {
		if( equal(osptr->oshdr.s_name,_TEXT, 8))
			creatsym(_ETEXT,
				osptr->oshdr.s_paddr + osptr->oshdr.s_size);
		else if( equal(osptr->oshdr.s_name,_DATA, 8))
			creatsym(_EDATA,
				osptr->oshdr.s_paddr + osptr->oshdr.s_size);
		else if( equal(osptr->oshdr.s_name,_BSS, 8)) {
			base = osptr->oshdr.s_paddr + osptr->oshdr.s_size;
			creatsym(_END, base);
			if( IAPX20MAGIC(magic) ) {
				base = (base + 15) >> 4;
				creatsym(_SORIGIN, base);
				}
			}
		else if( equal(osptr->oshdr.s_name,_TV,8) ) {
			creatsym(_TVORIG, osptr->oshdr.s_paddr >> 4);
			creatsym(_TV, ((tvspec.tvbndadr == -1L) ?
					0L : tvspec.tvbndadr));
			zero(&aux, AUXESZ);
			aux.x_tv.x_tvlen = tvspec.tvlength;
			aux.x_tv.x_tvran[0] = tvspec.tvrange[0];
			aux.x_tv.x_tvran[1] = tvspec.tvrange[1];
			aux.x_tv.x_tvfill = -1L;
			sym = findsym(_TV);
			makeaux(sym, &aux, 0);
			}
		osptr = osptr->osnext;
		}
#endif /* if iAPX286 */
}
