/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)bits.c	1.3 - 85/08/09 */
/*
 */
#include	<stdio.h>
#include	"dis.h"
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"sgs.h"
#include	"scnhdr.h"

#define		RM	0	/* in case RMMR, is the value of the	*/
				/* 'd' bit that indicates an RM rather	*/
				/* than MR instruction			*/
#define		TWOlw	1	/* is value in 'l' and 'w' bits that	*/
				/* indicates a 2 byte immediate operand	*/
#define		TWOw	1	/* value of 'w' bit that indicates a	*/
				/* 2 byte operand			*/
#define		REGv	1	/* value of 'v' bit that indicates a	*/
				/* register needs to be included as an	*/
				/* operand				*/
#define		TYPEv	0	/* in case Iv for an interrupt;value of	*/
				/* 'v' bit which indicates the type of	*/
				/* interrupt				*/
#define		TYPE3	3	/* indicates a type of interrupt	*/
#define		DISP_0	0	/* indicates a 0 byte displacement	*/
#define		DISP_1	8	/* indicates a 1 byte displacement	*/
#define		DISP_2	16	/* indicates a 2 byte displacement	*/
#define		REG_ONLY 3	/* indicates a single register with	*/
				/* no displacement is an operand	*/
#define		MAXERRS	5	/* maximum # of errors allowed before	*/
				/* abandoning this disassembly as a	*/
				/* hopeless case			*/
#define		OPLEN	25	/* maximum length of a single operand	*/
static	char	operand[4][OPLEN];	/* to store operands as they	*/
					/* are encountered		*/
static	int	opindex;	/* will index into the 'operand' array	*/

static	int override;	/* to save the fact that an override segment	*/
			/* register request is outstanding.		*/
static char *overreg;	/* to save the override register		*/
static int errlev = 0;	/* to keep track of errors encountered during	*/
			/* the disassembly, probably due to being out	*/
			/* of sync.					*/
#define MASKw(x)	(x & 0x1)		/* to get w bit	*/
#define MASK3w(x)	( (x >> 3) & 0x1)	/* to get w bit from middle */
						/* where form is  'xxxxwxxx' */
#define MASKlw(x)	(x & 0x3)		/* to get 'lw' bits */
#define MASKreg(x)	(x & 0x7)		/* to get 3 bit register */
#define MASKv(x)	( (x >> 1) & 0x1)	/* to get 'v' bit */
#define MASKd(x)	( (x >> 1) & 0x1)	/* to get 'd' bit */
#define MASKseg(x)	( (x >> 3) & 0x3)	/* get seg reg from op code*/

#define R_M(x)		(x & 0x7)		/* get r/m field from byte */
						/* after op code */
#define REG(x)		( (x >> 3) & 0x7)	/* get register field from */
						/* byte after op code      */
#define MODE(x)		( (x >> 6) & 0x3)	/* get mode field from byte */
						/* after op code */
#define	LOW4(x)		( x & 0xf)		/* ----xxxx low 4 bits */
#define	BITS7_4(x)	( (x >> 4) & 0xf)	/* xxxx---- bits 7 to 4 */
#define	BITS43(x)	( (x >> 3) & 0x3)	/* ---xx--- bits 3 & 4 */
#if iAPX286
#define MASKret(x)	( (x >> 2) & 0x1)	/* return result bit
						for 287 instructions	*/
#endif
/*ftg dis_text () */
/*
 *	dis_text ()
 *
 *	disassemble a text section
 */

dis_text()

