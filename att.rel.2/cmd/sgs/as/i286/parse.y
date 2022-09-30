/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
/* @(#)parse.y	1.4 - 85/09/10 */
%}
%{
%}
%{
	/* modified March 84 to add 186 instrs; jws */
#include <stdio.h>
#include <filehdr.h>
#include <sgs.h>
#include <symbols.h>
#include "systems.h"
#include "instab.h"
#include "gendefs.h"

	/* RANGE determines if an absolute expression is optimizable */
#define RANGE(x)	( x >= -128L && x <= 127L )

extern char
	file[],		/* contains the name of this file */
	cfile[];	/* contains C source name obtained from .file */

extern unsigned short
	line;	/* current line number */

extern short
#if !iAPX286
	xflag,		/* indicates if x86 syntax is accepted */
#endif
	transvec,	/* indicate transfer vector program addressing */
	bitpos,	/* current bit position unused in outword */
	opt;	/* optimize */

#if iAPX286
extern model;	/* run time flag to choose between 286 small or
			large memory models */
#endif

short	localopt = 1; /* local optimize flag */

extern long
	dotbss,	/* size of bss section */
	newdot; /* up-to-date value of "." */

extern symbol
	*dot;	/* "." */

extern FILE
#if !ONEPROC
	*fdstring,
#endif
	*fdin;

extern int
	deflab();

extern upsymins
	*lookup();

long
	defsegprfx() ;	/* returns default seg register for this instruction */

extern unsigned short
	actreloc();

char	newname[9];	/* used to contain new instr. name */

short	nameindx,	/* index into newname array */
	dbit;	/* "d" bit in 8086 opcode */

union addrmdtag
	memloc;	/* contains addressing mode information for this instr. */

upsymins
	newins;	/* pointer to new instruction */

static unsigned short
	action;	/* action routine to be called */

static short
	numbits,	/* total number of bits seen in bit packing exprs */
	spctype,	/* distinguishes between .byte, .half, or .word */
	width;	/* bit field width */

static long
	opcd;	/* opcode for this instruction */

static short
	flag186,	/* indicate that 186 instructions exist in the program */
	flag286;	/* indicate that 286 instructions exist in the program */

int special287 = 0 ;

%}
%union {
	long uulong;
	rexpr uuexpr;
	symbol *uusymptr;
	instr *uuinsptr;
	addrmode uumode;
	char *uustrptr;
	floatval uufval;
}
%token <uuinsptr> NO_OP BSLOD SLOD STROP1 STROP2 BSTROP2
%token <uuinsptr> STROP3 BSTROP3 STROP4 BSTROP4 PROT1 PROT2
%token <uuinsptr> LSDT ARPL BOUND ENTER
%token <uuinsptr> FLOAT1 FLOAT2 FLOAT3 FLOAT4 FLOAT5 FLOAT6 FLOAT7 FREG
%token <uuinsptr> EXPOP1 EXPOP2 RETOP SPOP1 CLR INCDEC MULDIV
%token <uuinsptr> LOOP JMPOP1 JMPOP2 LJMPOP CALLOP LCALLOP MONOP1 LOGIC1 LOGIC2
%token <uuinsptr> BCLR BINCDEC BMONOP1 BLOGIC1 BMULDIV
%token <uuinsptr> MOVOP TEST DYADIC AOTOP XCHG 
%token <uuinsptr> BMOVOP BTEST BDYADIC BXCHG
%token <uuinsptr> PSGLOBAL PSSET PSEVEN PSTV
%token <uuinsptr> PSBSS PSFILE PSTEXT PSDATA PSIDENT PSCOMM PSLCOMM
%token <uuinsptr> PSBYTE PSVALUE PSDEF PSVAL PSSCL PSTYPE PSSTRING
%token <uufval> PSFLOAT PSDOUBLE PSTEMP PSLONG PSLLONG PSBCD FNUMBER
%token <uuinsptr> PSTAG PSLINE PSSIZE PSDIM PSENDEF PSLN PSOPT PSJMPTAB
%token <uulong> EXPOPT MONOPT SPOPT DYADOPT INCOPT XCHGOPT JMPOPT
%token <uulong> IMMED MOVOPT INDIRECT SEGMEM
%token <uulong> SP XCLAIM QUOTE SHARP DOLLAR REG AMP SQ LP RP MUL PLUS
%token <uulong> COMMA MINUS ALPH DIV DIG COLON SEMI QUEST LB RB
%token <uulong> LT GT OR NL SLASH STAR ESCAPE MOD
%token <uulong> SEGPART OFFPART HIFLOAT LOFLOAT
%token <uulong> RSHIFT LSHIFT HAT EQ PERCNT GRAVE
%token <uusymptr> ID
%token <uuinsptr> REG16 BREG16 AREG16 DREG16 IREG16 SREG
%token <uuinsptr> REG8 AREG8 CLREG8
%token <uulong> NUMBER
%token <uustrptr> STRING
%token <uulong> ERR
%type <uulong> attrib
%type <uuexpr> exprX expr term floatreg
%type <uumode> dualreg8 dualreg16 dual8opnd dual16opnd
%type <uumode> segmem reg8mem reg16mem segdef segextaddr
%type <uumode> reg16 reg8
%type <uumode> mem immd reg16md reg8md
%type <uumode> extaddrmd dispmd expdispmd
%type <uuinsptr> dispbase dualdispbase
%%
wholeprog:	dotfile program = {
#if iAPX286
			switch ( model ) {
				case 's' :
					opcd = (long)I286SMAGIC;
					break;
				case 'l' :
					opcd = (long)I286LMAGIC;
					break;
				default:
					yyerror("Unrecognized model");
					break;
				}
#else
			if (xflag)
				opcd = (transvec) ? (long)IAPX20TV :
								(long)IAPX20;
			else
				opcd = (transvec) ? (long)IAPX16TV:
								(long)IAPX16;
#endif
			generate(0,SETMAG,opcd,NULLSYM);
			if (flag186 || flag286 ) {
				generate(0,SETFLAGS,(long)(flag286 ? F_80286 : F_80186),
									NULLSYM);
			}
		}
	;

	/* the next nonterminal symbol (dotfile) was added
	 * to satisfy yhe field update requirement that the .file
	 * pseudo op must be the fisrt one in the symbol table
	 */

dotfile:	PSFILE  STRING = {
			if (cfile[0] != '\0')
				yyerror("Only 1 '.file' allowed");
			if (strlen($2) > 14)
				yyerror(".file name too long");
			else {
				strcpy(cfile,$2);
#if !ONEPROC
				fprintf(fdstring,"%s\n",$2);
#endif
			}
			generate(0,SETFILE,NULLVAL,NULLSYM);
		}
	|	/* empty */ = {
			werror("cannot field update- '.file' not on first line");
		}
	;

program:	/* empty */
	|	program  linstr  endstmt = {
			goto reset;
		}
	|	program  error  endstmt = {
			yyerrok;
		reset:
			dbit = DBITOFF;
			memloc.addrmdfld = 0;
			dot->value = newdot; /* syncronize */
		}
	;

endstmt	:	NL = {
			++line;
			generate(0,NEWSTMT,(long)line,NULLSYM);
		}
	|	SEMI = {
			generate(0,NEWSTMT,(long)line,NULLSYM);
		}
	;

linstr :	instruction
	|	label  instruction
	;

label :		ID  COLON = {
			if (($1->styp & TYPE) != UNDEF)
				yyerror("multiply defined label");
			$1->value = newdot;
			$1->styp |= dot->styp;
			if (opt && (dot->styp == TXT))
				deflab($1);
		}
	;

