/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)special2.c	1.3 - 85/08/09 */

#include <stdio.h>

#include "system.h"
#include "aouthdr.h"
#include "structs.h"
#include "ldfcn.h"
#include "tv.h"
#include "ldtv.h"
#include "slotvec.h"
#include "reloc.h"
#include "extrns.h"
#include "sgsmacros.h"
#include "params.h"
#include "sgs.h"

#if iAPX286
#include "list.h"
#endif

#if AR32W
extern unsigned short swapb2();
#define SWAPB2(x) swapb2(x)
#else
#define SWAPB2(x) x
#endif

#if TRVEC
#if AR32W
static union {
	long l;
	unsigned short u[2];
	char c[4];
	} tmpslot;
static char tmp_cs;
#endif
#endif


static char tvreferr[] = "tv reference to non-tv symbol: addr %0.1lx, index %ld, file %s";

void
adjneed(need, osp, sap)

ADDRESS	*need;
OUTSECT	*osp;
ANODE	*sap;
{
	/*
	 * adjust amount of memory allocated for a particular
	 * section;  special processing step now used only
	 * for DMERT with TRVEC
	 */
}


void
undefine()
{
	/*
	 * Make the symbol _CSTART in ld01.c undefined
	 */

	undefsm(_CSTART);
}

void
dfltsec()
{

	/*
	 * For iapx version of ld, there are no default SECTIONS directives
	 * to apply
	 */
#if iAPX286

	ENODE *textorg, *dataorg;  /* orgins of the text and data segments    */
	long textnum;		   /* contains the value pointed to by textorg*/
	ACTITEM *aiptr, *grptr;

/*
 * If any SECTIONS directives have been input, they take priority,
 * and no default action is taken
 */

	if( bldoutsc.head )
		return;

/*
 * Generate a series of action itmes, as if the following had been
 * input to i286ld:
 *
 * for the small model:
 *
 *	SECTIONS {
 *		.text textorg: {}
 *		GROUP dataorg : {
 *					.data : {}
 *					.bss  : {}
 *		}
 *	}
 *
 * for the large model:
 *	SECTIONS {
 *		.text textorg : {}
 *		.data dataorg : {}
 *		.bss : {}
 *	}
 *
 * Steps:
 *	1. Define background variables
 *	2. Process the .text definition
 *	3. Process the GROUP definition
 *	4. Process the .data definition
 *	5. Process the .bss definition
 *
 *
 * The logic used was obtained by simulating the action of the parser.
 */

	curfilnm = "*i286.default.file*";

	textnum	= SMTXTORG;
	textorg = cnstnode( textnum );
	
	if ( model == M_LARGE ) 
		memorg = textnum;

	if ( model == M_SMALL ) 
		dataorg = cnstnode( textnum + NEXTSECTION + kflag );
	else /* model == M_LARGE */
		dataorg = cnstnode( (textnum & 0x70000L) | NEXTSECTION );
			
	lineno = 2;
	aiptr = dfnscngrp( AIDFNSCN, iflag ? NULL : textorg, NULL, NULL);
	aiptr->dfnscn.aisctype = STYP_TEXT;
	copy( aiptr->dfnscn.ainame, _TEXT, 8 );
	aiptr->dfnscn.aifill = 0;
	aiptr->dfnscn.aifillfg = 0;
	listadd( l_AI, &bldoutsc, aiptr );

	lineno = 3;

	if ( model == M_SMALL ) {
		grptr = dfnscngrp( AIDFNGRP, iflag ? NULL : dataorg, NULL, NULL );
		copy( grptr->dfnscn.ainame, "*group*", 7);

		lineno = 4;
		aiptr = dfnscngrp( AIDFNSCN, NULL, NULL, NULL );
	} else {
		aiptr = dfnscngrp( AIDFNSCN, NULL, dataorg, NULL);
	}
	aiptr->dfnscn.aisctype = STYP_DATA;
	copy( aiptr->dfnscn.ainame, _DATA, 8 );
	aiptr->dfnscn.aifill = 0;
	aiptr->dfnscn.aifillfg = 0;

	if ( model == M_SMALL )
		listadd( l_AI, &grptr->dfnscn.sectspec, aiptr );
	else
		listadd( l_AI, &bldoutsc, aiptr );

	lineno = (model == M_SMALL) ? 5: 4;
	aiptr = dfnscngrp( AIDFNSCN, NULL, NULL, NULL );
	aiptr->dfnscn.aisctype = STYP_BSS;
	copy( aiptr->dfnscn.ainame, _BSS, 8 );
	aiptr->dfnscn.aifill = 0;
	aiptr->dfnscn.aifillfg = 0;

	if ( model == M_SMALL) {
		listadd( l_AI, &grptr->dfnscn.sectspec, aiptr );
		listadd( l_AI, &bldoutsc, grptr );
	} else 
		listadd( l_AI, &bldoutsc, aiptr );

	return;
#endif
}