{
	/* the following 4 arrays are contained in _tbls.c	*/
	extern	struct	instable	distable[16][16];
#if iAPX286
	extern	struct	instable	op0F[8];
	extern	struct	instable	opfp1n2[8][8];
	extern	struct	instable	opfp3[8][8];
	extern	struct	instable	opfp4[4][8];
	extern	struct	instable	opfp5[8];
#endif
	extern	struct	addr		addrmod[8][4];
	extern	char	*regster[8][2];
	extern	char	*segreg[4];
	/* the following entries are from _extn.c	*/
	extern	SCNHDR	scnhdr;
	extern	char	*sname;
	extern	char	*fname;
	extern	LDFILE	*t_ptr;
	extern	int	Lflag;
	extern	int	oflag;
	extern	long	 loc;
	extern	char	mneu[];
	extern	char	object[];
	extern	unsigned short curbyte;
	extern	unsigned short cur2bytes;
	/* the following routines are in _utls.c	*/
	extern	int	printline();
	extern	int	looklabel();
	extern	int	line_nums();
	extern	int	prt_offset();
	extern	int	compoff();
	extern	int	convert();
	extern	short	sect_name();
	extern	int	getbyte();
	extern	int	lookbyte();

	SCNHDR		*sech;
	struct instable *dp, *submode;
	int w_default, twobytes;
	int dbit, wbit, vbit, lwbits;
	unsigned key1, key2, key3, mode, reg, r_m, temporary;
#if iAPX286
	unsigned key4, key5;
#endif
	unsigned short tmpshort;
	char	temp[NCPS+1];
	short	sect_num;
	long	lngval;

	/* initialization for each beginning of text disassembly	*/
	override = FALSE;
	w_default = 1;
	sech = &scnhdr;

	if (Lflag > 0)
		sect_num = sect_name();
	/*
	 * An instruction is disassembled with each iteration of the
	 * following loop.  The loop is terminated upon completion of the
	 * section (loc minus the section's physical address becomes equal
	 * to the section size) or if the number of bad op codes encountered
	 * would indicate this disassembly is hopeless.
	 */

	for (loc = sech->s_paddr; ((loc-sech->s_paddr) < sech->s_size) && (errlev < MAXERRS); printline()){
		if (Lflag > 0)
			looklabel(loc,sect_num);/* look for C source labels */
		line_nums(sech->s_nlnno);	/* print breakpoint line # */
		prt_offset();			/* print offset		   */
		opindex = 0;
		sprintf(operand[opindex],"");
		sprintf(operand[1],"");
	again:
		/* key1 and key2 are the first and second nibbles of	*/
		/* the op code, respectively				*/
		get_opcode(&key1, &key2);
		dp = &distable[key1][key2];
#if iAPX286
/* 286 instructions have 2 bytes of opcode before the mod_r/m byte	*/
/* so we need to perform a table indirection.				*/
		if (dp-> indirect == op0F) {
			get_opcode(&key4,&key5);
			if (key4 != 0 || key5 > 7) {
				bad_opcode();
				continue;
			}
			else
				dp = dp -> indirect + key5;
				/* only low 3 bits needed */
		}
#endif
		submode = dp -> indirect;
		if (dp -> indirect != TERM) {
			/* This must have been an opcode for which several
			 * instructions exist.  The key3 field (from the
			 * next byte) determines exactly which instruction
			 * this is.
			 */
			get_decode(&mode, &key3, &r_m);
#if iAPX286
/* figuring out the names for 287 instruction isn't as easy as just	*/
/* looking at the opcode byte and key3.  Let's simply say it's a mix	*/
/* of this and that.							*/
		/* 287 instructions fall between D8 and DF */
		if (key1 == 0xD && key2 >= 0x8) {
			if (key2 == 0xB && mode == 0x3 && key3 == 4)
			/* instruction form 5 */
				dp = &opfp5[r_m];
			else if (key2 == 0xB && mode == 0x3 && key3 > 4)
				bad_opcode();
			else if (key2 == 0x9 && mode == 0x3 && key3 >= 4)
			/* instruction form 4 */
				dp = &opfp4[key3-4][r_m];
			else if (key2 >= 8 && mode == 0x3)
			/* instruction form 3 */
				dp = &opfp3[key2-8][key3];
			else
			/* instruction form 1 and 2 */
				dp = &opfp1n2[key2-8][key3];
			}
		else {
#endif
			dp = dp -> indirect + key3;
			/* now dp points the proper subdecode table entry */
#if iAPX286
		}
#endif
		}
		sprintf(mneu,"%-7s",dp -> name);  /* print the mnemonic */

		/*
		 * Each instruction has a particular instruction syntax format
		 * stored in the disassembly tables.  The assignment of formats
		 * to instructins was made by the author.  Individual formats
		 * are explained as they are encountered in the following
		 * switch construct.
		 */

		switch(dp -> adr_mode){

		/* default segment register in next instruction will	*/
		/* be overridden.  This fact, along with the segment	*/
		/* register to be used, must be saved and recognized	*/
		/* later in the 'prtaddress' routine			*/
		case OVERRIDE:
			overreg = dp -> name;
			override = TRUE;
			goto again;		/* please forgive me */
		/* register to or from a memory or register operand,	*/
		/* based on the 'd' bit					*/
		case RMMR:
			dbit = MASKd(key2);
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			if (dbit == RM)
				sprintf(mneu,"%s%s,%s",mneu,regster[reg][wbit],operand[0]);
			else
				sprintf(mneu,"%s%s,%s",mneu,operand[0],regster[reg][wbit]);
			continue;
		/* added for 186 support; March 84; jws */
		case RMMRI:
			lwbits = MASKlw(key2 >> 2);
			twobytes = (lwbits == TWOlw) ? TRUE : FALSE;
			dbit = MASKd(key2);
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			opindex = 1;
			imm_data(twobytes,t_ptr);
			opindex = 0;
			if (dbit == RM)
				sprintf(mneu,"%s%s,%s,%s",mneu,operand[1],regster[reg][wbit],operand[0]);
			else
				sprintf(mneu,"%s%s,%s,%s",mneu,operand[1],operand[0],regster[reg][wbit]);
			continue;
		/* memory or register operand to register, with 'w' bit	*/
		case MRw:
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],regster[reg][wbit]);
			continue;
		/* register to memory or register operand, with 'w' bit	*/
		case RMw:
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress( mode, r_m, wbit);
			sprintf(mneu,"%s%s,%s",mneu,regster[reg][wbit],operand[0]);
			continue;

		/*
		 *	For the following two cases, one must read the
		 *	displacement (if present) and have it saved for
		 *	printing after the immediate operand.  This is
		 *	done by calling the 'saving_disp' routine.
		 *	For further documention see this routine.
		 */

		/* immediate to memory or register operand with both	*/
		/* 'l' and 'w' bits present				*/

		case IMlw:
			lwbits = MASKlw(key2);
			wbit = MASKw(key2);
			opindex = 2;
			saving_disp(mode, r_m);
			twobytes = (lwbits == TWOlw) ? TRUE : FALSE;
			opindex = 0;
			imm_data(twobytes, t_ptr);
			opindex = 1;
			ck_prt_override();
			/* operands are imm data, override, disp, regster */
			sprintf(mneu,"%s%s,%s%s%s",mneu,operand[0],operand[1],
				operand[2], (mode == REG_ONLY) ?
				regster[r_m][wbit] : addrmod[r_m][mode].regs);
			continue;
		/* immediate to memory or register operand with the	*/
		/* 'w' bit present					*/
		case IMw:
			wbit = MASKw(key2);
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			opindex = 2;
			saving_disp(mode, r_m);
			twobytes = (wbit == TWOw) ? TRUE : FALSE;
			opindex = 0;
			imm_data(twobytes, t_ptr);
			opindex = 1;
			ck_prt_override();
			sprintf(mneu,"%s%s,%s%s%s",mneu,operand[0],operand[1],
				operand[2], (mode == REG_ONLY) ?
				regster[r_m][wbit] : addrmod[r_m][mode].regs);
			continue;

		/* immediate to register with register in low 3 bits	*/
		/* of op code						*/
		case IR:
			wbit = (key2 >> 3) & 0x1;
			reg = MASKreg(key2);
			twobytes = (wbit == TWOw) ? TRUE : FALSE;
			imm_data(twobytes, t_ptr);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],regster[reg][wbit]);
			continue;

		/* memory operand to accumulator			*/
		case OA:
			wbit = MASKw(key2);
			twobytes = TRUE;
			displacement(twobytes, t_ptr);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],
				(wbit == TWOw) ? "%ax" : "%al");
			continue;

		/* accumulator to memory operand			*/
		case AO:
			wbit = MASKw(key2);
			twobytes = TRUE;
			displacement(twobytes, t_ptr);
			sprintf(mneu,"%s%s,%s",mneu, (wbit == TWOw) ?
				"%ax" : "%al", operand[0]);
			continue;
		/* memory or register operand to segment register	*/
		case MS:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],segreg[reg]);
			continue;

		/* segment register to memory or register operand	*/
		case SM:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			sprintf(mneu,"%s%s,%s",mneu,segreg[reg],operand[0]);
			continue;

		/* memory or register operand, which may or may not	*/
		/* go to the cl register, dependent on the 'v' bit	*/
		case Mv:
			vbit = MASKv(key2);
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			sprintf(mneu,"%s%s%s",mneu,(vbit == REGv) ? "%cl,":"",
				operand[0]);
			continue;
		/* added for 186 support; March 84; jws */
		case MvI:
			vbit = MASKv(key2);
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			twobytes = FALSE;
			opindex = 1;
			imm_data(twobytes,t_ptr);
			sprintf(mneu,"%s%s,%s%s",mneu,operand[1],(vbit == REGv) ? "%cl,":"",
				operand[0]);
			opindex = 0;
			continue;

		/* single memory or register operand with 'w' bit present*/
		case Mw:
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			continue;

		/* single memory or register operand			*/
		case M:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			continue;

		/* single register operand with register in the low 3	*/
		/* bits of op code					*/
		case R:
			reg = MASKreg(key2);
			sprintf(mneu,"%s%s",mneu,regster[reg][w_default]);
			continue;
		/* register to accumulator with register in the low 3	*/
		/* bits of op code					*/
		case RA:
			reg = MASKreg(key2);
			sprintf(mneu,"%s%s,%%ax",mneu,regster[reg][w_default]);
			continue;

		/* single segment register operand, with register in	*/
		/* bits 3-4 of op code					*/
		case SEG:
			reg = BITS43(curbyte);
			sprintf(mneu,"%s%s",mneu,segreg[reg]);
			continue;

		/* memory or register operand to register		*/
		case MR:
			if (submode == NULL)
				get_decode(&mode, &reg, &r_m);
			prtaddress(mode, r_m, w_default);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],regster[reg][w_default]);
			continue;

		/* immediate operand to accumulator			*/
		case IA:
			wbit = MASKw(key2);
			twobytes = (wbit == TWOw) ? TRUE : FALSE;
			imm_data(twobytes, t_ptr);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],
				(twobytes == TRUE) ? "%ax" : "%al");
			continue;

		/* memory or register operand to accumulator		*/
		case MA:
			wbit = MASKw(key2);
			prtaddress( mode, r_m, wbit);
			sprintf(mneu,"%s%s,%s",mneu, operand[0],
				(wbit == TWOw) ? "%ax" : "%al");
			continue;

		/* si register to di register				*/
		case SD:
			ck_prt_override();
			sprintf(mneu,"%s%s(%%si),(%%di)",mneu,operand[0]);
			continue;

		/* accumulator to di register				*/
		case AD:
			wbit = MASKw(key2);
			ck_prt_override();
			sprintf(mneu,"%s%s,%s(%%di)",mneu,(wbit == TWOw) ?
				"%ax" : "%al",operand[0]);
			continue;

		/* si register to accumulator				*/
		case SA:
			wbit = MASKw(key2);
			ck_prt_override();
			sprintf(mneu,"%s%s(%%si),%s",mneu,operand[0],
				(wbit == TWOw) ? "%ax" : "%al");
			continue;

		/* single operand, a 16 bit displacement		*/
		/* added to current offset by 'compoff'			*/
		case D:
			twobytes = TRUE;
			displacement(twobytes, t_ptr);
			lngval = (cur2bytes & 0x8000) ? cur2bytes | ~0xffffL :
							cur2bytes & 0xffffL;
			compoff(lngval, operand[1]);
			sprintf(mneu,"%s%s%s",mneu,operand[0],
				(cur2bytes == 0) ? "" : operand[1]);
			continue;

		/* indirect to memory or register operand		*/
		case INM:
			prtaddress(mode, r_m, w_default);
			sprintf(mneu,"%s*%s",mneu,operand[0]);
			continue;
		/* for long jumps and long calls -- a new code segment	*/
		/* register and an offset in IP -- stored in object	*/
		/* code in reverse order				*/
		case SO:
			twobytes = TRUE;
			opindex = 1;
			displacement(twobytes, t_ptr);
			opindex = 0;
			/* will now get segment operand*/
			displacement(twobytes, t_ptr);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			continue;

		/* single operand, 8 bit displacement			*/
		/* added to current offset in 'compoff'			*/
		case BD:
			twobytes = FALSE;
			displacement(twobytes, t_ptr);
			lngval = (curbyte & 0x80) ? curbyte | ~0xffL :
						    curbyte & 0xffL;
			compoff(lngval, operand[1]);
			sprintf(mneu,"%s%s%s",mneu, operand[0],
				(curbyte == 0) ? "" : operand[1]);
			continue;

		/* single 16 bit immediate operand			*/
		case I:
			twobytes = TRUE;
			imm_data(twobytes,t_ptr);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			continue;

		/* single 8 bit immediate operand			*/
		case Ib:
			twobytes = FALSE;
			imm_data(twobytes,t_ptr);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			continue;

		/* added for 186 support; March 84; jws */
		case II:
			twobytes = TRUE;
			imm_data(twobytes,t_ptr);
			twobytes = FALSE;
			opindex = 1;
			imm_data(twobytes,t_ptr);
			opindex = 0;
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			continue;

		/* single 8 bit port operand				*/
		case P:
			twobytes = FALSE;
			displacement(twobytes, t_ptr);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			continue;

		/* single operand, dx register (variable port instruction)*/
		case V:
			ck_prt_override();
			sprintf(mneu,"%s%s(%%dx)",mneu,operand[0]);
			continue;
		/* operand is either 3 or else the next 8 bits,		*/
		/* dependent on the 'v' bit (indicates type of interrupt)*/
		case Iv:
			vbit = MASKw(key2);
			if (vbit == TYPEv) {
				temporary = TYPE3;
				convert(temporary, temp, LEAD);
				sprintf(mneu,"%s$%s",mneu,temp);
			}
			else {
				twobytes = FALSE;
				imm_data(twobytes,t_ptr);
				sprintf(mneu,"%s%s",mneu,operand[0]);
			}
			continue;

		/* an unused byte must be discarded			*/
		case U:
			getbyte(t_ptr);
			continue;

		/* no disassembly, the mnemonic was all there was	*/
		/* so go on						*/
		case GO_ON:
			continue;

		/* Special byte indicating a the beginning of a 	*/
		/* jump table has been seen. The jump table addresses	*/
		/* will be printed until the address 0xffff which	*/
		/* indicates the end of the jump table is read.		*/
		case JTAB:
			sprintf(mneu,"***JUMP TABLE BEGINNING***");
			printline();
			prt_offset();
			lookbyte(t_ptr);
			if (curbyte == FILL) {
				if (Lflag > 0)
	   				looklabel(loc,sect_num);
	   			line_nums(sech->s_nlnno);
				sprintf(mneu,"FILL BYTE FOR ALIGNMENT");
				sprintf(object,"%s90",object);
				printline();
				prt_offset();
				printf("\t");
				lookbyte(t_ptr);
				tmpshort = curbyte;
				lookbyte(t_ptr);
				(oflag)? sprintf(object,"%s%03o %03o",
						object,curbyte,tmpshort):
				         sprintf(object,"%s%02x %02x",
						object,curbyte,tmpshort);
			}
			else {
				tmpshort = curbyte;
				lookbyte(t_ptr);
				printf("\t");
				(oflag)? sprintf(object,"%s%03o %03o",
						object,curbyte,tmpshort):
				         sprintf(object,"%s%02x %02x",
						object,curbyte,tmpshort);
			}
			sprintf(mneu,"");
			while ((curbyte != 0x00ff) || (tmpshort != 0x00ff)) {
				printline();
				prt_offset();
				printf("\t");
				lookbyte(t_ptr);
				tmpshort = curbyte;
				lookbyte(t_ptr);
				(oflag)? sprintf(object,"%s%03o %03o",
						object,curbyte,tmpshort):
				         sprintf(object,"%s%02x %02x",
						object,curbyte,tmpshort);
			}
			sprintf(mneu,"***JUMP TABLE END***");
			continue;