instruction :	/* empty */
	|	NO_OP = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSLOD = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	SLOD = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSLOD  SREG = {
			generate(8,NOACTION,((long)SEGPFX | ($2->opcode << 3)),NULLSYM);
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	SLOD  SREG = {
			generate(8,NOACTION,((long)SEGPFX | ($2->opcode << 3)),NULLSYM);
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSLOD  dispbase  COMMA  AREG8 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	SLOD  dispbase  COMMA  AREG16 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSLOD  SREG  COLON  dispbase  COMMA  AREG8 = {
			strgen($1,$2,$4->name);
		}
	|	SLOD  SREG  COLON  dispbase  COMMA  AREG16 = {
			strgen($1,$2,$4->name);
		}
	|	STROP1 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	STROP1  SREG = {
			generate(8,NOACTION,((long)SEGPFX | ($2->opcode << 3)),NULLSYM);
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	STROP1  dispbase  COMMA  dispbase = {
			str1gen($1,$2->name,$4->name);
		}
	|	STROP1  SREG  COLON  dispbase  COMMA  dispbase = {
			/*
			 * don't generate seg prefix if %ds (default)
			 */
			
			if( $2->opcode != 3 )
				generate(8,NOACTION,((long)SEGPFX | ($2->opcode << 3)),NULLSYM);
			str1gen($1,$4->name,$6->name);
		}
	|	STROP1  SREG  COLON  dispbase  COMMA SREG COLON dispbase = {
			/*
			 *	second segment override must be es
			 */

			 if ( $6->opcode != 0 )
				yyerror("destination must be within es");

			/*
			 * don't generate seg prefix if %ds (default)
			 */
			
			if( $2->opcode != 3 )
				generate(8,NOACTION,((long)SEGPFX | ($2->opcode << 3)),NULLSYM);
			str1gen($1,$4->name,$8->name);
		}
	|	BSTROP2 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSTROP2  AREG8  COMMA  dispbase = {
			str2gen($1,$4->name);
		}
	|	STROP2 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	STROP2  AREG16  COMMA  dispbase = {
			str2gen($1,$4->name);
		}
	|	BSTROP3 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSTROP3 LP IREG16 RP COMMA LP DREG16 RP = {
			str3gen($1,$3->name,$7->name);
		}
	| 	STROP3 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
	}
	| 	STROP3 LP IREG16 RP COMMA LP DREG16 RP = {
			str3gen($1,$3->name,$7->name);
		}
	|	BSTROP4 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	BSTROP4 LP DREG16 RP COMMA LP IREG16 RP = {
			str4gen($1,$3->name,$7->name);
		}
	| 	STROP4 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	| 	STROP4 LP DREG16 RP COMMA LP IREG16 RP = {
			str4gen($1,$3->name,$7->name);
		}
	|	EXPOP1 = {
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'I';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
			if (newins.itp != NULL) {
				generate(newins.itp->nbits,NOACTION,newins.itp->opcode,NULLSYM);
			}
			else 
				yyerror("Illegal instruction");
		}
	|	EXPOP1  immd = {
			if (($2.adexpr.symptr == NULLSYM) &&	/* absolute */
				($2.adexpr.expval == 3L)) {	/* & is 3 */
				nameindx = copystr($1->name,newname);
				newname[nameindx++] = 'I';
				newname[nameindx] = '\0';
				newins = *lookup(newname,N_INSTALL,MNEMON);
				if (newins.itp != NULL) {
					generate(newins.itp->nbits,NOACTION,newins.itp->opcode,NULLSYM);
				}
				else 
					yyerror("Illegal instruction");
			}
			else {
				generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
				generate(8,RESABS,$2.adexpr.expval,$2.adexpr.symptr);
			}
		}
	|	EXPOP2  LP  DREG16  RP = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	EXPOP2  SREG  COLON  LP  DREG16  RP = {
			generate(8,NOACTION,((long)SEGPFX | ($2->opcode << 3)),NULLSYM);
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	EXPOP2  segextaddr = {
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'E';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
			if (newins.itp != NULL) {
				generate(newins.itp->nbits,NOACTION,newins.itp->opcode,NULLSYM);
				generate(8,LOW8BITS,$2.adexpr.expval,$2.adexpr.symptr);
			}
			else 
				yyerror("Illegal instruction");
		}
	|	RETOP = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	RETOP  segextaddr = {
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'E';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
			if (newins.itp != NULL) {
				generate(newins.itp->nbits,NOACTION,newins.itp->opcode,NULLSYM);
				generate(16,actreloc($2.adexpr.exptype,SWAPB),
					$2.adexpr.expval,$2.adexpr.symptr);
			}
			else
				yyerror("Illegal ret");
		}
	|	SPOP1  reg16mem = {
			rmongen($1,&($2));
		}
	|	SPOP1  SREG = {
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'S';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
			if (newins.itp != NULL) {
				opcd = newins.itp->opcode;
				opcd |= $2->opcode << 3;
				generate(newins.itp->nbits,NOACTION,opcd,NULLSYM);
			}
			else
				aerror("Cannot find SP instr");
		}
	/* added March 84 for 186 pushI (push immediate) instr; jws */
	/* changed for pushIB (push immediate byte) 3/85; tyee */
	|	SPOP1 immd = {
			if($1->name[1] == 'o')
				yyerror("Syntax error:	cannot pop to an immediate ");
			nameindx = copystr($1->name,newname);
			/* select one byte push immediate or 2 byte push immd */
			if( ($2.adexpr.symptr == NULLSYM ) &&
				( !($2.adexpr.expval & 0xff80) ||
				   ( ($2.adexpr.expval & 0xff80) == 0xff80)) )
			{
				newname[nameindx++] = 'I';
				newname[nameindx++] = 'B';
				newname[nameindx] = '\0';
				newins = *lookup(newname,N_INSTALL,MNEMON);
				generate(newins.itp->nbits,NOACTION,
				    newins.itp->opcode,NULLSYM);
				immedgen(8,&($2));
			}
			else
			{
				newname[nameindx++] = 'I';
				newname[nameindx] = '\0';
				newins = *lookup(newname,N_INSTALL,MNEMON);
				generate(newins.itp->nbits,NOACTION,
				    newins.itp->opcode,NULLSYM);
				immedgen(16,&($2));
			}
		}
	|	CLR  reg16mem = {
			if ($2.admode & REGMASK) {
				memloc.addrmd.reg = $2.adreg;
				newins = *lookup("xor",N_INSTALL,MNEMON);
				if (newins.itp != NULL) {
					generate(newins.itp->nbits,NOACTION,(long)(newins.itp->opcode | memloc.addrmdfld),NULLSYM);
				}
				else
					aerror("Lost xor instr");
			}
			else {
				newins = *lookup("movIG",N_INSTALL,MNEMON);
				if (newins.itp != NULL) {
					addrgen(newins.itp->opcode,&($2));
					generate(16,NOACTION,0L,NULLSYM);
				}
				else
					aerror("Lost mov (immed) instr");
			}
		}
	|	BCLR  reg8mem = {
			if ($2.admode & REGMASK) {
				memloc.addrmd.reg = $2.adreg;
				newins = *lookup("xorb",N_INSTALL,MNEMON);
				if (newins.itp != NULL) {
					generate(newins.itp->nbits,NOACTION,(long)(newins.itp->opcode | memloc.addrmdfld),NULLSYM);
				}
				else
					aerror("Lost xorb instr");
			}
			else {
				newins = *lookup("movbIG",N_INSTALL,MNEMON);
				if (newins.itp != NULL) {
					addrgen(newins.itp->opcode,&($2));
					generate(8,NOACTION,0L,NULLSYM);
				}
				else
					aerror("Lost movb (immed) instr");
			}
		}
	|	INCDEC  reg16mem = {
			rmongen($1,&($2));
		}
	|	BINCDEC  reg8mem = {
			addrgen($1->opcode,&($2));
		}
	|	LOOP  segextaddr = {
			loopgen($1,&($2));
		}
	|	JMPOP1  segextaddr = {
			jmp1opgen($1,&($2));
		}
	|	JMPOP2  segextaddr = {
			jmp2opgen($1,&($2));
		}
	|	JMPOP2  segdef = {
			defgen($1->name,&($2));
		}
	|	LJMPOP  extaddrmd  = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
  			generate(16,actreloc($2.adexpr.exptype,SWAPB),
				$2.adexpr.expval,$2.adexpr.symptr);
			$2.adexpr.exptype |= HI12TYPE ;
			generate(16,actreloc($2.adexpr.exptype,SWAPB),
				$2.adexpr.expval,$2.adexpr.symptr);
		}
	|	LJMPOP  extaddrmd  COMMA extaddrmd = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
			generate(16,actreloc($4.adexpr.exptype,SWAPB),
				$4.adexpr.expval,$4.adexpr.symptr);
			generate(16,actreloc($2.adexpr.exptype,SWAPB),
				$2.adexpr.expval,$2.adexpr.symptr);
		}
	|	LJMPOP  segdef = {
			defgen($1->name,&($2));
		}
	|	CALLOP  segextaddr = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
			generate(16,LONGREL,$2.adexpr.expval,$2.adexpr.symptr);
		}
	|	CALLOP  segdef = {
			defgen($1->name,&($2));
		}
	|	LCALLOP  extaddrmd  = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
  			generate(16,actreloc($2.adexpr.exptype,SWAPB),
				$2.adexpr.expval,$2.adexpr.symptr);
			$2.adexpr.exptype |= HI12TYPE ;
			generate(16,actreloc($2.adexpr.exptype,SWAPB),
				$2.adexpr.expval,$2.adexpr.symptr);
		}
	|	LCALLOP  extaddrmd  COMMA extaddrmd = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
			generate(16,actreloc($4.adexpr.exptype,SWAPB),
				$4.adexpr.expval,$4.adexpr.symptr);
			generate(16,actreloc($2.adexpr.exptype,SWAPB),
				$2.adexpr.expval,$2.adexpr.symptr);
		}
	|	LCALLOP  segdef = {
			defgen($1->name,&($2));
		}
	|	BMULDIV  reg8mem = {
			addrgen($1->opcode,&($2));
		}
	|	BMULDIV  reg8  COMMA  AREG8 = {
			memloc.addrmd.rm = $2.adreg;
			memloc.addrmd.mod = REGMOD;
			generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	BMULDIV  segmem  COMMA  AREG8 = {
			addrgen($1->opcode,&($2));
		}
	|	MULDIV  reg16mem = {
			addrgen($1->opcode,&($2));
		}
	|	MULDIV  reg16  COMMA  AREG16 = {
			memloc.addrmd.rm = $2.adreg;
			memloc.addrmd.mod = REGMOD;
			generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	MULDIV  segmem  COMMA  AREG16 = {
			addrgen($1->opcode,&($2));
		}
	|	MULDIV	immd COMMA reg16mem COMMA reg16 = {
			if(($1->name[0] != 'i') && ($1->name[1] != 'm'))
				yyerror("Syntax error.");
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'I';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
	
			/* select one byte or 2 byte multiply */

			if( ($2.adexpr.symptr == NULLSYM) &&
				( !($2.adexpr.expval & 0xff80) ||
				   ( ($2.adexpr.expval & 0xff80) == 0xff80)) )
			{
				addrgen ( newins.itp->opcode | 0x0200 | ($6.adreg << 3) , &($4) );
				immedgen(8,&($2));
			}
			else
			{
				addrgen ( newins.itp->opcode | ($6.adreg << 3) ,&($4) );
				immedgen(16,&($2));
			}
		}
	|	MULDIV	immd COMMA reg16 = {
			if(($1->name[0] != 'i') && ($1->name[1] != 'm'))
				yyerror("Syntax error.");
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'I';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
	
			/*
			 *	force correct address mode
			 */

			memloc.addrmd.mod = REGMOD;
			memloc.addrmd.reg = $4.adreg;
			memloc.addrmd.rm = $4.adreg;

			/* only one byte multiply is legal */
			/* in INTEL but allow both forms for */
			/* our compiler */

			if( ($2.adexpr.symptr == NULLSYM) &&
				( !($2.adexpr.expval & 0xff80) ||
				   ( ($2.adexpr.expval & 0xff80) == 0xff80)) )
			{
				addrgen ( newins.itp->opcode | 0x0200 | ($4.adreg << 3) , &($4) );
				immedgen(8,&($2));
			}
			else
			{
				addrgen ( newins.itp->opcode | ($4.adreg << 3) ,&($4) );
				immedgen(16,&($2));
			}
		}
	|	BMONOP1  reg8mem = {
			addrgen($1->opcode,&($2));
		}
	|	MONOP1  reg16mem = {
			addrgen($1->opcode,&($2));
		}
	/*
	 *	80287 - no operands
	 */
	|	FLOAT1 = {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	/*
	 *	80287 - single (memory access) operand
	 */
	|	FLOAT2 segmem = {
			/* special case if size is 24 - then generate fwait */
			if($1->nbits == 24)
				generate( 8 , NOACTION , 0x9bL , NULLSYM);
			addrgen($1->opcode,&($2));
		}
	/*
	 *	80287 - two operands of form %st,%st(i) or %st(i),%st
	 *		destination %st	  - direction flag = 0
	 *		destination %st(i)- direction flag = 1
	 *	also:-
	 *		without any operands implies
	 *		destination %st(1) - direction flag = 1
	 */
	|	FLOAT3 = {
			generate($1->nbits,NOACTION,$1->opcode|0x0e01,NULLSYM);
		}
	|	FLOAT3 FREG COMMA floatreg = {
			generate($1->nbits,NOACTION,$1->opcode|$4.expval|0x0400,NULLSYM);
		}
	|	FLOAT3 floatreg COMMA FREG = {
			generate($1->nbits,NOACTION,$1->opcode|$2.expval,NULLSYM);
		}
	/*
	 *	80287 - single operand of form %st(i) 
	 */
	|	FLOAT4 floatreg = {
			generate($1->nbits,NOACTION,$1->opcode|$2.expval,NULLSYM);
		}
	/*
	 *	80287 - two operands %st,%st(i) only direction allowed
	 */
	|	FLOAT5 FREG COMMA floatreg = {
			generate($1->nbits,NOACTION,$1->opcode|$4.expval,NULLSYM);
		}
	/*
	 *	80287 - stsw specials either %ax or memory access allowed
	 */
	|	FLOAT6 AREG16 = {
			/* special case if size is 24 - then generate fwait */
			if($1->nbits == 24)
				generate( 8 , NOACTION , 0x9bL , NULLSYM);
			generate(16,NOACTION,0xdfe0L,NULLSYM);
		}
	|	FLOAT6 segmem = {
			/* special case if size is 24 - then generate fwait */
			if($1->nbits == 24)
				generate( 8 , NOACTION , 0x9bL , NULLSYM);
			addrgen($1->opcode,&($2));
		}
	/*
	 *	80287 special instructions - usually take just 1 parameter
	 *		%st(i)
	 *	but can have implied default of %st(1) if no operands supplied
	 */
	|	FLOAT7 floatreg = {
			generate($1->nbits,NOACTION,$1->opcode|$2.expval,NULLSYM);
		}
	|	FLOAT7  = {
			generate($1->nbits,NOACTION,$1->opcode|0x0001,NULLSYM);
		}
	|	PROT1 	reg16mem = {
			laddrgen($1->opcode,&($2));
		}
	|	PROT2   dual16opnd = {
			if(dbit == DBITOFF)
				yyerror ( "Syntax error - use mem , reg");
			laddrgen($1->opcode | dbit,&($2));
		}
	|	PROT2 dualreg16 {
			generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
			}
	|	LSDT	segmem = {
			laddrgen($1->opcode,&($2));
		}
	|	ARPL	dual16opnd = {
			if(dbit == DBITON)
				yyerror ( "Syntax error - use arpl reg , mem");
			addrgen($1->opcode ,&($2));
		}
	|	ARPL	dualreg16 {
			{
				short swapper;
			/* must swap register fields as arpl is "strange" */
				swapper = memloc.addrmd.rm ;
				memloc.addrmd.rm = memloc.addrmd.reg ;
				memloc.addrmd.reg = swapper ;
				generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
				}
			}
	|	BOUND	dual16opnd = {
			if(dbit == DBITOFF)
				yyerror ( "Syntax error - use bound mem , reg");
			addrgen($1->opcode ,&($2));
		}
	|	ENTER immd COMMA immd= {
			generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
			immedgen(16,&($2));
			immedgen(8,&($4));
		}
	|	LOGIC1  reg16mem = {
			addrgen($1->opcode,&($2));
		}
	|	LOGIC1  CLREG8  COMMA  reg16mem = {
			addrgen($1->opcode ^ 0x0200,&($4));
		}
	|	LOGIC1  immd	COMMA  reg16mem = {
			flag186 = YES;
			if( ($2.adexpr.symptr == NULLSYM) &&
				($2.adexpr.expval == 1L) )
			{
				addrgen($1->opcode,&($4));
			}
			else
			{
				addrgen($1->opcode & 0xc1ff ,&($4));
				immedgen(8,&($2));
			}
		}
	|	BLOGIC1  immd	COMMA  reg8mem = {
			flag186 = YES;
			if( ($2.adexpr.symptr == NULLSYM) &&
				($2.adexpr.expval == 1L) )
			{
				addrgen($1->opcode,&($4));
			}
			else
			{
				addrgen($1->opcode & 0xc1ff ,&($4));
				immedgen(8,&($2));
			}
		}
	|	BLOGIC1  reg8mem = {
			addrgen($1->opcode,&($2));
		}
	|	BLOGIC1  CLREG8  COMMA  reg8mem = {
			addrgen($1->opcode ^ 0x0200,&($4));
		}
	|	MOVOP  dualreg16 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	MOVOP  dual16opnd = {
			movgen($1,&($2));
		}
	|	MOVOP  immd  COMMA  reg16mem = {
			mov2gen($1->name,&($4));
			immedgen(16,&($2));
		}
	|	MOVOP  SREG  COMMA  reg16mem = {
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'S';
			newname[nameindx++] = 'G';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
			if (newins.itp != NULL) {
				memloc.addrmd.reg = $2->opcode;
				addrgen(newins.itp->opcode,&($4));
			}
			else
				aerror("Cannot find mov (seg) instr");
		}
	|	MOVOP  segmem  COMMA  SREG = {
			goto smov;
		}
	|	MOVOP  reg16  COMMA  SREG = {
			memloc.addrmd.rm = $2.adreg;
			memloc.addrmd.mod = REGMOD;
		smov:
			nameindx = copystr($1->name,newname);
			newname[nameindx++] = 'G';
			newname[nameindx++] = 'S';
			newname[nameindx] = '\0';
			newins = *lookup(newname,N_INSTALL,MNEMON);
			if (newins.itp != NULL) {
				memloc.addrmd.reg = $4->opcode;
				addrgen(newins.itp->opcode,&($2));
			}
			else
				aerror("Cannot find mov (seg) instr");
		}
	|	BMOVOP  dualreg8 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	BMOVOP  dual8opnd = {
			movgen($1,&($2));
		}
	|	BMOVOP  immd  COMMA  reg8mem = {
			mov2gen($1->name,&($4));
			immedgen(8,&($2));
		}
	|	LOGIC2  dualreg16 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	LOGIC2  dual16opnd = {
			addrgen($1->opcode | dbit,&($2));
		}
	|	LOGIC2  immd = {
			dyad1gen($1->name);
			immedgen(16,&($2));
		}
	|	LOGIC2  immd  COMMA  reg16mem = {
			dyad2gen($1->name,&($4));
			immedgen(16,&($2));
		}
/* 	the BLOGIC2 token will be replaced by the BDYADIC token since they
 *	do the same thing.
 *	|	BLOGIC2  dualreg8 = {
 *			generate($1->nbits,NOACTION,(long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
 *		}
 *	|	BLOGIC2  dual8opnd = {
 *			addrgen($1->opcode | dbit,&($2));
 *		}
 *	|	BLOGIC2  immd  COMMA  reg8mem = {
 *			dyad2gen($1->name,&($4));
 *			immedgen(8,&($2));
 *		}
 *	|	BLOGIC2  immd = {
 *			dyad1gen($1->name);
 *			immedgen(8,&($2));
 *		}
 */
	|	TEST  reg16mem = {
			testopt($1->name,&($2),NO);
		}
	|	TEST  dualreg16 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	TEST  dual16opnd = {
			addrgen($1->opcode,&($2));
		}
	|	TEST  immd = {
			dyad1gen($1->name);
			immedgen(16,&($2));
		}
	|	TEST  immd  COMMA  reg16mem = {
			testgen($1->name,&($4));
			immedgen(16,&($2));
		}
	|	BTEST  reg8mem = {
			testopt($1->name,&($2),YES);
		}
	|	BTEST  dualreg8 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	BTEST  dual8opnd = {
			addrgen($1->opcode,&($2));
		}
	|	BTEST  immd  COMMA  reg8mem = {
			testgen($1->name,&($4));
			immedgen(8,&($2));
		}
	|	BTEST  immd = {
			dyad1gen($1->name);
			immedgen(8,&($2));
		}
	|	DYADIC  dualreg16 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	DYADIC  dual16opnd = {
			addrgen($1->opcode | dbit,&($2));
		}
	|	DYADIC  immd = {
			dyad1gen($1->name);
			immedgen(16,&($2));
		}
	|	DYADIC  immd  COMMA  reg16mem = {
			dbit = DBITOFF;
			if (($2.adexpr.symptr == NULLSYM) &&	/* absolute */
				opt && RANGE($2.adexpr.expval))
				dbit = DBITON;
#if !iAPX286
			if (xflag && ($2.adexpr.exptype & HI12TYPE))
#else
			if (($2.adexpr.exptype & HI12TYPE))
#endif
			{
				if($2.adexpr.expval)
					yyerror("Illegal fixup");
				dbit = DBITOFF;
			}
			dyad2gen($1->name,&($4));
			immedgen(dbit ? 8 : 16,&($2));
		}
	|	BDYADIC  dualreg8 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | dbit | memloc.addrmdfld),NULLSYM);
		}
	|	BDYADIC  dual8opnd = {
			addrgen($1->opcode | dbit,&($2));
		}
	|	BDYADIC  immd  COMMA  reg8mem = {
			dyad2gen($1->name,&($4));
			immedgen(8,&($2));
		}
	|	BDYADIC  immd = {
			dyad1gen($1->name);
			immedgen(8,&($2));
		}
	|	AOTOP  dualreg16 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	AOTOP  segmem  COMMA  reg16 = {
			memloc.addrmd.reg = $4.adreg;
			addrgen($1->opcode,&($2));
		}
	|	XCHG  reg16 = {
			xchgopt($1->name,$2.adreg);
		}
	|	XCHG  dualreg16 = {
			if (opt && ($2.admode == AREG16MD)) {
				xchgopt($1->name,$2.adreg);
			}
			else {
				generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
			}
		}
	|	XCHG  dual16opnd = {
			addrgen($1->opcode,&($2));
		}
	|	BXCHG  dualreg8 = {
			generate($1->nbits,NOACTION,(long)($1->opcode | memloc.addrmdfld),NULLSYM);
		}
	|	BXCHG  dual8opnd = {
			addrgen($1->opcode,&($2));
		}
	|	asmdir
	;

