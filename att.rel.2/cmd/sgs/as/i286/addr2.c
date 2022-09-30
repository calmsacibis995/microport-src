/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)addr2.c	1.3 - 85/08/08 */
#include <stdio.h>
#include <reloc.h>
#include <storclass.h>
#include <syms.h>
#include <symbols.h>
#include <codeout.h>
#include "systems.h"
#include "gendefs.h"
#include "instab.h"

extern BYTE
	*longsdi;

extern unsigned short
	relent;

#if !iAPX286
#if !ONEPROC
short	xflag = NO;
#else
extern short	xflag;
#endif
#endif

extern short
	transvec,
	filedef;

extern long
	newdot;

extern FILE
	*fdrel,
	*fdcode;

extern symbol
	*dot;

extern SYMENT
	sment;

extern symbol
	symtab[];

extern upsymins
	*lookup();

typedef struct {
	char lobyte;
	char hibyte;
} bytes;

static short
	currbit = 0;

static unsigned short
	bitfield = 0;

relocat(sym,rtype)
	register symbol *sym;
	int rtype;
{
	register short stype;
	register char *rsym;
	prelent trelent;

	stype = sym->styp & TYPE;	/* guaranteed a symbol is */
					/* there by swapb and longrel */
	switch (stype) {

		case ABS:
			return;
		case TXT:
			rsym = ".text";
			break;
		case DAT:
			rsym = ".data";
			break;
		case BSS:
			rsym = ".bss";
			break;
		case UNDEF:
			sym->styp |= EXTERN; /* make sure it gets in symbol table */
			rsym = sym->_name.name;
			break;
		default:
			aerror("Invalid type");
	}

	trelent.relval = newdot;
	trelent.relname = rsym;
	trelent.reltype = (short) rtype;
	fwrite((char *)(&trelent),sizeof(prelent),1,fdrel);
	++relent;
} /* relocat */

symrel(rsym,rtype)
	symbol *rsym;
	int rtype;
{
	prelent trelent;

	if ((rsym->styp & TYPE) == UNDEF)
		/* leave statics alone but */
		/* if symbol not defined here */
		/* then declare it global */
		rsym->styp |= EXTERN;
	else if ((rsym->styp & EXTERN) == 0)
		rsym->styp |= HI12TYPE; /* goes in object file symbol table */
	trelent.relval = newdot;
	trelent.relname = rsym->_name.name;
	trelent.reltype = (short) rtype;
	fwrite((char *)(&trelent),sizeof(prelent),1,fdrel);
	relent++;
}

cjmpopt(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	long	opcd;

	opcd = code->cvalue;
	if (fread((char *)code,sizeof(*code),1,fdcode) != 1)
		aerror("Unexpected EOF on temporary (code) file");
	sym = symtab + (code->cindex - 1);	/* guaranteed a symbol is */
						/* there by shortsdi in pass 1 */
	if (*++longsdi) {
		opcd ^= 0x01L;
		codgen(8,opcd);
		codgen(8,3L);
		codgen(8,0xe9L);
		/* codout (which called this routine) will call codgen	*/
		/* with whatever is left in "code", so must let codout	*/
		/* generate the last field of code			*/
		longrel(sym,code);
		code->cnbits = 16;
	}
	else {
		codgen(8,opcd);
		relpc8(sym,code);
	} /* else */
} /* cjmpopt */

ujmpopt(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	long	opcd;

	opcd = code->cvalue;
	if (fread((char *)code,sizeof(*code),1,fdcode) != 1)
		aerror("Unexpected EOF on temporary (code) file");
	sym = symtab + (code->cindex - 1);	/* guaranteed a symbol is */
						/* there by shortsdi in pass 1 */
	if (*++longsdi) {
		opcd ^= 0x02L;
		codgen(8,opcd);
		/* codout (which called this routine) will call codgen	*/
		/* with whatever is left in "code", so must let codout	*/
		/* generate the last field of code			*/
		longrel(sym,code);
		code->cnbits = 16;
	}
	else {
		codgen(8,opcd);
		relpc8(sym,code);
	} /* else */
} /* ujmpopt */