void
procspecobj(fdes, filename)
LDFILE *fdes;
char *filename;
{
	/*
	 * No more additional files to check
	 */

#if !iAPX286
	if ( (TYPE (fdes) == IAPX16) || (TYPE (fdes) == IAPX20) )
		lderror(1, 0, filename,
			"non-tv file %s in transfer vector run",
			filename);
	else if ( (TYPE (fdes) == IAPX16TV) || (TYPE (fdes) == IAPX20TV) )
		lderror(1, 0, filename,
			"tv file %s in non transfer vector run",
			filename);
	else
#endif
		lderror(1, 0, filename,
			"file %s is of unknown type: magic number = %06.1x",
			filename, TYPE(fdes));
}

void
adjsize(osp)
OUTSECT *osp;
{
	/*
	 * No special cases so no need to adjust size of output section
	 */
}

void
adjaout(aout)
AOUTHDR *aout;
{
	/*
	 * no special additions to a.out header needed for iapx
	 */
}

void
relocate(ifd, infl, isp, fdes, rdes, sect_buf, buffer_size)
register FILE *ifd;
INFILE *infl;
INSECT *isp;
FILE *fdes, *rdes;
register char *sect_buf;
long buffer_size;
{
	long vaddiff;
	register long rdif;
	RELOC rentry, aentry;
	register SLOTVEC *svp;
	long sect_size, chunk_size, bytes_so_far;
	long byte_offset, indx;
	int reloc_read;
	int auxflag = 0;		/* indicates special R_AUX entry */

	union
	{
		unsigned short u[2];
		char	 c[4];
	} value;

	vaddiff = isp->isnewvad - isp->ishdr.s_vaddr;
	sect_size = isp->ishdr.s_size;
	chunk_size = min( sect_size, buffer_size );
	if (fread( sect_buf, (int)chunk_size, 1, ifd ) != 1)
		lderror( 2, 0, NULL, "cannot read section %.8s of file %s",
			isp->ishdr.s_name, infl->flname );
	sect_size -= chunk_size;
	bytes_so_far = 0;
	fseek( ifd, isp->ishdr.s_relptr + infl->flfiloff, 0 );

	for ( reloc_read = 1; reloc_read <= isp->ishdr.s_nreloc; reloc_read++ )
	{
		if (fread( &rentry, RELSZ, 1, ifd ) != 1)
			lderror( 2, 0, NULL, "cannot read relocation entries of section %.8s of %s",
				isp->ishdr.s_name, infl->flname );

		if ((svp = svread( rentry.r_symndx)) == NULL)
		{
			lderror(1, 0, NULL, "relocation entry found for non-relocatable symbol in %s",
				infl->flname);
			continue;
		}

		if (rentry.r_type == R_ABS)
			continue;

		rdif = svp->svnvaddr - svp->svovaddr;
		byte_offset = rentry.r_vaddr - isp->ishdr.s_vaddr;

		if (byte_offset < bytes_so_far){
			if(chunk_size != isp->ishdr.s_size)
				lderror(2,0,NULL, "Reloc entries out of order in section %.8s of file %s",
					isp->ishdr.s_name, infl->flname);
			else
				lderror(1,0,NULL, "Reloc entries out of order in section %.8s of file %s",
					isp->ishdr.s_name, infl->flname);
			}
		while (byte_offset > bytes_so_far + chunk_size)
		{
			bytes_so_far += chunk_size;
			fwrite( sect_buf, (int)chunk_size, 1, fdes );
			chunk_size = min( sect_size, buffer_size );
			fseek( ifd, isp->ishdr.s_scnptr + bytes_so_far + infl->flfiloff, 0 );
			if (fread( sect_buf, (int)chunk_size, 1, ifd ) != 1)
				lderror( 2, 0, NULL, "cannot read section %.8s of %s", isp->ishdr.s_name, infl->flname );
			sect_size -= chunk_size;
			fseek( ifd, infl->flfiloff + isp->ishdr.s_relptr + reloc_read * RELSZ, 0);
		}

		if ((byte_offset + 4 > bytes_so_far + chunk_size) && sect_size)
				/* 4 bytes is the address size, must be sure all
				   address is in buffer */
		{
			bytes_so_far += chunk_size - 4;
			fwrite( sect_buf, (int)chunk_size - 4, 1, fdes );
			sect_buf[0] = sect_buf[chunk_size - 4];
			sect_buf[1] = sect_buf[chunk_size - 3];
			sect_buf[2] = sect_buf[chunk_size - 2];
			sect_buf[3] = sect_buf[chunk_size - 1];
			chunk_size = min( sect_size, buffer_size  - 4);
			fseek( ifd, isp->ishdr.s_scnptr + bytes_so_far + 4 + infl->flfiloff, 0 );
			if (fread( sect_buf + 4, (int)chunk_size, 1, ifd ) != 1)
				lderror( 2, 0, NULL, "cannot read section %.8s of %s", isp->ishdr.s_name, infl->flname );
			sect_size -= chunk_size;
			chunk_size +=4;
			fseek( ifd, infl->flfiloff + isp->ishdr.s_relptr + reloc_read * RELSZ, 0);
		}

		indx = byte_offset - bytes_so_far;
		value.c[0] = sect_buf[indx];
		value.c[1] = sect_buf[indx + 1];

		switch( rentry.r_type ) {

		case R_REL16:
			/*
			 * PC relative addressing:
			 *	1. Adjust for movement of the source instructon
			 *	2. Adjust for movement of the target
			 */
			value.u[0] = SWAPB2( SWAPB2(value.u[0]) +
					(rdif - vaddiff) );
			break;
		case R_DIR16:
			value.u[0] = SWAPB2( SWAPB2(value.u[0]) + rdif );
			break;
		case R_IND16: 
			/*
			 * Transfer Vector address
			 */
			if( ((svp->svflags & (SV_TV | SV_ERR)) == 0)  &&  aflag ) {
				svp->svflags |= SV_ERR;
				lderror(1,0,NULL,tvreferr,
					rentry.r_vaddr, rentry.r_symndx, infl->flname);
				}
			else
				value.u[0] = SWAPB2(svp->svnvaddr);
			break;
		case R_OFF8:		/* IAPX20 */
			value.c[0] = (((unsigned) value.c[0]) - 
					(((unsigned) svp->svovaddr) & 0xff) +
					(((unsigned) svp->svnvaddr) & 0xff)) & 0xff;
			break;
			
		case R_OFF16:		/* IAPX20 */
			value.u[0] = SWAPB2( (SWAPB2(value.u[0]) -
					(((unsigned) svp->svovaddr) & 0xff) +
					(((unsigned) svp->svnvaddr) & 0xff)));
			break;
			
		case R_SEG12:		/* IAPX20 */
#if iAPX286
			value.u[0] = SWAPB2( (unsigned short) ((unsigned long) svp->svnvaddr >> 16 )); /* change casts */
#else
			if (fread( &aentry, RELSZ, 1, ifd ) != 1)
				lderror( 2, 0, NULL, 
			    "cannot read relocation entries of section %.8s of %s",
						isp->ishdr.s_name, infl->flname );
			if( aentry.r_type == R_AUX ) {
				reloc_read++;
				auxflag++;
				rdif = aentry.r_vaddr;
			}
			else {
				fseek( ifd, - (long) RELSZ, 1 );
				rdif = 0L;
			}
			value.u[0] = SWAPB2(((unsigned) ((svp->svnvaddr + rdif) >> 4))
					& (~ 0xf));
#endif
			break;

		}

		/* put the relocated address back in the buffer */
		sect_buf[indx] = value.c[0];
		sect_buf[indx + 1] = value.c[1];

		/*
		 * Preserve relocation entries
		 */
		if( rflag )
		{
			rentry.r_vaddr += vaddiff;
			rentry.r_symndx = svp->svnewndx;
			fwrite(&rentry, RELSZ, 1, rdes);
			
			/*
			 * Output, if present, the aux entry
			 * associated with a R_SEG12
			 */
			
			if( auxflag ) {
				auxflag = 0;
				aentry.r_symndx = svp->svnewndx;
				fwrite(&aentry, RELSZ, 1, rdes);
			}
		}
	}

	while (sect_size){
		bytes_so_far += chunk_size;
		fwrite( sect_buf, (int)chunk_size, 1, fdes );
		chunk_size = min( sect_size, buffer_size );
		fseek( ifd, infl->flfiloff + isp->ishdr.s_scnptr + bytes_so_far, 0 );
		if (fread( sect_buf, (int)chunk_size, 1, ifd ) != 1)
			lderror( 2, 0, NULL, "cannot read section %.8s of %s", isp->ishdr.s_name, infl->flname );
		sect_size -= chunk_size;
		}
	fwrite( sect_buf, (int)chunk_size, 1, fdes );

}