dualreg16 :	reg16  COMMA  reg16 = {
			memloc.addrmd.mod = REGMOD;
			memloc.addrmd.reg = $3.adreg;
			memloc.addrmd.rm = $1.adreg;
			dbit = DBITON;
			if ($1.admode == AREG16MD)
				$$.adreg = $3.adreg;
			else
				$$.admode = $3.admode;
		}
	;

dual16opnd :	reg16  COMMA  segmem = {
			memloc.addrmd.reg = $1.adreg;
			dbit = DBITOFF;
			$$ = $3;
		}
	|	segmem  COMMA  reg16 = {
			memloc.addrmd.reg = $3.adreg;
			dbit = DBITON;
		}
	;

dualreg8 :	reg8  COMMA  reg8 = {
			memloc.addrmd.mod = REGMOD;
			memloc.addrmd.reg = $3.adreg;
			memloc.addrmd.rm = $1.adreg;
			dbit = DBITON;
			if ($1.admode == AREG8MD)
				$$.adreg = $3.adreg;
			else
				$$.admode = $3.admode;
		}
	;

dual8opnd :	reg8  COMMA  segmem = {
			memloc.addrmd.reg = $1.adreg;
			dbit = DBITOFF;
			$$ = $3;
		}
	|	segmem  COMMA  reg8 = {
			dbit = DBITON;
			memloc.addrmd.reg = $3.adreg;
		}
	;