loopopt(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	long	opcd;

	opcd = code->cvalue;
	if (fread((char *)code,sizeof(*code),1,fdcode) != 1)
		aerror("Unexpected EOF on temporary (code) file");
	sym = symtab + (code->cindex - 1);	/* guaranteed a symbol is */
						/* there by shortsdi in pass 1 */
	if (*++longsdi) {
		codgen(8,opcd);
		codgen(8,0x02L);
		codgen(8,0xebL);
		codgen(8,0x03L);
		codgen(8,0xe9L);
		/* codout (which called this routine) will call codgen	*/
		/* with whatever is left in "code", so must let codout	*/
		/* generate the last field of code			*/
		longrel(sym,code);
		code->cnbits = 16;
	}
	else {
		codgen(8,opcd);
		relpc8(sym,code);
	} /* else */
} /* loopopt */

resabs(sym,code)
	register symbol *sym;
	codebuf *code;
{
	if (sym != NULL) {
		switch (sym->styp & TYPE) {
	
			case ABS:
				code->cvalue += sym->value; /* sym must be non-null */
				return;
			case UNDEF:
				yyerror("Undefined symbol in absolute expression");
				return;
			default:
				yyerror("Relocatable symbol in absolute expression");
				return;
		}
	}
} /* resabs */

relpc8(sym,code)
	symbol *sym;
	codebuf *code;
{
	register short relval;
	register short stype;
	long val;

	val = code->cvalue;
	if (sym != NULLSYM)
		if ((stype = sym->styp & TYPE) != dot->styp && stype != ABS)
			aerror("relpc8: reference to symbol in another section");
		else
			val += sym->value;
	relval = (short)(val - (newdot + 1L)); /* compute number of bytes */
	code->cvalue = (long)relval;
	if ((relval < -128) || (relval > 127))
		aerror("relpc8: offset out of range");
} /* relpc8 */

swapb(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	register short temp;
	short val;

	val = (short)(code->cvalue);
	if (sym != NULLSYM) {
		if (transvec && (sym->styp & TVDEF)) {
			symrel(sym,R_IND16);
			code->cvalue = 0L;
			return;
		}
#if !iAPX286
		if (xflag && ((sym->styp & TYPE) != ABS))
			werror("Questionable relocatable reference");
#endif
		val += (short)(sym->value);
		relocat(sym,R_DIR16);
	}

/*
 *	This swabbing is necessary due to the way the 8086
 *	expects its memory reference operands (absolutes in b16 lingo)
 */

	temp = ((bytes *)(&val))->hibyte;
	((bytes *)(&val))->hibyte = ((bytes *)(&val))->lobyte;
	((bytes *)(&val))->lobyte = temp;
	code->cvalue = (long)val;
}

longrel(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	register short temp;
	short val;

	val = (short)(code->cvalue);
	if (sym != NULLSYM) {
		val += (short)(sym->value);
		relocat(sym,R_REL16);
	}
	val -= newdot + 2;	/* this magic number is the remaining */
				/* length of the instruction */

	temp = ((bytes *)(&val))->hibyte;
	((bytes *)(&val))->hibyte = ((bytes *)(&val))->lobyte;
	((bytes *)(&val))->lobyte = temp;
	code->cvalue = (long)val;
}

pack8(sym,code)
	symbol *sym;
	codebuf *code;
{
	register short numbits,
			val;

	numbits = code->cnbits;
	resabs(sym,code);
	val = (short)(code->cvalue);

	/* now mask out the proper number of bits */
	val &= (1 << numbits) - 1;
	currbit += numbits;
	bitfield |= val << (8 - currbit);	/* save the bits in their proper place */
	code->cnbits = 0;
}

pack16(sym,code)
	symbol *sym;
	codebuf *code;
{
	register short numbits,
			val;

	numbits = code->cnbits;
	resabs(sym,code);
	val = (short)(code->cvalue);

	/* now mask out the proper number of bits */
	val &= (numbits < 16) ? (1 << numbits) - 1 : 0xffff;
	currbit += numbits;
	bitfield |= val << (16 - currbit);	/* save the bits in their proper place */
	code->cnbits = 0;
}

/*ARGSUSED*/

