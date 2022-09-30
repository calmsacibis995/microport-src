/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)maps.c	1.3 - 85/08/09 */
#include "system.h"

#include <stdio.h>
#include "structs.h"
#include "attributes.h"
#include "extrns.h"
#include "sgsmacros.h"
#include "sgs.h"

ldmap()
{
#if !iAPX286
    if ( !(IAPX16MAGIC(magic)) ){
	/*
	 * This version of maps.c, for Xiapxld, does NOT have:
	 *	1. REGION directives
	 *	2. virtual addresses
	 */
	/*eject*/

	/*
	 * Generate the output map requested through the "-m" flag
	 */
	
		ANODE *a;
	
	/*
	 * Step 1: Output any non-default description of memory
	 */
	
		if (memlist.head != memlist.tail) {
			MEMTYPE *p;
			printf("\n\n\n\t\t\tMEMORY CONFIGURATION\n");
			printf("\nname\t\torigin\t\tlength\t\tattributes\n\n");
			for( p = (MEMTYPE *) memlist.head; p; p = p->mtnext ) {
#if UNIX < 4
				printf("%-8.8s\t%08.2lx\t%08.2lx\t  ",
#else
				printf("%-8.8s\t%#8.6lx\t%#8.6lx\t  ",
#endif
					p->mtname, p->mtorig, p->mtlength);
				if (p->mtattr & att_R) printf("R");
				if (p->mtattr & att_W) printf("W");
				if (p->mtattr & att_I) printf("I");
				if (p->mtattr & att_X) printf("X");
				printf("\n");
				}
			printf("\n\n");
			}
	
	/*
	 * Step 2: Output the physical memory map
	 */
	
		printf("\t\tLINK EDITOR MEMORY MAP\n\n");
		printf("\noutput\t\tinput\t\tphysical");
		printf("\nsection\t\tsection\t\taddress\t\tsize\n");
	
		for (a = (ANODE *) avlist.head; a; a = a->adnext) {
			long disp, ndx, vaddr;
			unsigned fill;
			INSECT *isp;
			OUTSECT *osp;
	
			if (a->adtype == ADPAD)
				continue;
	
			vaddr = a->adpaddr;
	
			if (a->adtype == ADAVAIL) {
#if UNIX < 4
				printf("\n*avail*\t\t\t\t%08.2lx\t%08.2lx\n",
#else
				printf("\n*avail*\t\t\t\t%#8.6lx\t%#8.6lx\n",
#endif
					vaddr, a->adsize);
				continue;
				}
	
			osp = a->adscnptr;
#if UNIX < 4
			printf("\n%-8.8s\t\t\t%08.2lx\t%08.2lx",
#else
			printf("\n%-8.8s\t\t\t%#8.6lx\t%#8.6lx",
#endif
				osp->oshdr.s_name, vaddr, a->adsize);
			if (osp->oshdr.s_flags & STYP_DSECT)
				printf("\tDSECT");
			else if (osp->oshdr.s_flags & STYP_NOLOAD)
				printf("\tNOLOAD");
			else if (osp->oshdr.s_scnptr == 0L)
				printf("\tuninitialized");
			printf("\n");
	
			ndx = 0;
			isp = osp->osinclhd;
			if( (osp->oshdr.s_scnptr != 0L || isp)  &&
			    (! equal(_TV,osp->oshdr.s_name,8)) )
				while (ndx < osp->oshdr.s_size) {
					if (isp)
						disp = isp->isdispl;
					else
						disp = osp->oshdr.s_size;
					if (ndx < disp) {
						if (osp->osflags & FILL)
							fill = osp->osfill;
						else
							fill = globfill;
#if UNIX < 4
						printf("\t\t*fill*\t\t%08.2lx\t%08.2lx\t%06.2x\n",
#else
						printf("\t\t*fill*\t\t%#8.6lx\t%#8.6lx\t%#6.4x\n",
#endif
						  	vaddr + ndx,
						  	disp - ndx, fill);
						ndx = disp;
						}
					if (isp) {
						if (isp->ishdr.s_size > 0) {
#if UNIX < 4
							printf( "\t\t%-8.8s\t%08.2lx\t%08.2lx %s",
#else
							printf( "\t\t%-8.8s\t%#8.6lx\t%#8.6lx %s",
#endif
								isp->ishdr.s_name,
	        						vaddr + ndx,
	        						isp->ishdr.s_size,
	        						sname(isp->isfilptr->flname));
							if( isp->isfilptr->flfilndx != -1)
								printf( " (%d)", isp->isfilptr->flfilndx);
							printf( "\n" );
	        					ndx += isp->ishdr.s_size;
	    						}
						isp = isp->isincnxt;
						}
					}
			}
    } else
#endif /* if !iAPX286 */
    {

/*
 * This version of maps.c, for iapxld, supports:
 *	1. REGION directives
 *	2. both physical and virtual addresses
 */
/*eject*/

/*
 * Generate the output map requested through the "-m" flag
 */

	register ANODE *a;
	register REGION *r;
	register MEMTYPE *p;

/*
 * Step 1: Output any non-default description of memory
 */

	if (memlist.head != memlist.tail) {
		printf("\n\n\n\t\t\tMEMORY CONFIGURATION\n");
		printf("\nname\t\torigin\t\tlength\t\tattributes\n\n");
		for( p = (MEMTYPE *) memlist.head; p; p = p->mtnext ) {
			printf("%-8.8s\t%08.2lx\t%08.2lx\t  ",
				p->mtname, p->mtorig, p->mtlength);
			if (p->mtattr & att_R) printf("R");
			if (p->mtattr & att_W) printf("W");
			if (p->mtattr & att_I) printf("I");
			if (p->mtattr & att_X) printf("X");
			printf("\n");
			}
		printf("\n\n");
		}

/*
 * Step 2: Output the virtual memory map
 */

	printf("\t\tLINK EDITOR MEMORY MAP\n\n");
	printf("\n\toutput\tinput       physical\tvirtual");
	printf("\nregion\tsection\tsection     address\taddress      size\n");

	for( r = (REGION *) reglist.head; r; r = r->rgnext ) {
		long disp, ndx, vaddr;
		unsigned fill;
		INSECT *isp;
		OUTSECT *osp;

		if( strlen(r->rgname) > 0 ) {
			printf("\n%-8.8s\t\t    %08.2lx    %08.2lx    %08.2lx\n",
				r->rgname, r->rgorig, r->rgvaddr, r->rglength);
			}
		else
			printf("\n*region origin*\t\t    %08.2lx    %08.2lx    %08.2lx\n",
				r->rgorig, r->rgvaddr, r->rglength);
		
		for( a = r->rgaddrhd; a != r->rgaddrtl->adnext; a = a->adnext ) {
			if( (a->adregp != r)  &&  a->adregp )
				continue;
			if(a->adtype == ADPAD )
				continue;
			vaddr = a->adpaddr - r->rgorig + r->rgvaddr;
			if( a->adtype == ADAVAIL ) {
				printf("\n\t*avail*\t\t    %08.2lx    %08.2lx    %08.2lx\n",
					a->adpaddr, vaddr, a->adsize);
				continue;
				}

			osp = a->adscnptr;
			printf("\n\t%-8.8s\t    %08.2lx    %08.2lx    %08.2lx",
					osp->oshdr.s_name, a->adpaddr, vaddr, a->adsize);
			if( osp->oshdr.s_flags & STYP_DSECT )
				printf("\tDSECT");
			else if( osp->oshdr.s_flags & STYP_NOLOAD )
				printf("\tNOLOAD");
			else if( osp->oshdr.s_scnptr == 0L )
				printf("\tuninitialized");
			printf("\n");

			ndx = 0;
			isp = osp->osinclhd;
			if( (osp->oshdr.s_scnptr != 0L || isp)  &&
			    ( ! equal(_TV, osp->oshdr.s_name, 8)) )
				while( ndx < osp->oshdr.s_size ) {
					if (isp)
						disp = isp->isdispl;
					else
						disp = osp->oshdr.s_size;
					if( ndx < disp ) {
						if( osp->osflags & FILL )
							fill = osp->osfill;
						else
							fill = globfill;
						printf("\t\t*fill*\t    %08.2lx    %08.2lx    %08.2lx\t\t%06.2x\n",
						  	a->adpaddr + ndx,
						  	vaddr + ndx,
						  	disp - ndx,   fill);
						ndx = disp;
						}
					if( isp ) {
						if( isp->ishdr.s_size > 0) {
        						printf("\t\t%-8.8s    %08.2lx    %08.2lx    %08.2lx\t%s",
								isp->ishdr.s_name,
        						  	a->adpaddr + ndx, vaddr + ndx,
        						  	isp->ishdr.s_size,
        						  	sname(isp->isfilptr->flname));
							if (isp->isfilptr->flfilndx != -1)
								printf(" (%d)", isp->isfilptr->flfilndx);
							printf("\n");
        						ndx += isp->ishdr.s_size;
    							}
						isp = isp->isincnxt;
						}
					}
			}
		}
    }
}