#if iAPX286
		/* float reg */
		case F:
			sprintf(mneu,"%s%%st(%1.1d)",mneu,r_m);
			continue;

		/* float reg to float reg, with ret bit present */
		case FF:
			if ( MASKret(key2) )
				/* st -> st(i) */
				sprintf(mneu,"%s%%st,%%st(%1.1d)",mneu,r_m);
			else
				/* st(i) -> st */
				sprintf(mneu,"%s%%st(%1.1d),%%st",mneu,r_m);
			continue;
#endif

		/* an invalid op code (there aren't too many)	*/
		case UNKNOWN:
			bad_opcode();
			continue;

		default:
			printf("%sdis bug: notify implementor:",SGS);
			printf(" case from instruction table not found");
			exit(4);
			break;
		} /* end switch */
	}  /* end of for */

	if (errlev >= MAXERRS) { /* just a newsy, informative note */
		printf("%sdis: %s: %s: section probably not text section\n",
			SGS,fname, sname);
		printf("\tdisassembly terminated\n");
		exit(4);
	}
}
/*ftg get_decode () */
/*
 *	get_decode (mode, reg, r_m)
 *
 *	Get the byte following the op code and separate it into the
 *	mode, register, and r/m fields.
 */

get_decode(mode, reg, r_m)
unsigned	*mode;
unsigned	*reg;
unsigned	*r_m;