segextaddr :	extaddrmd
	|	SREG  COLON  extaddrmd = {
			if( defsegprfx() != $1->opcode)
				generate(8,NOACTION,((long)SEGPFX | ($1->opcode << 3)),NULLSYM);
			$$ = $3;
		}
	;

segdef :	STAR  reg16mem = {
			$$ = $2;
		}
	;

reg16mem :	reg16md
	|	segmem
	;

reg8mem :	reg8md
	|	segmem
	;

segmem :	mem
	|	SREG  COLON  mem = {
			if( defsegprfx() != $1->opcode)
				generate(8,NOACTION,((long)SEGPFX | ($1->opcode << 3)),NULLSYM);
			$$ = $3;
		}
	;

mem	:	extaddrmd
	|	expdispmd
	|	dispmd
	;

immd	:	DOLLAR  expr = {
			$$.admode = IMMD;
			$$.adexpr = $2;
		}
	;

reg16md :	reg16 = {
			memloc.addrmd.rm = $1.adreg;
			memloc.addrmd.mod = REGMOD;
		}
	;

reg8md :	reg8 = {
			memloc.addrmd.rm = $1.adreg;
			memloc.addrmd.mod = REGMOD;
		}
	;

extaddrmd:	expr = {
			$$.admode = EXADMD;
			$$.adexpr = $1;
			memloc.addrmd.mod = NODISP;
			memloc.addrmd.rm = EA16;
		}
	;

