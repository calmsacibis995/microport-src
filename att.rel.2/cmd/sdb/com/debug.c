/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
	/*  debug.c: 1.8 7/18/83 */

#include "head.h"
#include "coff.h"
/*
 * debugging for sdb
 */
extern MSG		NOPCS;

prstentry(stp)	/* debug */
register struct syment *stp;
{
#if DEBUG > 1
	fprintf( FPRT2, "debug: print symbol table entry:\n" );
	fprintf( FPRT2, "	n_name=  %s;\n", stp->n_name );
	fprintf( FPRT2, "	n_value= %#lx;\n", stp->n_value );
	fprintf( FPRT2, "	n_scnum= %d;\n", (int)stp->n_scnum );
	fprintf( FPRT2, "	n_type=  %#x;\n", (int)stp->n_type );
	fprintf( FPRT2, "	n_sclass=%d;\n", (int)stp->n_sclass );
	fprintf( FPRT2, "	n_numaux=%d;\n", (int)stp->n_numaux );
#endif
}

prprocaux(axp, name)	/* debug */
AUXENT *axp;
char *name;
{
#if DEBUG > 1
#define AXA axp->x_sym
#define AXF	AX.x_fcnary
	fprintf(FPRT2, " proc auxent for %s:\n", name);
	fprintf(FPRT2, "	x_tagndx= 0x%8.8lx\n", AXA.x_tagndx);
	fprintf(FPRT2, "	x_fsize=  0x%8.8lx\n", AXA.x_misc.x_fsize);
#if !iAPX286
	fprintf(FPRT2, "	x_lnnoptr=0x%8.8lx\n", AXF.x_fcn.x_lnnoptr);
	fprintf(FPRT2, "	x_endndx= 0x%8.8lx\n", AXF.x_fcn.x_endndx);
#endif
#endif
}

prprstk()	/* debug */
{
#if DEBUG > 1
	register int n, i;
#if vax || u3b 
	fprintf(FPRT2, "((struct user *)uu)->u.ar0 = %#o\n",
		((struct user *)uu)->u_ar0);
#else
#if iAPX286
	fprintf(FPRT2, "((struct user *)uu)->u.ar0 = %#lx\n",
		((struct user *)uu)->u_ar0);
#else
#if u3b5
	readregs();
#endif
#endif
#endif
	fprintf(FPRT2, "debug process stack:\n");
	if( pid == 0 && PC < 0 )
	{
		error( NOPCS );
		return( -1 );
	}
	else
	{
#if u3b5
		fprintf(FPRT2, "	SDBREG(PC) = %#o\n", regvals[PC]);
#else
#if iAPX286
		fprintf(FPRT2, "        SDBREG(PC) = %#lx\n", PCC);
#else
		fprintf(FPRT2, "	SDBREG(PC) = %#o\n", SDBREG(PC));
#endif
#endif
	}

#if u3b
	fprintf(FPRT2, "	SDBREG(PS) = %#o\n", SDBREG(PS));
	fprintf(FPRT2, "	SDBREG(PSBR) = %#o\n", SDBREG(PSBR));
	fprintf(FPRT2, "	SDBREG(SSBR) = %#o\n", SDBREG(SSBR));
#else
#if vax
	fprintf(FPRT2, "	SDBREG(PS) = %#o\n", SDBREG(PS));
#else
#if u3b5
	for (i = 0; i <= 12; i++)
		fprintf(FPRT2,"%s = 0x%8.8lx\n",reglist[i].rname,regvals[reglist[i].roffs]);
#endif
#endif
#endif
#endif
}