{
	extern	int	getbyte();	/* in _utls.c */
	extern	unsigned short curbyte;	/* in _extn.c */
	extern	int	convert();	/* in _utls.c */
	extern	LDFILE	*t_ptr;		/* pointer from _extn.c */

	char	temp[NCPS+1];

	getbyte(t_ptr);

	*r_m = R_M(curbyte);
	*reg = REG(curbyte);
	*mode = MODE(curbyte);

}
/*ftg ck_prt_override () */
/*
 *	ck_prt_override ()
 *
 *	Check to see if there is a segment override prefix pending.
 *	If so, print it in the current 'operand' location and set
 *	the override flag back to false.
 */

ck_prt_override()
{
	if (override == FALSE)
		return;

	sprintf(operand[opindex],"%s",overreg);
	override = FALSE;
}
/*ftg displacement () */
/*
 *	displacement (twobytes, ptr)
 *
 *	Get and print in the 'operand' array a one or two byte displacement.
 */


displacement(twobytes, ptr)
int	twobytes;
LDFILE	*ptr;

{
	extern	int		getbyte();	/* in _utls.c */
	extern	unsigned short	curbyte;	/* in _extn.c */
	extern	unsigned short	cur2bytes;	/* in _extn.c */
	extern	int		convert();	/* in _utls.c */
	extern	char		mneu[];		/* in _extn.c */
	extern	int		oflag;		/* in _extn.c */

	unsigned	short	bytehigh;
	unsigned	short	bytelow;
	char		templow[NCPS+1];
	char		temphigh[NCPS+1];
	char	temp[(NCPS*2)+1];

	getbyte(ptr);
	bytelow = curbyte;

	if (twobytes == TRUE){
		getbyte(ptr);
		bytehigh = curbyte;
		bytehigh = (bytehigh << 8) | bytelow;
		/* if location if to be computed by 'compoff', the */
		/* displacement will be around in 'cur2bytes'	*/
		cur2bytes = bytehigh;

		if (curbyte == 0) {	/* if high byte 00, print leading 0 */
			if (oflag)
				sprintf(temp,"00%o",bytehigh);
			else
				sprintf(temp,"0x0%x",bytehigh);
		}
		else {
			convert(bytehigh, temphigh, LEAD);
			sprintf(temp,"%s",temphigh);
		}
	}

	else {	/* have a one byte displacement */
		convert(bytelow, templow, LEAD);
		sprintf(temp,"%s",templow);
	}

	ck_prt_override();
	sprintf(operand[opindex],"%s%s",operand[opindex],temp);
}
/*ftg prtaddress () */
/*
 *	prtaddress (mode, r_m, wbit)
 *
 *	Print the registers used to find the address mode,  checking 
 *	to see if there was a segment override prefix present.
 */