expdispmd:	expr  dispbase = {
			$$.adexpr = $1;
			if (($1.symptr == NULLSYM) &&
		 		($1.expval == 0L) &&
		 		(memloc.addrmd.rm != BP)) {
		 
		 		$$.admode = DSPMD;
		 		memloc.addrmd.mod = NODISP;
		 	}
		 	else {
		 
				$$.admode = EDSPMD;
				/* indicate long form for now */
				memloc.addrmd.mod = DISP16;
		
		 	}
		 
		}
	|	expr  dualdispbase = {
			$$.adexpr = $1;
			if (($1.symptr == NULLSYM) &&
		 		($1.expval == 0L)) {
		 
		 		$$.admode = DSPMD;
		 		memloc.addrmd.mod = NODISP;
		 	}
		 	else {
		 
				$$.admode = EDSPMD;
				/* indicate long form for now */
				memloc.addrmd.mod = DISP16;
		
		 	}
		 
		}
	;

dispmd :	dispbase = {
			$$.admode = DSPMD;
			memloc.addrmd.mod = NODISP;
			if (memloc.addrmd.rm == BP)
				yyerror("Illegal BP use");
		}
	|	dualdispbase = {
			$$.admode = DSPMD;
			memloc.addrmd.mod = NODISP;
		}
	;

reg16 :		REG16 = {
			$$.admode = REG16MD;
			/* register number is in opcode field of instab */
			$$.adreg = (short)($1->opcode);
		}
	|	BREG16 = {
			$$.admode = REG16MD;
			$$.adreg = (short)($1->opcode);
		}
	|	AREG16 = {
			$$.admode = AREG16MD;
			$$.adreg = (short)($1->opcode);
		}
	|	IREG16 = {
			$$.admode = REG16MD;
			$$.adreg = (short)($1->opcode);
		}
	|	DREG16 = {
			$$.admode = REG16MD;
			$$.adreg = (short)($1->opcode);
		}
	;

reg8 :		AREG8 = {
			$$.admode = AREG8MD;
			$$.adreg = (short)($1->opcode);
		}
	|	REG8 = {
			$$.admode = REG8MD;
			$$.adreg = (short)($1->opcode);
		}
	|	CLREG8 = {
			$$.admode = REG8MD;
			$$.adreg = (short)($1->opcode);
		}
	;

dispbase:	LP  BREG16  RP = {
			$$ = $2;
			memloc.addrmd.rm = $2->tag;
		}
	|	LP  IREG16  RP = {
			$$ = $2;
			memloc.addrmd.rm = $2->tag;
		}
	;

dualdispbase :	LP  BREG16  COMMA  IREG16  RP = {
			$$ = $2;
			if ($2->name[1] == 'p')
				memloc.addrmd.rm = 02;
			if ($4->name[0] == 'd')
				memloc.addrmd.rm |= 01;
		}
	;
	/*
	 *	80287 - floatreg 
	 *	%st(i)
	 *		0 <= i <= 15
	 */
floatreg:	FREG LP expr RP = {
			if( ($3.symptr != NULLSYM ) ||
			    ($3.exptype != ABS ) ||
			    ($3.expval & 0xfff8 ) )
			{
				yyerror ( "Syntax error - stack register index should be constant <= 7 and >=0");
			}
			$$ = $3 ;
		}
	;

expr	:	exprX = {
			if(( $1.exptype & HI12TYPE ) && ( $1.expval) )
				yyerror("Can't relocate segment fixup");
			$$ = $1 ;
		}
exprX	:	term
	|	exprX  PLUS  term = {
			if ($1.symptr == NULLSYM) {
				$$.symptr = $3.symptr;
				$$.exptype = $3.exptype;
#if !iAPX286
				if (xflag && ($3.exptype & LO8TYPE))
#else
				if (($3.exptype & LO8TYPE))
#endif
					$$.exptype = ($$.exptype & (~LO8TYPE)) |
							LO16TYPE;
			}
			else if ($3.symptr == NULLSYM) {
				$$.symptr = $1.symptr;
				$$.exptype = $1.exptype;
#if !iAPX286
				if (xflag && ($1.exptype & LO8TYPE))
#else
				if (($1.exptype & LO8TYPE))
#endif
					$$.exptype = ($$.exptype & (~LO8TYPE)) |
							LO16TYPE;
			}
			else {
				yyerror("Illegal addition");
			}

			$$.expval = $1.expval + $3.expval;
		}
	|	exprX  MINUS  term = {
			if ($3.symptr == NULLSYM) {
				$$.symptr = $1.symptr;
				$$.exptype = $1.exptype;
				$$.expval = $1.expval - $3.expval;
			}
			else if (($1.exptype == $3.exptype) &&
				($1.exptype != UNDEF)) {
				$$.symptr = NULLSYM;
				$$.exptype = ABS;
				$$.expval = $1.expval - $3.expval;
				if ($1.symptr != NULLSYM)
					$$.expval += $1.symptr->value -
							$3.symptr->value;
			}
			else {
				yyerror("Illegal subtraction");
				$$.symptr = NULLSYM;
				$$.exptype = UNDEF;
				$$.expval = 0L;
			}
		}
	|	exprX  MUL  term = {
			if ($1.symptr != NULLSYM || $3.symptr != NULLSYM) {
				yyerror("Illegal multiplication");
			}
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval * $3.expval;
		}
	|	exprX  DIV  term = {
			if ($1.symptr != NULLSYM || $3.symptr != NULLSYM) {
				yyerror("Illegal division");
			}
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval / $3.expval;
		}
	|	exprX  AMP  term = {
			if (($1.symptr != NULLSYM) || ($3.symptr != NULLSYM))
				yyerror("Illegal logical and");
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval & $3.expval;
		}
	|	exprX  OR  term = {
			if (($1.symptr != NULLSYM) || ($3.symptr != NULLSYM))
				yyerror("Illegal logical or");
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval | $3.expval;
		}
	|	exprX  RSHIFT  term = {
			if (($1.symptr != NULLSYM) || ($3.symptr != NULLSYM))
				yyerror("Illegal right shift");
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval >> $3.expval;
		}
	|	exprX  LSHIFT  term = {
			if (($1.symptr != NULLSYM) || ($3.symptr != NULLSYM))
				yyerror("Illegal left shift");
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval << $3.expval;
		}
	|	exprX  MOD  term = {
			if (($1.symptr != NULLSYM) || ($3.symptr != NULLSYM))
				yyerror("Illegal modulo division");
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval % $3.expval;
		}
	|	exprX  XCLAIM  term = {
			if (($1.symptr != NULLSYM) || ($3.symptr != NULLSYM))
				yyerror("Illegal bang");
			$$.symptr = NULLSYM;
			$$.exptype = ABS;
			$$.expval = $1.expval & (~ $3.expval);
		}
	|	exprX  HAT  term = {
			$$.symptr = $1.symptr;
			$$.exptype = $3.exptype;
			$$.expval = $1.expval;
		}
	;

