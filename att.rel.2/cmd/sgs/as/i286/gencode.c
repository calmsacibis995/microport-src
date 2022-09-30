/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)gencode.c	1.3 - 85/08/08 */
#include <stdio.h>
#include <symbols.h>
#include <expand.h>
#include "systems.h"
#include "instab.h"
#include "parse.h"
#include "gendefs.h"
#include "expand2.h"

/*
 *	"gencode.c" contains the code generation routines for the
 *	first pass of the Basic-16 Assembler. These routines are
 *	called by "yyparse" to determine optimizations, new opcodes,
 *	or confirm proper use of an instruction. Since this is the
 *	only general purpose file in pass one, the flag parsing
 *	routine, "flags", is here.
 */

/*
 *	To understand the generation of the instruction bit pattern
 *	it is necessary to note the instruction format for the 8086.
 *
 *	+------+-+-+--+---+---+--------+--------+--------+--------+
 *	|  op  |d|w|  |reg|r/m| offset | offset | immed  | immed  |
 *	| code | | |  |   |   | addr   | (w==1) |  data  |  data  |
 *	+------+-+-+--+---+---+--------+--------+--------+--------+
 *		    ^
 *		   mod
 *
 *	opcode	This field is used exclusively to represent the opcode
 *		for the instruction. The complete opcode for an instruction
 *		may be spread across other field, i.e. "d", "w", and "reg".
 *
 *	d	This is the direction bit for dyadic instructions. The 
 *		direction means either "to" or "from" a register. In
 *		general, all dyadic instructions must use a register as
 *		one of the operands.
 *
 *	w	This is the word bit, 1 denotes a word oriented instruction
 *		and 0 denotes a byte instruction.
 *
 *	mod	This is the mode field. It specifies how the "r/m" field
 *		is used in locating the operand. For example, mod==10
 *		and r/m==110 means 16-bit offset indirect through the "bp"
 *		base register.
 *
 *	reg	This field contains the register operand in a dyadic
 *		instruction or opcode information in some optimized
 *		instruction forms and monadic instructions.
 *
 *	r/m	This is the register/memory field. This conatins either
 *		the register designation or the base register and/or
 *		the index register designation. The only exception is
 *		mod==00 and r/m==110 which indicates 16-bit external
 *		addressing mode.
 *
 *	The opcode is always stored in the "opcode" field of the
 *	instruction table entry. Note that if the reg field or the
 *	"d" bit is unknown then that field is zero in the "opcode" field of
 *	the instruction. The variable "dbit" is set to the proper
 *	value once there is enough information that its value can
 *	be ascertained. The "mod" (addressing mode), "reg" (register),
 *	and "r/m" (register/memory) fields contain all addressing mode
 *	information for the instruction. This is possible since some
 *	optimizations and immediate addressing mode are represented by
 *	different opcodes. All of the "mod", "reg", and "r/m" information
 *	is kept in "memloc". Therefore to generate a complete opcode
 *	and its associated addressing mode it may be necessary to
 *	logical 'or' the "opcode", "dbit", and "memloc" fields.
 *
 *	Immediate data always is generated last, after the address offset
 *	if it is present.
 *
 */

extern char
#if !ONEPROC
	*xargp[],
#endif
	newname[];

extern short
	argindex,
	opt,	/* optimize flag */
	localopt;	/* local optimize flag */

extern long
	newdot;		/* up-to-date value of "." */

extern symbol
	*dot;		/* current location counter */

extern short
	nameindx,
	dbit;

extern upsymins
	newins;

extern upsymins
	*lookup();

extern union addrmdtag
	memloc;

#if !iAPX286
short	xflag = NO;
#endif

unsigned short
actreloc(type,deflt)
	register short type;
	int deflt;
{
#if !iAPX286
	if (xflag && (type & X86TYPE))
#else
	if (type & X86TYPE)
#endif
		return((unsigned short)((type & LO8TYPE) ? LOW8BITS :
			((type & LO16TYPE) ? LO16BITS : HI12BITS)));
	else
		return((unsigned short)deflt);
} /* actreloc */