prtaddress(mode, r_m, wbit)
unsigned	mode;
unsigned	r_m;
int		wbit;

{
	extern	struct	addr	addrmod[8][4]; /* in _tbls.c */
	extern	char 	*regster[8][2];	 /* in _tbls.c */
	extern	LDFILE	*t_ptr;		 /* pointer from _extn.c */

	int	twobytes;

	/* check for a segment override prefix 	*/

	ck_prt_override();

	if (addrmod[r_m][mode].disp == DISP_1){
		twobytes = FALSE;
		displacement(twobytes, t_ptr);
	}
	else if (addrmod[r_m][mode].disp == DISP_2){
		twobytes = TRUE;
		displacement(twobytes, t_ptr);
	}

	sprintf(operand[opindex],"%s%s",operand[opindex], (mode == REG_ONLY) ?
		regster[r_m][wbit] : addrmod[r_m][mode].regs);
}
/*ftg saving_disp () */
/*
 *	saving_disp (mode, r_m)
 *
 *	If a one or two byte displacement is needed, call 'displacement'
 *	to both read and save it in the current 'operand' location
 *	for printing in the mnuemonic output.
 */


saving_disp(mode, r_m)
unsigned	mode;
unsigned	r_m;

{
	extern	struct	addr	addrmod[8][4]; /* in _tbls.c */
	extern	LDFILE	*t_ptr;		/* pointer from _main.c */

	int	twobytes;

	sprintf(operand[opindex],"");
	if (addrmod[r_m][mode].disp == DISP_0)
		return;

	twobytes = (addrmod[r_m][mode].disp == DISP_1) ?
			FALSE : TRUE;

	displacement(twobytes, t_ptr);
}
/*ftg imm_data () */
/*
 *	imm_data (twobytes, ptr)
 *
 *	Determine if 1 or 2 bytes of immediate data are needed, then
 *	get and print them.  The bytes will be in reverse order
 *	(i.e., 'byte low    byte high ') in the object code.
 */