term	:	ID = {
			if (($$.exptype = $1->styp & TYPE) == ABS) {
				$$.expval = $1->value;
				$$.symptr = NULLSYM;
			}
			else {
				$$.symptr = $1;
				$$.expval = 0L;
			}
		}
	|	NUMBER = {
			$$.exptype = ABS;
			$$.expval = $1;
			$$.symptr = NULLSYM;
		}
	|	MINUS  term = {
			$$.exptype = $2.exptype;
			$$.symptr = $2.symptr;
			$$.expval = - $2.expval;
		}
	|	LB  exprX  RB = {
			$$ = $2;
		}
	|	OFFPART  term = {
			$$ = $2;
#if iAPX286
			$$.exptype |= LO16TYPE;
#else
			$$.exptype |= LO8TYPE;
#endif
		}
	|	SEGPART  term = {
			$$ = $2;
			$$.exptype |= HI12TYPE;
		}
	;

asmdir	:	PSTEXT = {
			cgsect((short)($1->tag));
		}
	|	PSDATA = {
			cgsect((short)($1->tag));
		}
	|	PSBSS = {
			cgsect((short)($1->tag));
		}
	|	PSCOMM ID COMMA exprX = {
			if ($4.exptype != ABS)
				yyerror("comm size not absolute");
			if (($2->styp & TYPE) != UNDEF)
				yyerror("illegal attempt to redefine symbol");
			else
			{
				$2->styp = (EXTERN | UNDEF);
				$2->value = $4.expval;
			}
		}
	|	PSLCOMM ID COMMA exprX = {
			if ($4.exptype != ABS)
				yyerror("lcomm size not absolute");
			if (($2->styp & TYPE) != UNDEF)
				yyerror("illegal attempt to redefine symbol");
			else
			{
			/* assume that odd length items may be on arbitrary
			** boundaries, but even length items must be on even
			** boundaries
			*/
				if (($2->value & 01) == 0 && (dotbss & 01) != 0)
					 dotbss++;
				$2->value = dotbss;
				dotbss += $4.expval;
				$2->styp |= BSS;
			}
		}
	|	PSGLOBAL ID = {
			$2->styp |= EXTERN;
		}
	|	PSTV  ID = {
			$2->styp |= TVDEF;
			transvec = YES;
		}
	|	PSSET  ID  COMMA  exprX  =  {
			if ($2 == dot)
				if (($2->styp != $4.exptype))
					yyerror("Incompatible types");
				else {
					/* Operands are of the same type but
					 * cannot assume 'expr' is relocatable
					 * just because 'dot' is because
					 * of the HAT operator!
					 */

					long incr;

					incr = ($4.symptr != NULLSYM) ? $4.symptr->value : 0L;
					if ((incr += $4.expval - $2->value) < 0L)
						yyerror("Cannot decrement '.'");
					else {
						if ($2->styp != BSS)
							fill(incr);
						else
							newdot += incr;
					} /* else */
				} /* $2->styp == $4.expval */
			else { /* $2!=dot */
			/* reset all but EXTERN bit */
				$2->styp &= EXTERN;
				if (($2->styp |= $4.exptype & TYPE) == UNDEF)
					yyerror("Illegal `.set' expression");
				$2->value = $4.expval;
				if ($4.symptr != NULLSYM) {
					$2->value += $4.symptr->value;
					$2->maxval = $4.symptr->maxval;
				}
			} /* else */
		}
	|	ID  EQ  exprX  =  {
			if ($1 == dot)
				if (($1->styp != $3.exptype))
					yyerror("Incompatible types");
				else {
					/* Operands are of the same type but
					 * cannot assume 'expr' is relocatable
					 * just because 'dot' is because
					 * of the HAT operator!
					 */

					long incr;

					incr = ($3.symptr != NULLSYM) ? $3.symptr->value : 0L;
					if ((incr += $3.expval - $1->value) < 0L)
						yyerror("Cannot decrement '.'");
					else {
						if ($1->styp != BSS)
							fill(incr);
						else
							newdot += incr;
					} /* else */
				} /* $1->styp == $3.expval */
			else { /* $1!=dot */
			/* reset all but EXTERN bit */
				$1->styp &= EXTERN;
				if (($1->styp |= $3.exptype & TYPE) == UNDEF)
					yyerror("Illegal `.set' expression");
				$1->value = $3.expval;
				if ($3.symptr != NULLSYM) {
					$1->value += $3.symptr->value;
					$1->maxval = $3.symptr->maxval;
				}
			} /* else */
		}
	|	PSBSS  ID  COMMA  exprX = {
			short oldsect;

			oldsect = dot->styp;
			cgsect((short)($1->tag));
			if ($4.exptype != ABS)
				yyerror("'.bss' size not absolute");
			if ($4.expval < 0L)
				yyerror("Invalid bss size");
			if (($2->styp & TYPE) != UNDEF)
				yyerror("Multiply defined label in bss");
			$2->styp &= EXTERN;
			$2->styp |= BSS;
			$2->value = newdot;
			newdot += $4.expval;
			cgsect(oldsect);
		}
	|	PSEVEN = {
			ckalign(2L);
		}
	|	dotbyte  exprlist = {
			if (numbits) {
				generate(8-numbits,DUMPBITS,8L,NULLSYM);
				numbits = 0;
			}
		}
	|	dotvalue  exprlist = {
			if (numbits) {
				generate(16-numbits,DUMPBITS,16L,NULLSYM);
				numbits = 0;
			}
		}
	|	PSSTRING STRING = {
			genstring ( $2 ) ;
		}
	/*
	 *	new types for initialisation of 80287 values
	 */

	|	dotlong fexprlist = {
		special287 = 0 ;
		}

	|	dotllong fexprlist = {
		special287 = 0 ;
		}

	|	dotfloat fexprlist = {
		special287 = 0 ;
		}

	|	dotdouble fexprlist = {
		special287 = 0 ;
		}

	|	dottemp fexprlist = {
		special287 = 0 ;
		}

	|	dotbcd fexprlist = {
		special287 = 0 ;
		}

	|	SHARP  NUMBER = {
			line = (unsigned short)($2) - 1;
		}
	|	SHARP  NUMBER  STRING = {
			line = (unsigned short)($2) - 1;
			strcpy(file,$3);
		}
	|	PSDEF  ID = {
			generate(0,DEFINE,NULLVAL,$2);
		}
	|	PSENDEF = {
			generate(0,ENDEF,NULLVAL,NULLSYM);
		}
	|	attrib  exprX = {
			generate(0,(unsigned short)$1,$2.expval,$2.symptr);
		}
	|	PSTAG  ID = {
			generate(0,SETTAG,NULLVAL,$2);
		}
	|	dotdim
	|	PSLN  exprX = {
			generate(0,LLINENO,$2.expval,$2.symptr);
		}
	|	PSLN  exprX  COMMA  exprX = {
			generate(0,LLINENUM,$2.expval,$2.symptr);
			generate(0,LLINEVAL,$4.expval,$4.symptr);
		}
	|	PSOPT = {
			/* .optim and .noopt assembler directives */
			/* temporarily turn on/off optimization of jumps */

			localopt = (short)($1->opcode);
		}
	|	PSJMPTAB = {

		generate($1->nbits,NOACTION,$1->opcode,NULLSYM);
		}
	|	PSIDENT STRING = {
			comment($2);
		}
	;

dotbyte	:	PSBYTE = {
			spctype = NBPW / 2;
		}
	;

dotvalue :	PSVALUE = {
			spctype = NBPW;
			if ((dot->styp != TXT) || !opt)
				ckalign(2L);
		}
	;

	/*
	 *	new terminals for 80287 initialisation
	 */

dotfloat:	PSFLOAT = {
		special287 = PSFLOAT ;
		}
	;