unsigned short
swapb2(val)

unsigned short val;
{
#if AR32W

	unsigned short result;

	/*
	 * reorder the bytes of a short, converting to/from DEC/8086 format
	 */

	result = ( val >> 8 ) & 0x00ff;
	result |= ( (val << 8) & 0xff00 );
	return( result );
#endif
}

#if TRVEC
void
setslot(pglob, psect, outslot)
SYMTAB *pglob;
SCNHDR *psect;
TVENTRY *outslot;
{
	psect = &pglob->smscnptr->isoutsec->oshdr;
	if( IAPX16MAGIC(magic) ) {
		outslot->tv_cs = (unsigned short) ((psect->s_paddr - psect->s_vaddr) >> 4);
		outslot->tv_ip = (unsigned short) (pglob->smnewval - ((long) outslot->tv_cs << 4));
	}else{
		outslot->tv_cs = (unsigned short) ((pglob->smnewval & (~0xff)) >> 4);
		outslot->tv_ip = (unsigned short) (pglob->smnewval & 0xff);
	}
}

void
slotassign(tvent, pglob, cs)
TVENTRY *tvent;
SYMTAB *pglob;
unsigned short *cs;
{
	SCNHDR *psect;

	psect = &pglob->smscnptr->isoutsec->oshdr;

	if( IAPX16MAGIC(magic) ) {
		*cs = (unsigned short) ((psect->s_paddr - psect->s_vaddr) >> 4);
		tvent->tv_cs = *cs;
		tvent->tv_ip = (unsigned short)(pglob->smnewval - ((long) *cs << 4));
	}else{
		tvent->tv_cs = (unsigned short) ((pglob->smnewval & (~0xff)) >> 4);
		tvent->tv_ip = (unsigned short) (pglob->smnewval & 0xff);
	}
#if AR32W
	tmpslot.u[0] = tvent->tv_ip;
	tmpslot.u[1] = tvent->tv_cs;
	tmp_cs = tmpslot.c[0];
	tmpslot.c[0] = tmpslot.c[1];
	tmpslot.c[1] = tmp_cs;
	tmp_cs = tmpslot.c[2];
	tmpslot.c[2] = tmpslot.c[3];
	tmpslot.c[3] = tmp_cs;
	tvent->tv_ip = tmpslot.u[0];
	tvent->tv_cs = tmpslot.u[1];
#endif

}