/* address generation */

addrgen(opcd,opnd1)
	long opcd;
	register addrmode *opnd1;
{
	register long val;

	val = opnd1->adexpr.expval;
	if (opt && (memloc.addrmd.mod == DISP16) &&
		(opnd1->adexpr.symptr == NULLSYM)) {
		/* attempt to optimize for */
		/* 16-bit absolute displacement */
		if ((val >= -128L) && (val <= 127L)) {
#if !iAPX286
			if (xflag &&
#else
			if (
#endif
				(opnd1->adexpr.exptype & HI12TYPE))
				goto out;
			memloc.addrmd.mod = DISP8;
			generate(16,NOACTION,(long)(opcd | memloc.addrmdfld),NULLSYM);
			generate(8,NOACTION,val,NULLSYM);
			return;
		}
	}
out:	generate(16,NOACTION,(long)(opcd | memloc.addrmdfld),NULLSYM);
	if (opnd1->admode & EXPRMASK) {
		generate(16,actreloc(opnd1->adexpr.exptype,SWAPB),
			val,opnd1->adexpr.symptr);
	}
}

/* loop instructions */

loopgen(insptr,opnd1)
	register instr *insptr;
	register addrmode *opnd1;
{
	register symbol *sym;
	long val;

	sym = opnd1->adexpr.symptr;
	val = opnd1->adexpr.expval;
	if (opt && localopt) {
		switch (shortsdi(sym,val,LOOPI)) {
			case S_SDI:
				generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
				generate(8,RELPC8,val,sym);
				return;
			case U_SDI:	/* don't know, generate short and flag */
				generate(insptr->nbits,LOOPOPT,insptr->opcode,NULLSYM);
				generate(8,RELPC8,val,sym);
				return;
			/* case L_SDI: fall through */
		}
	}
	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
	generate(8,NOACTION,0x02L,NULLSYM);
	generate(8,NOACTION,0xebL,NULLSYM);
	generate(8,NOACTION,0x03L,NULLSYM);
	generate(8,NOACTION,0xe9L,NULLSYM);
	generate(16,LONGREL,val,sym);
}

/* short conditional jmp */

jmp1opgen(insptr,opnd1)
	register instr *insptr;
	register addrmode *opnd1;
{
	register symbol *sym;
	long val;

	sym = opnd1->adexpr.symptr;
	val = opnd1->adexpr.expval;
	if (opt && localopt) {
		switch (shortsdi(sym,val,CJMP)) {
			case S_SDI: /* guaranteed to fit */
				generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
				generate(8,RELPC8,val,sym);
				return;
			case U_SDI: /* don't know, so generate short and flag */
				generate(insptr->nbits,CJMPOPT,insptr->opcode,NULLSYM);
				generate(8,RELPC8,val,sym);
				return;
			/* case L_SDI: fall through */
		} /* switch */
	}
	generate(insptr->nbits,NOACTION,insptr->opcode ^ 0x01L,NULLSYM);
	generate(8,NOACTION,3L,NULLSYM);
	generate(8,NOACTION,0xe9L,NULLSYM);
	generate(16,LONGREL,val,sym);
} /* jmpopgen */

/* short unconditional jmp */

jmp2opgen(insptr,opnd1)
	register instr *insptr;
	register addrmode *opnd1;
{
	register symbol *sym;
	long val;

	val = opnd1->adexpr.expval;
	sym = opnd1->adexpr.symptr;
	if (opt && localopt) {
		switch (shortsdi(sym,val,UJMP)) {
			case S_SDI:
				generate(insptr->nbits,NOACTION,(long)((short)(insptr->opcode) | 0x02),NULLSYM);
				generate(8,RELPC8,val,sym);
				return;
			case U_SDI:	/* don't know, generate short and flag */
				generate(insptr->nbits,UJMPOPT,(long)((short)(insptr->opcode) | 0x02),NULLSYM);
				generate(8,RELPC8,val,sym);
				return;
			/* case L_SDI: fall through */
		}
	}
	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
	generate(16,LONGREL,val,sym);
}

defgen(iname,opnd1)
	char *iname;
	addrmode *opnd1;
{
	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'N';
	newname[nameindx] = '\0';
	newins = *lookup(newname,N_INSTALL,MNEMON);
	if (newins.itp != NULL) {
		addrgen(newins.itp->opcode,opnd1);
	}
	else
		aerror("Cannot find deferred instr");
}

/* special optimization for some register monadics */

rmongen(insptr,opnd1)
	instr *insptr;
	register addrmode *opnd1;
{
	if (opt && (opnd1->admode & REGMASK)) {
		/* optimize register handling */
		nameindx = copystr(insptr->name,newname);
		newname[nameindx++] = 'R';
		newname[nameindx] = '\0';
		newins = *lookup(newname,N_INSTALL,MNEMON);
		if (newins.itp != NULL) {
			generate(newins.itp->nbits,NOACTION,(long)(newins.itp->opcode | opnd1->adreg),NULLSYM);
		}
		else
			aerror("Cannot find instr");
		return;
	}
	addrgen(insptr->opcode,opnd1);
}

/* generate regular mov instruction */

movgen(insptr,opnd1)
	instr *insptr;
	register addrmode *opnd1;
{
	register long opcd;

	if (	opt &&
		(memloc.addrmd.reg == AREGOPCD) &&
		(opnd1->admode == EXADMD)) {
		/* optimize for accumulator and */
		/* the external address mode */
		nameindx = copystr(insptr->name,newname);
		newname[nameindx++] = 'A';
		newname[nameindx] = '\0';
		newins = *lookup(newname,N_INSTALL,MNEMON);
		if (newins.itp != NULL) {
			opcd = newins.itp->opcode;
			if (dbit == DBITOFF)
				opcd |= 0x02L;
			generate(newins.itp->nbits,NOACTION,opcd,NULLSYM);
			generate(16,actreloc(opnd1->adexpr.exptype,SWAPB),
				opnd1->adexpr.expval,opnd1->adexpr.symptr);
		}
		else
			aerror("Cannot find mov instr");
		return;
	}
	addrgen((long)(insptr->opcode | dbit),opnd1);
}

/* mov immediate */

mov2gen(iname,opnd2)
	char *iname;
	addrmode *opnd2;
{
	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	if (opt && (opnd2->admode & REGMASK)) {

		/* optimize for register */

		newname[nameindx++] = 'R';
		newname[nameindx] = '\0';
		newins = *lookup(newname,N_INSTALL,MNEMON);
		if (newins.itp != NULL) {
			generate(newins.itp->nbits,NOACTION,(long)((short)(newins.itp->opcode) | opnd2->adreg),NULLSYM);
		}
		else
			aerror("Cannot find mov instr");
		return;
	}

	/* general memory reference */

	newname[nameindx++] = 'G';
	newname[nameindx] = '\0';
	newins = *lookup(newname,N_INSTALL,MNEMON);
	if (newins.itp != NULL) {
		addrgen(newins.itp->opcode,opnd2);
	}
	else
		aerror("Cannot find mov instr");
}

/* immediate with accumulator for dyadic instructions */

dyad1gen(iname)
	char *iname;
{
	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'A';
	newname[nameindx] = '\0';
	newins = *lookup(newname,N_INSTALL,MNEMON);
	if (newins.itp != NULL) {
		generate(newins.itp->nbits,NOACTION,newins.itp->opcode,NULLSYM);
	}
	else
		aerror("Cannot find dyadic (acc)");
}

/* immediate with general memory */

dyad2gen(iname,opnd2)
	char *iname;
	addrmode *opnd2;
{
	if (opt && (opnd2->admode & AREGMASK)) {
		/* optimize for accumulator */
		dyad1gen(iname);
		dbit = DBITOFF;
		return;
	}

	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'G';
	newname[nameindx] = '\0';
	newins = *lookup(newname,N_INSTALL,MNEMON);
	if (newins.itp != NULL) {
		addrgen((long)(newins.itp->opcode | dbit),opnd2);
	}
	else
		aerror("Cannot find dyad");
}

/* xchg register with accumulator */

xchgopt(iname,reg)
	char *iname;
	char reg;
{
	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'A';
	newname[nameindx] = '\0';
	newins = *lookup(newname,N_INSTALL,MNEMON);
	if (newins.itp != NULL) {
		generate(8,NOACTION, (newins.itp->opcode | (long)reg),NULLSYM);
	}
	else
		aerror("Cannot find xchg instr");
}

/* test immediate with general memory */

testgen(iname,opnd2)
	char *iname;
	addrmode *opnd2;
{
	if (opt && (opnd2->admode & AREGMASK)) {
		/* optimize for accumulator */
		dyad1gen(iname);
		return;
	}
	nameindx = copystr(iname,newname);
	newname[nameindx++] = 'I';
	newname[nameindx++] = 'G';
	newname[nameindx] = '\0';
	newins = *lookup(newname,N_INSTALL,MNEMON);
	if (newins.itp != NULL) {
		addrgen(newins.itp->opcode,opnd2);
	}
	else
		aerror("Cannot find test");
}

/* optimize test to work with only one memory operand */

testopt(iname,opnd1,sflag)
	char *iname;
	addrmode *opnd1;
	short sflag;
{
	nameindx = copystr(iname,newname);
	if (opnd1->admode & REGMASK) {
		newname[nameindx++] = 'S';
		newname[nameindx] = '\0';
		newins = *lookup(newname,N_INSTALL,MNEMON);
		if (newins.itp != NULL) {
			memloc.addrmd.reg = opnd1->adreg;
			generate(newins.itp->nbits,NOACTION,(long)(newins.itp->opcode | memloc.addrmdfld),NULLSYM);
		}
		else
			aerror("Cannot find test opt");
	}
	else {
		newname[nameindx++] = 'I';
		newname[nameindx++] = 'G';
		newname[nameindx] = '\0';
		newins = *lookup(newname,N_INSTALL,MNEMON);
		if (newins.itp != NULL) {
			addrgen(newins.itp->opcode,opnd1);
			generate((sflag) ? 8 : 16,NOACTION,-1L,NULLSYM);
		}
		else
			aerror("Cannot find test (opt) immed");
	}
}

strgen(insptr,segptr,ireg)
	instr *insptr;
	instr *segptr;
	char *ireg;
{
	if (*ireg != 's')
		yyerror("Must specify si for source");
	generate(8,NOACTION,(long)(SEGPFX | ((short)(segptr->opcode) << 3)),NULLSYM);

	/*
	 *	don't output seg prefix fo %ds: (this is the default)
	 */

	if(segptr->opcode != 3)
		generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
}

str1gen(insptr,ireg1,ireg2)
	instr *insptr;
	char *ireg1, *ireg2;
{
	if (*ireg1 != 's')
		yyerror("Must specify si for source");
	if (*ireg2 != 'd')
		yyerror("Must specify di for dest");
	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
}

str3gen(insptr,ireg1,ireg2)
instr *insptr;
char  *ireg1;
char  *ireg2;
{
	if(*ireg1 != 's')
		yyerror("Must specify si for source");
	if (*ireg2 != 'd')
		yyerror("Must specify dx for dest");
	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
}
str4gen(insptr,ireg1,ireg2)
instr *insptr;
char  *ireg1;
char  *ireg2;
{
	if(*ireg1 != 'd')
		yyerror("Must specify dx for source");
	if (*ireg2 != 'd')
		yyerror("Must specify di for dest");
	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
}

str2gen(insptr,ireg)
	instr *insptr;
	char *ireg;
{
	if (*ireg != 'd')
		yyerror("Must specify di for dest");
	generate(insptr->nbits,NOACTION,insptr->opcode,NULLSYM);
}

immedgen(nbits,immed)
	register short nbits;
	register addrmode *immed;
{
	register short type;

#if !iAPX286
	if (xflag && (type = immed->adexpr.exptype & X86TYPE)) {
#else
	if (type = immed->adexpr.exptype & X86TYPE) {
#endif
		if ((nbits == 8) && (type & HI12TYPE))
			yyerror("Cannot request 12 bits in 8 bit expr");
		generate(nbits, (unsigned short)((type & LO8TYPE) ?
			LOW8BITS :
			((type & LO16TYPE) ? LO16BITS : HI12BITS)),
			immed->adexpr.expval, immed->adexpr.symptr);
		return;
	}
	generate(nbits, (unsigned short)((nbits == 8) ?
		RESABS : SWAPB),
		immed->adexpr.expval, immed->adexpr.symptr);
}

flags(ch)
	register char ch;
{
	char errmsg[28];

#if !iAPX286
	if (ch == 'x') {
		xflag = YES;
#if !ONEPROC
		xargp[argindex++] = "-x";
#endif
	}
#else
	if (ch == 'M')
		{}
#endif
	else {
		sprintf(errmsg,"Illegal flag (%c) - ignored",ch);
		werror(errmsg);
	}
}

copystr(str1,str2)
	register char *str1, *str2;
{
	register int n = 0;

	while (*str2++ = *str1++) ++n;
	return(n);
}

/* address generation */

laddrgen(opcd,opnd1)
long opcd;
register addrmode *opnd1;
{
	register long val;

/*
	val = opnd1->adexpr.expval ;
    	generate(24,NOACTION,(long)(opcd | memloc.addrmdfld),NULLSYM);
	generate(16,actreloc(opnd1->adexpr.exptype,SWAPB),
	    val,opnd1->adexpr.symptr);
*/

	val = opnd1->adexpr.expval;
	if (opt && (memloc.addrmd.mod == DISP16) &&
		(opnd1->adexpr.symptr == NULLSYM)) {
		/* attempt to optimize for */
		/* 16-bit absolute displacement */
		if ((val >= -128L) && (val <= 127L)) {
#if !iAPX286
			if (xflag &&
#else
			if (
#endif
				(opnd1->adexpr.exptype & HI12TYPE))
				goto out;
			memloc.addrmd.mod = DISP8;
			generate(24,NOACTION,(long)(opcd | memloc.addrmdfld),NULLSYM);
			generate(8,NOACTION,val,NULLSYM);
			return;
		}
	}
out:	generate(24,NOACTION,(long)(opcd | memloc.addrmdfld),NULLSYM);
	if (opnd1->admode & EXPRMASK) {
		generate(16,actreloc(opnd1->adexpr.exptype,SWAPB),
			val,opnd1->adexpr.symptr);
	}
}

/*
**	genstring
**
**	generate string initilisation
**
*/

genstring ( p )
char *p ;
{
	int c;
	int val;

	while( (c = *p++) ){
		switch( c ) {

		case '\\':
			switch( c = *p++ ){

			case '\n':
				continue;

			default:
				val = c;
				goto mkcc;

			case 'n':
				val = '\n';
				goto mkcc;

			case 'r':
				val = '\r';
				goto mkcc;

			case 'b':
				val = '\b';
				goto mkcc;

			case 't':
				val = '\t';
				goto mkcc;

			case 'f':
				val = '\f';
				goto mkcc;

			case 'v':
				val = '\013';
				goto mkcc;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				val = c-'0';
				c = *p++;  /* try for 2 */
				if( (c>='0') && (c<='7') ){
					val = (val<<3) | (c-'0');
					c = *p++;  /* try for 3 */
					if((c>='0') && (c<='7') ){
						val = (val<<3) | (c-'0');
						}
					else --p;
					}
				else --p;

				goto mkcc;

				}
		default:
			val =c;
		mkcc:
			generate ( 8 , NOACTION , (long) val , NULLSYM) ;
			continue;
			}
		break;
		}
	/* end of string or  char constant */

	/*
	 * and , finally, a NULL character to mark the end of the string
	 */

	generate ( 8 , NOACTION , (long) 0 , NULLSYM) ;

}