dotdouble:	PSDOUBLE = {
		special287 = PSDOUBLE ;
		}
	;

dottemp:	PSTEMP = {
		fprintf(stderr,"temp type not yet supported\n");
		special287 = PSTEMP ;
		}
	;

dotlong:	PSLONG = {
		special287 = PSLONG ;
		}
	;

dotllong:	PSLLONG = {
		fprintf(stderr,"llong type not yet supported\n");
		special287 = PSLLONG ;
		}
	;

dotbcd:		PSBCD = {
		special287 = PSBCD ;
		}
	;

attrib	:	PSVAL = {
			$$ = SETVAL;
		}
	|	PSSCL = {
			$$ = SETSCL;
		}
	|	PSTYPE = {
			$$ = SETTYP;
		}
	|	PSLINE = {
			$$ = SETLNO;
		}
	|	PSSIZE = {
			$$ = SETSIZ;
		}
	;

dotdim	:	PSDIM  exprX = {
			generate(0,SETDIM1,$2.expval,$2.symptr);
		}
	|	dotdim  COMMA  exprX = {
			generate(0,SETDIM2,$3.expval,$3.symptr);
		}
	;

	/*
	 *	special exprlist type for the 80287 initialisers
	 */

fexprlist:	fgexpr
	|	fexprlist {
				dot->value = newdot ;
				generate(0,NEWSTMT,(long)line,NULLSYM);
		} COMMA fgexpr
	;

fgexpr	:	FNUMBER {
		switch(special287) {

		case PSLONG:
		case PSFLOAT:
			width = 2 ;
			break ;
		case PSLLONG:
		case PSDOUBLE:
			width = 4 ;
			break ;
		case PSTEMP:
		case PSBCD:
			width = 5 ;
			break ;
		}
		for(action=0;action<width;action++)
			generate(16,SWAPB,(unsigned long)$1.fvala[action],NULLSYM);
		}
	|	MINUS FNUMBER {
		switch(special287) {
		case PSLONG:
			$2.fvall = 0 - $2.fvall ;
			width = 2 ;
			break ;
		case PSFLOAT:
			$2.fvala[1] |= 0x8000 ;
			width = 2 ;
			break ;
		case PSLLONG:
			width = 4 ;
			break ;
		case PSDOUBLE:
			$2.fvala[3] |= 0x8000 ;
			width = 4 ;
			break ;
		case PSTEMP:
			$2.fvala[4] |= 0x8000 ;
			width = 5 ;
			break ;
		case PSBCD:
			$2.fvala[4] |= 0x8000 ;
			width = 5 ;
			break ;
		}
		for(action=0;action<width;action++)
			generate(16,SWAPB,(unsigned long)$2.fvala[action],NULLSYM);
		}
	;

exprlist :	dgexpr
	|	exprlist {dot->value = newdot; generate(0,NEWSTMT,(long)line,NULLSYM); }  COMMA  dgexpr
	;

dgexpr	:	exprX = {
			if (bitpos > 0)
				yyerror("Expression crosses field boundary");
			/* Figure out which action routine to use   */
			/* in case there is an unresolved symbol.   */
			/* If a full word is being used, then       */
			/* a relocatable may be specified.          */
			/* Otherwise it is restricted to being an   */
			/* absolute (forward reference).            */
#if !iAPX286
			if (xflag && ($1.exptype & X86TYPE))
#else
			if ($1.exptype & X86TYPE)
#endif
				action = ($1.exptype & LO8TYPE) ? LOW8BITS :
					(($1.exptype & LO16TYPE) ? LO16BITS :
						HI12BITS);
			else
				action = actreloc($1.exptype,
					(spctype == NBPW) ? SWAPB :
					(($1.symptr != NULLSYM) ? RESABS :
						NOACTION));
			if((action==HI12BITS) && ($1.expval))
				yyerror("Illegal fixup generation");
			generate(spctype,action,$1.expval,$1.symptr);
		}
	|	NUMBER  COLON  exprX = {
			width = $1;
			numbits += width;
			if (bitpos + width > spctype)
				yyerror("Expression crosses field boundary");
			generate(width,(spctype == NBPW / 2) ?
				(unsigned short)PACK8 :
				(unsigned short)PACK16,$3.expval,$3.symptr);
		}
	;
%%

char yytext[1026];	/* must be the same size as "file" in errors.c */

static short type[] = {
	EOF,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	SP,	NL,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	SP,	XCLAIM,	QUOTE,	SHARP,	DOLLAR,	PERCNT,	AMP,	SQ,
	LP,	RP,	STAR,	PLUS,	COMMA,	MINUS,	ALPH,	SLASH,
	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,
	DIG,	DIG,	COLON,	SEMI,	LT,	EQ,	GT,	QUEST,
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	LB,	ESCAPE,	RB,	HAT,	ALPH,
	GRAVE,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	0,	OR,	0,	0,	0,
};