imm_data(twobytes, ptr)
int	twobytes;
LDFILE	*ptr;

{
	extern	int	getbyte();	/* from _utls.c */
	extern	int	convert();	/* from _utls.c */
	extern	unsigned short curbyte;	/* from _extn.c */

	unsigned	short	bytelow, bytehigh;
	char		temphigh[NCPS+1];
	char		templow[NCPS+1];

	getbyte(ptr);
	bytelow = curbyte;

	if (twobytes == TRUE) {
		getbyte(ptr);
		bytehigh = curbyte;
		bytehigh = (bytehigh << 8) | bytelow;
		convert(bytehigh, temphigh, LEAD);
		sprintf(operand[opindex],"$%s",temphigh);
	}

	else {
		convert(bytelow, templow, LEAD);
		sprintf(operand[opindex],"$%s",templow);
	}
}
/*ftg get_opcode () */
/*
 *	get_opcode (high, low)
 *
 *	Get the next byte and separate the op code into the high and
 *	low nibbles.
 */

get_opcode(high, low)
unsigned *high;
unsigned *low;		/* low 4 bits of op code   */

{

	extern	LDFILE	*t_ptr;			/* from _extn.c */
	extern	unsigned short curbyte;		/* from _extn.c */
	unsigned short	byte;
	char	temp[NCPS+1];

	getbyte(t_ptr);
	byte = curbyte;
	*low = LOW4(byte);
	*high = BITS7_4(byte);
}

/* 	bad_opcode	*/
/* 	print message and try to recover */

bad_opcode()
{
	sprintf(mneu,"***ERROR--unknown op code***");
	printline();	/* to print the error message	*/
	/* attempt to resynchronize */
	if (resync() == FAILURE)	/* if cannot recover */
		errlev++;		/* stop eventually.  */
	sprintf(object,""); /* to prevent extraneous printing when */
	sprintf(mneu,"");   /* continuing to the 'for' loop iteration */
}