void
settventry(tvent, psym, cs)
TVENTRY *tvent;
struct syment *psym;
unsigned short *cs;
{
	if(IAPX16MAGIC(magic)) {
		tvent->tv_cs = *cs;
		tvent->tv_ip = (unsigned short)(psym->n_value - ((long) *cs << 4));
	}else{
		tvent->tv_cs= (unsigned short) ((psym->n_value & (~0xff)) >> 4);
		tvent->tv_ip= (unsigned short) (psym->n_value & 0xff);
	}
#if AR32W
	tmpslot.u[0] = tvent->tv_ip;
	tmpslot.u[1] = tvent->tv_cs;
	tmp_cs = tmpslot.c[0];
	tmpslot.c[0] = tmpslot.c[1];
	tmpslot.c[1] = tmp_cs;
	tmp_cs = tmpslot.c[2];
	tmpslot.c[2] = tmpslot.c[3];
	tmpslot.c[3] = tmp_cs;
	tvent->tv_ip = tmpslot.u[0];
	tvent->tv_cs = tmpslot.u[1];
#endif

}

void
filltvassign(psym)
SYMTAB *psym;
{

			struct scnhdr *psect;

	if(IAPX16MAGIC(magic)) {
		psect = &psym->smscnptr->isoutsec->oshdr;
		tvspec.tvfill.tv_cs = (unsigned short) ((psect->s_paddr - psect->s_vaddr) >> 4);
		tvspec.tvfill.tv_ip = (unsigned short)(psym->smnewval - ((long) tvspec.tvfill.tv_cs << 4));
	}else{
		tvspec.tvfill.tv_cs = (unsigned short) ((psym->smnewval & (~0xff)) >> 4);
		tvspec.tvfill.tv_ip = (unsigned short) (psym->smnewval & 0xff);
	}
}
#endif