yylex()
{
	register short c,
		ctype;
	register char *yychrptr;
	register int insttype;	/* temporary holding place for inst. type */
	upsymins istp;
	static short mnemseen = NO; /* indicates if mnemonic seen yet */
	short base;
	long val;
	int escaped = 0 ;

	while (type[(c = getc(fdin)) + 1] == SP);

	switch (ctype = type[c+1]) {
		case ALPH:
			yychrptr = yytext;
			do {
#if FLEXNAMES
				if (yychrptr < &yytext[sizeof(yytext)])
#else
				if (yychrptr < &yytext[NCPS])
#endif
					*yychrptr++ = c;
			} while	((type[(c = getc(fdin))+1] == ALPH)
				|| (type[c+1] == DIG));
			*yychrptr = '\0';
			while (type[c + 1] == SP)
				c = getc(fdin);
			ungetc(c,fdin);
			if (c == ':')
				/* found a label; lookup USRNAME */
				istp = *lookup(yytext,INSTALL,USRNAME);
			else {
				if (mnemseen || (c == '='))
					istp = *lookup(yytext,INSTALL,USRNAME);
				else {
					istp = *lookup(yytext,N_INSTALL,MNEMON);
					if ((istp.stp == NULL) || ((istp.stp)->tag == 0))
						yyerror("Illegal mnemonic");
				}
				mnemseen = YES; /* set for next time */
			}
			if (istp.stp == NULL)
				return(ERR);
			if ((istp.stp)->tag == 0) {
				yylval.uusymptr = istp.stp;
				return(ID);
			}
			else {
				yylval.uuinsptr = istp.itp;
#if iAPX286
			/*
			 * This is a work around for a compiler bug
			 * (long switch constants are tested as ints).
			 * Replace it with the #else portion when fixed.
			 */
			{
				long l = istp.itp->opcode;

				if (
					l == 0x60L ||
					l == 0x61L ||
					l == 0x68L ||
					l == 0xc8L ||
					l == 0xc9L ||
					l == 0x6cL ||
					l == 0x6dL ||
					l == 0x6eL ||
					l == 0x6fL ||
					l == 0x6200L ||
					l == 0x6900L
				    ) flag186 = YES;
				else if (
					l == 0x0f06L ||
					l == 0x0f0110L ||
					l == 0x0f0100L ||
					l == 0x0f0118L ||
					l == 0x0f0108L ||
					l == 0x0f0010L ||
					l == 0x0f0000L ||
					l == 0x0f0018L ||
					l == 0x0f0008L ||
					l == 0x0f0130L ||
					l == 0x0f0120L ||
					l == 0x0f0200L ||
					l == 0x0f0300L ||
					l == 0x6300L ||
					l == 0x0f0020L ||
					l == 0x0f0028L
				     ) flag286 = YES;
			}
#else
				switch ( istp.itp->opcode ) {
					case 0x60L:
					case 0x61L:
					case 0x68L:
					case 0xc8L:
					case 0xc9L:
					case 0x6cL:
					case 0x6dL:
					case 0x6eL:
					case 0x6fL:
					case 0x6200L:
					case 0x6900L:
							flag186 = YES;
							break;
					case 0x0f06L:
					case 0x0f0110L:
					case 0x0f0100L:
					case 0x0f0118L:
					case 0x0f0108L:
					case 0x0f0010L:
					case 0x0f0000L:
					case 0x0f0018L:
					case 0x0f0008L:
					case 0x0f0130L:
					case 0x0f0120L:
					case 0x0f0200L:
					case 0x0f0300L:
					case 0x6300L:
					case 0x0f0020L:
					case 0x0f0028L:
							flag286 = YES;
							break;
					default:
							break;
				}
#endif
				insttype = istp.itp->val;
				/* since we have more than 127 tokens,
				any token values>127 placed in
				istp.itp->val will appear negative
				because istp.itp->val is byte-sized,
				so we have to compensate */
				insttype += (istp.itp->val > 0) ? 256 : 512;
				return(insttype);
			}

		case DIG:
			if(special287)
			{
				for( yychrptr =yytext;
				     yychrptr < &yytext[sizeof(yytext)-1];
				     ++yychrptr )
				{
				if ( (type[c+1] == DIG) ||
					( ('a' <= c) && (c <= 'f') ) ||
					( ('A' <= c) && (c <= 'F') ) ||
					( (c == 'x') || (c == 'X')
					  || (c == '.') || (c=='-')
					  || (c == '+') ) )
				{
					*yychrptr = c ;
					c = getc(fdin); 
				}
				else
				{
					*yychrptr = '\0' ;
					ungetc(c,fdin);
					if(c = conv287 ( special287 , 
						      yytext ,
						      yylval.uufval.fvala ) )
					{
						yyerror("conv287 errored");
					}
					return ( FNUMBER );
				}
				}
				yyerror("Constant too long");
				yylval.uufval.fvala[0] = 0 ;
				yylval.uufval.fvala[1] = 0 ;
				yylval.uufval.fvala[2] = 0 ;
				yylval.uufval.fvala[3] = 0 ;
				yylval.uufval.fvala[4] = 0 ;
				return(FNUMBER);
			}
			val = c - '0';
			if (c == '0') {
				c = getc(fdin);
				if (c == 'x' || c == 'X') {
					base = 16;
				} else if (c == 'b' || c == 'B') {
					base = 2;
				} else {
					ungetc(c,fdin);
					base = 8;
				}
			} else
				base = 10;
			while ( (type[(c = getc(fdin))+1] == DIG)
			    || ((base == 16) &&
				((('a'<=c) && (c<='f'))||(('A'<=c) && (c<='F')))))
			{
				if (base == 8)
					val <<= 3;
				else if (base == 10)
					val *= 10;
				else if (base == 2)
					val <<= 1;
				else
					val <<= 4;
				if ('a' <= c && c <= 'f')
					val += 10 + c - 'a';
				else if ('A' <= c && c <= 'F')
					val += 10 + c - 'A';
				else	val += c - '0';
			}
			ungetc(c,fdin);

			yylval.uulong = val;
			return(NUMBER);

		case PERCNT:
			yychrptr = yytext;
			while (type[(c = getc(fdin)) + 1] == ALPH) {
#if FLEXNAMES
				if (yychrptr < &yytext[sizeof(yytext)])
#else
				if (yychrptr < &yytext[NCPS])
#endif
					*yychrptr++ = c;
			};
			*yychrptr = '\0';
			ungetc(c,fdin);
			istp = *lookup(yytext,N_INSTALL,MNEMON);
			if ((istp.itp == NULL) || (istp.itp->tag == 0)) {
				yyerror("Illegal register");
				return(ERR);
			}
			yylval.uuinsptr = istp.itp;
			{
				int val = istp.itp->val;
				/* since we have more than 127 tokens,
				any token values>127 placed in
				istp.itp->val will appear negative
				because istp.itp->val is byte-sized,
				so we have to compensate */
			    	val += (istp.itp->val > 0) ? 256 : 512;
				return(val);
			}

		case SEMI:
			/* reinitialize for next statement */
			mnemseen = NO;
			return(SEMI);

		case SLASH:
			/* comment; skip rest of line */
			while (getc(fdin) != '\n');
			/* same as in case NL */
			/* no break */

		case NL:
			/* reinitialize for next statement */
			mnemseen = NO;
			return(NL);

		case ESCAPE:
			/* escaped operator */
			switch (type[getc(fdin) + 1]) {
				case STAR:
					return(MUL);
				case SLASH:
					return(DIV);
				case PERCNT:
					return(MOD);
				default:
					return(ERR);
			}

		case QUOTE:
			yychrptr = yytext;
			yylval.uustrptr = yychrptr;
			while ( c = getc(fdin) )
			{

				if( (c == '"') && (!escaped) )
					break;

				/*
				 * escaped toggle is set when character is
				 * preceeded by backslash
				 * it is used to no find enclosed double quotes
				 */

				escaped = ( !escaped && (c == '\\') ) ;

				if ( (c == '\n' ) )
				{
					yyerror("Unterminated string");
					ungetc(c,fdin) ;
					break;
				}
				if (yychrptr < &yytext[sizeof(yytext) - 1])
					*yychrptr++ = c;
			}

			*yychrptr = '\0';
			return(STRING);

		case GT:
			switch (getc(fdin)) {
			case '>':
				return (RSHIFT);
			default:
				return (ERR);
			}

		case LT:
			switch (getc(fdin)) {
			case '<':
				return(LSHIFT);
			case 's':
				ctype = SEGPART;
				break;
			case 'o':
				ctype = OFFPART;
				break;
			case 'h':
				ctype = HIFLOAT;
				break;
			case 'l':
				ctype = LOFLOAT;
				break;
			default:
				yyerror("Illegal character after '<'");
				return(ERR);
			}
			if ((c = getc(fdin)) != '>')
				yyerror("Missing '>'");
			if (ctype != HIFLOAT && ctype != LOFLOAT)
				return(ctype);
			/* accumulate floating point number */
			while (type[(c = getc(fdin)) + 1] == SP);
			for (yychrptr = yytext;
				yychrptr < &yytext[sizeof(yytext)-1];
				++yychrptr)
			{
				if (type[c+1]==DIG || c=='.'
					|| c=='+' || c=='-'
					|| c=='E' || c=='e')
				{
					*yychrptr = c;
					c = getc(fdin);
				}
				else {
					*yychrptr = '\0';
					ungetc(c,fdin);
					/* convert to b16 format */
					c = atob16f(yytext,&val);
					if (c) {
				yyerror("Error in floating point number");
						yylval.uulong = 0;
					}
					else
						/* take high or low part */
						yylval.uulong =
					(ctype == HIFLOAT)
					?	((val >> 16) & 0x0000FFFFL)
					:	(val & 0x0000FFFFL);
					return(NUMBER);
				} /* else */
			} /* for */
			yyerror("Floating point number too long");
			yylval.uulong = 0;
			return(NUMBER);

		case 0:
			yyerror("illegal character");
			return(ERR);

		default:
			return(ctype);
	}
} /* yylex */

fill(nbytes)
long nbytes;
{
	long fillchar;

	if (dot->styp != BSS) {
		fillchar = (dot->styp == TXT) ?  TXTFILL : FILL;
		while (nbytes--)
			generate(BITSPBY,NOACTION,fillchar,NULLSYM);
	}
	else
		newdot += nbytes;
} /* fill */

ckalign(size)
long size;
{
	long mod;
	if ((mod = newdot % size) != 0) {
		fill(size - mod);
	}
} /* ckalign */

/*
**	defsegprfx
**
**	uses memloc bits to decide default segment selector for
**	current instruction and returns it
**
**	This is used to remove unneccessary segment prefixes
*/

long
defsegprfx()
{
	if( memloc.addrmd.mod == 3 )		/* register */
		return -1L ;

	if( (memloc.addrmd.mod == 0) && (memloc.addrmd.rm == 6) )
		return 3L ;			/* ds */

	if( (memloc.addrmd.rm == 6)	/* xx(%bp) */
	 || (memloc.addrmd.rm == 2)	/* xx(%bp,%si) */
	 || (memloc.addrmd.rm == 3)	/* xx(%bp,%di) */
	  )
		return 2L ;			/* ss */

	return 3L ;		/* everything else is probably ds */
}