dumpbits(sym,code)
	symbol *sym;
	codebuf *code;
{
	register short temp,
		numbits;

	numbits = (short)(code->cvalue);
	if (numbits == NBPW) {	/* swap bytes */
		temp = ((bytes *)(&bitfield))->hibyte;
		((bytes *)(&bitfield))->hibyte = ((bytes *)(&bitfield))->lobyte;
		((bytes *)(&bitfield))->lobyte = temp;
	}
	code->cnbits = (BYTE)numbits;
	code->cvalue = (long)bitfield;
	currbit = 0;	/* re-initialize */
	bitfield = 0;	/* re-initialize */
}

low8bits(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	if (sym != NULLSYM) {
		code->cvalue += sym->value;
		if ((sym->styp & TYPE) != ABS)
			symrel(sym,R_OFF8);
	}
	if (code->cnbits > 8)
		code->cvalue <<= 8; /* swap bytes */
} /* low8bits */

lo16bits(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	register short temp;
	short val;

	if (sym != NULLSYM) {
		code->cvalue += sym->value & 0xff;
		if ((sym->styp & TYPE) != ABS)
			symrel(sym,R_OFF16);
	}

	if (code->cnbits > 8) {
		/* swap bytes */
		val = (short)(code->cvalue);
		temp = ((bytes *)(&val))->hibyte;
		((bytes *)(&val))->hibyte = ((bytes *)(&val))->lobyte;
		((bytes *)(&val))->lobyte = temp;
		code->cvalue = (long)val;
	}
} /* lo16bits */

hi12bits(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	register short
		temp;
	short	val;
	prelent	trelent;

	if (code->cnbits == 8)
		yyerror("Unable to place 12-bit segment in 8 bits");
	else {
		if (sym != NULLSYM) {
			symrel(sym,R_SEG12);
			if (code->cvalue != 0L) {
				/* generate auxiliary relocation entry */
				trelent.reltype = R_AUX;
				trelent.relval = code->cvalue;
				trelent.relname = sym->_name.name;
				fwrite((char *)(&trelent),sizeof(prelent),
					1,fdrel);
				++relent;
			}
			code->cvalue += sym->value;
		}
		/* insure low 4 bits are zero */
		val = (short)(code->cvalue >> 4) & 0xfff0;
		temp = ((bytes *)(&val))->hibyte;
		((bytes *)(&val))->hibyte = ((bytes *)(&val))->lobyte;
		((bytes *)(&val))->lobyte = temp;
		code->cvalue = (long)val;
	}
}

#if !ONEPROC

flags(ch)
	register char ch;
{
	char	errmsg[28];

	if (ch == 'x')
#if !iAPX286
		xflag = YES;
#else
		{sprintf(errmsg,"Illegal flag in iAPX286 (%c) - ignored\n",ch);
		werror(errmsg);
		}
	else if (ch == 'M')
		{}
#endif
	else {
		sprintf(errmsg,"Illegal flag (%c) - ignored\n",ch);
		werror(errmsg);
	}
}
#endif

extern
	define(),
	setval(),
	settyp(),
	setscl(),
	settag(),
	setlno(),
	setsiz(),
	setdim1(),
	setdim2(),
	lineno(),
	linenum(),
	lineval(),
	endef(),
	setfile(),
	setmagic(),
	setflags(),
	newstmt();

int (*(modes[NACTION+2]))() = {
/*0*/	0,
/*1*/	define,
/*2*/	setval,
/*3*/	setscl,
/*4*/	settyp,
/*5*/	settag,
/*6*/	setlno,
/*7*/	setsiz,
/*8*/	setdim1,
/*9*/	setdim2,
/*10*/	lineno,
/*11*/	linenum,
/*12*/	lineval,
/*13*/	endef,
/*14*/	newstmt,
/*15*/	setfile,
/*16*/	setmagic,
/*17*/	resabs,
/*18*/	relpc8,
/*19*/	cjmpopt,
/*20*/	ujmpopt,
/*21*/	loopopt,
/*22*/	swapb,
/*23*/	longrel,
/*24*/	pack8,
/*25*/	pack16,
/*26*/	dumpbits,
/*27*/	low8bits,
/*28*/	lo16bits,
/*29*/	hi12bits,
/*30*/	setflags,
	0 };
