/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)tables.c	1.3 - 85/08/09 */
/*
 */
/*
*	Modified March 84 to add 186 instructions
*/
#include	"dis.h"

#define		NONE	0	/* to indicate register only in addrmod array*/
#define		INVALID	{"",TERM,UNKNOWN}

/*
 *	Register operands may be indicated by a distinguished field.
 *	An '8' bit register is selected if the 'w' bit is equal to 0,
 *	and a '16' bit register is selected if the 'w' bit is equal to
 *	1 and also if there is no 'w' bit.
 */

char	*regster[16] = {

/* w bit		0		1		*/

/* reg bits */
/* 000	*/		"%al",		"%ax",
/* 001  */		"%cl",		"%cx",
/* 010  */		"%dl",		"%dx",
/* 011	*/		"%bl",		"%bx",
/* 100	*/		"%ah",		"%sp",
/* 101	*/		"%ch",		"%bp",
/* 110	*/		"%dh",		"%si",
/* 111	*/		"%bh",		"%di",

};


/*
 *	This initialized array will be indexed by the 'r/m' and 'mod'
 *	fields, to determine the addressing mode, and will also provide
 *	strings for printing.
 */

struct addr	addrmod [32] = {

/* mod		00			01			10			11 */
/* r/m */
/* 000 */	{0,"(%bx,%si)"},	{8,"(%bx,%si)"},	{16,"(%bx,%si)"},	{NONE,"%ax"},
/* 001 */	{0,"(%bx,%di)"},	{8,"(%bx,%di)"},	{16,"(%bx,%di)"},	{NONE,"%cx"},
/* 010 */	{0,"(%bp,%si)"},	{8,"(%bp,%si)"},	{16,"(%bp,%si)"},	{NONE,"%dx"},
/* 011 */	{0,"(%bp,%di)"},	{8,"(%bp,%di)"},	{16,"(%bp,%di)"},	{NONE,"%bx"},
/* 100 */	{0,"(%si)"},		{8,"(%si)"},		{16,"(%si)"},		{NONE,"%sp"},
/* 101 */	{0,"(%di)"},		{8,"(%di)"},		{16,"(%di)"},		{NONE,"%bp"},
/* 110 */	{16,""},		{8,"(%bp)"},		{16,"(%bp)"},		{NONE,"%si"},
/* 111 */	{0,"(%bx)"},		{8,"(%bx)"},		{16,"(%bx)"},		{NONE,"%di"},
};
/*
 *	Segment registers are selected by a two bit field.
 */

char	*segreg[4] = {

/* 00 */	"%es",
/* 01 */	"%cs",
/* 10 */	"%ss",
/* 11 */	"%ds",

};


/*
 *	Decode table for 0x0F00 opcodes
 */

struct instable op0F00[8] = {

/*  [0]  */	{"sldt",TERM,M},	{"str",TERM,M},	{"lldt",TERM,M},	{"ltr",TERM,M},
/*  [4]  */	{"verr",TERM,M},	{"verw",TERM,M},	INVALID,	INVALID,
};


/*
 *	Decode table for 0x0F01 opcodes
 */

struct instable op0F01[8] = {

/*  [0]  */	{"sgdt",TERM,M},	{"sidt",TERM,M},	{"lgdt",TERM,M},	{"lidt",TERM,M},
/*  [4]  */	{"smsw",TERM,M},	INVALID,	{"lmsw",TERM,M},	INVALID,
};


/*
 *	Decode table for 0x0F opcodes
 */

struct instable op0F[8] = {

/*  [00]  */	{"",op0F00,TERM},	{"",op0F01,TERM},	{"lar",TERM,MR},	{"lsl",TERM,MR},
/*  [04]  */	INVALID,	INVALID,	{"clts",TERM,GO_ON},	INVALID,
};


/*
 *	Decode table for 0x80 opcodes
 */

struct instable op80[8] = {

/*  [0]  */	{"addb",TERM,IMlw},	{"orb",TERM,IMw},	{"adcb",TERM,IMlw},	{"sbbb",TERM,IMlw},
/*  [4]  */	{"andb",TERM,IMw},	{"subb",TERM,IMlw},	{"xorb",TERM,IMw},	{"cmpb",TERM,IMlw},
};


/*
 *	Decode table for 0x81 opcodes.
 */

struct instable op81[8] = {

/*  [0]  */	{"add",TERM,IMlw},	{"or",TERM,IMw},	{"adc",TERM,IMlw},	{"sbb",TERM,IMlw},
/*  [4]  */	{"and",TERM,IMw},	{"sub",TERM,IMlw},	{"xor",TERM,IMw},	{"cmp",TERM,IMlw},
};


/*
 *	Decode table for 0x82 opcodes.
 */

struct instable op82[8] = {

/*  [0]  */	{"addb",TERM,IMlw},	INVALID,		{"adcb",TERM,IMlw},	{"sbbb",TERM,IMlw},
/*  [4]  */	INVALID,		{"subb",TERM,IMlw},	INVALID,		{"cmpb",TERM,IMlw},
};
/*
 *	Decode table for 0x83 opcodes.
 */

struct instable op83[8] = {

/*  [0]  */	{"add",TERM,IMlw},	INVALID,		{"adc",TERM,IMlw},	{"sbb",TERM,IMlw},
/*  [4]  */	INVALID,		{"sub",TERM,IMlw},	INVALID,		{"cmp",TERM,IMlw},
};

/*
 *	Decode table for 0xC0 opcodes.
 *	186 instruction set
 */

struct instable opC0[8] = {

/*  [0]  */	{"rolb",TERM,MvI},	{"rorb",TERM,MvI},	{"rclb",TERM,MvI},	{"rcrb",TERM,MvI},
/*  [4]  */	{"shlb",TERM,MvI},	{"shrb",TERM,MvI},	INVALID,		{"sarb",TERM,MvI},
};

/*
 *	Decode table for 0xD0 opcodes.
 */

struct instable opD0[8] = {

/*  [0]  */	{"rolb",TERM,Mv},	{"rorb",TERM,Mv},	{"rclb",TERM,Mv},	{"rcrb",TERM,Mv},
/*  [4]  */	{"shlb",TERM,Mv},	{"shrb",TERM,Mv},	INVALID,		{"sarb",TERM,Mv},
};

/*
 *	Decode table for 0xC1 opcodes.
 *	186 instruction set
 */

struct instable opC1[8] = {

/*  [0]  */	{"rol",TERM,MvI},	{"ror",TERM,MvI},	{"rcl",TERM,MvI},	{"rcr",TERM,MvI},
/*  [4]  */	{"shl",TERM,MvI},	{"shr",TERM,MvI},	INVALID,		{"sar",TERM,MvI},
};

/*
 *	Decode table for 0xD1 opcodes.
 */

struct instable opD1[8] = {

/*  [0]  */	{"rol",TERM,Mv},	{"ror",TERM,Mv},	{"rcl",TERM,Mv},	{"rcr",TERM,Mv},
/*  [4]  */	{"shl",TERM,Mv},	{"shr",TERM,Mv},	INVALID,		{"sar",TERM,Mv},
};


/*
 *	Decode table for 0xD2 opcodes.
 */

struct instable opD2[8] = {

/*  [0]  */	{"rolb",TERM,Mv},	{"rorb",TERM,Mv},	{"rclb",TERM,Mv},	{"rcrb",TERM,Mv},
/*  [4]  */	{"shlb",TERM,Mv},	{"shrb",TERM,Mv},	INVALID,		{"sarb",TERM,Mv},
};
/*
 *	Decode table for 0xD3 opcodes.
 */

struct instable opD3[8] = {

/*  [0]  */	{"rol",TERM,Mv},	{"ror",TERM,Mv},	{"rcl",TERM,Mv},	{"rcr",TERM,Mv},
/*  [4]  */	{"shl",TERM,Mv},	{"shr",TERM,Mv},	INVALID,		{"sar",TERM,Mv},
};


/*
 *	Decode table for 0xF6 opcodes.
 */

struct instable opF6[8] = {

/*  [0]  */	{"testb",TERM,IMw},	INVALID,		{"notb",TERM,Mw},	{"negb",TERM,Mw},
/*  [4]  */	{"mulb",TERM,MA},	{"imulb",TERM,MA},	{"divb",TERM,MA},	{"idivb",TERM,MA},
};


/*
 *	Decode table for 0xF7 opcodes.
 */

struct instable opF7[8] = {

/*  [0]  */	{"test",TERM,IMw},	INVALID,		{"not",TERM,Mw},	{"neg",TERM,Mw},
/*  [4]  */	{"mul",TERM,MA},	{"imul",TERM,MA},	{"div",TERM,MA},	{"idiv",TERM,MA},
};


/*
 *	Decode table for 0xFE opcodes.
 */

struct instable opFE[8] = {

/*  [0]  */	{"incb",TERM,Mw},	{"decb",TERM,Mw},	INVALID,		INVALID,
/*  [4]  */	INVALID,		INVALID,		INVALID,		INVALID,
};
/*
 *	Decode table for 0xFF opcodes.
 */

struct instable opFF[8] = {

/*  [0]  */	{"inc",TERM,Mw},	{"dec",TERM,Mw},	{"call",TERM,INM},	{"lcall",TERM,INM},
/*  [4]  */	{"jmp",TERM,INM},	{"ljmp",TERM,INM},	{"push",TERM,M},	INVALID,
};

#if iAPX286
/* for 287 instructions, which are a mess to decode */

struct instable opfp1n2[64] = {
/* bit pattern:	1101 1xxx MODxx xR/M */
/*  [0,0] */	{"fadds",TERM,M},	{"fmuls",TERM,M},	{"fcoms",TERM,M},	{"fcomps",TERM,M},
/*  [0,4] */	{"fsubs",TERM,M},	{"fsubrs",TERM,M},	{"fdivs",TERM,M},	{"fdivrs",TERM,M},
/*  [1,0]  */	{"flds",TERM,M},	INVALID,	{"fsts",TERM,M},	{"fstps",TERM,M},
/*  [1,4]  */	{"fldenv",TERM,M},	{"fldcw",TERM,M},	{"fnstenv",TERM,M},	{"fnstcw",TERM,M},
/*  [2,0]  */	{"fiaddl",TERM,M},	{"fimull",TERM,M},	{"ficoml",TERM,M},	{"ficompl",TERM,M},
/*  [2,4]  */	{"fisubl",TERM,M},	{"fisubrl",TERM,M},	{"fidivl",TERM,M},	{"fidivrl",TERM,M},
/*  [3,0]  */	{"fildl",TERM,M},	INVALID,	{"fistl",TERM,M},	{"fistpl",TERM,M},
/*  [3,4]  */	INVALID,	{"fldt",TERM,M},	INVALID,	{"fstpt",TERM,M},
/*  [4,0]  */	{"faddl",TERM,M},	{"fmull",TERM,M},	{"fcoml",TERM,M},	{"fcompl",TERM,M},
/*  [4,1]  */	{"fsubl",TERM,M},	{"fsubrl",TERM,M},	{"fdivl",TERM,M},	{"fdivrl",TERM,M},
/*  [5,0]  */	{"fldl",TERM,M},	INVALID,	{"fstl",TERM,M},	{"fstpl",TERM,M},
/*  [5,4]  */	{"frstor",TERM,M},	INVALID,	{"fnsave",TERM,M},	{"fnstsw",TERM,M},
/*  [6,0]  */	{"fiadd",TERM,M},	{"fimul",TERM,M},	{"ficom",TERM,M},	{"ficomp",TERM,M},
/*  [6,4]  */	{"fisub",TERM,M},	{"fisubr",TERM,M},	{"fidiv",TERM,M},	{"fidivr",TERM,M},
/*  [7,0]  */	{"fild",TERM,M},	INVALID,	{"fist",TERM,M},	{"fistp",TERM,M},
/*  [7,4]  */	{"fbld",TERM,M},	{"fildll",TERM,M},	{"fbstp",TERM,M},	{"fistpl",TERM,M},
};

struct instable opfp3[64] = {
/* bit  pattern:	1101 1xxx 11xx xREG */
/*  [0,0]  */	{"fadd",TERM,FF},	{"fmul",TERM,FF},	{"fcom",TERM,F},	{"fcomp",TERM,F},
/*  [0,4]  */	{"fsub",TERM,FF},	{"fsubr",TERM,FF},	{"fdiv",TERM,FF},	{"fdivr",TERM,FF},
/*  [1,0]  */	{"fld",TERM,F},	{"fxch",TERM,F},	{"fnop",TERM,GO_ON},	{"fstp",TERM},
/*  [1,4]  */	INVALID,	INVALID,	INVALID,	INVALID,
/*  [2,0]  */	INVALID,	INVALID,	INVALID,	INVALID,
/*  [2,4]  */	INVALID,	INVALID,	INVALID,	INVALID,
/*  [3,0]  */	INVALID,	INVALID,	INVALID,	INVALID,
/*  [3,4]  */	INVALID,	INVALID,	INVALID,	INVALID,
/*  [4,0]  */	{"fadd",TERM,FF},	{"fmul",TERM,FF},	{"fcom",TERM,F},	{"fcomp",TERM,F},
/*  [4,4]  */	{"fsub",TERM,FF},	{"fsubr",TERM,FF},	{"fdiv",TERM,FF},	{"fdivr",TERM,FF},
/*  [5,0]  */	{"ffree",TERM,F},	{"fxch",TERM,F},	{"fst",TERM,F},	{"fstp",TERM,F},
/*  [5,4]  */	INVALID,	INVALID,	INVALID,	INVALID,
/*  [6,0]  */	{"faddp",TERM,FF},	{"fmulp",TERM,FF},	{"fcomp",TERM,F},	{"fcompp",TERM,GO_ON},
/*  [6,4]  */	{"fsubp",TERM,FF},	{"fsubrp",TERM,FF},	{"fdivp",TERM,FF},	{"fdivrp",TERM,FF},
/*  [7,0]  */	{"ffree",TERM,F},	{"fxch",TERM,F},	{"fstp",TERM,F},	{"fstp",TERM,F},
/*  [7,4]  */	{"fstsw",TERM,M},	INVALID,	INVALID,	INVALID,
};

struct instable opfp4[32] = {
/* bit pattern:	1101 1001 111x xxxx */
/*  [0,0]  */	{"fchs",TERM,GO_ON},	{"fabs",TERM,GO_ON},	INVALID,	INVALID,
/*  [0,4]  */	{"ftst",TERM,GO_ON},	{"fxam",TERM,GO_ON},	INVALID,	INVALID,
/*  [1,0]  */	{"fld1",TERM,GO_ON},	{"fldl2t",TERM,GO_ON},	{"fldl2e",TERM,GO_ON},	{"fldpi",TERM,GO_ON},
/*  [1,4]  */	{"fldlg2",TERM,GO_ON},	{"fldln2",TERM,GO_ON},	{"fldz",TERM,GO_ON},	INVALID,
/*  [2,0]  */	{"f2xm1",TERM,GO_ON},	{"fyl2x",TERM,GO_ON},	{"fptan",TERM,GO_ON},	{"fpatan",TERM,GO_ON},
/*  [2,4]  */	{"fxtract",TERM,GO_ON},	INVALID,	{"fdecstp",TERM,GO_ON},	{"fincstp",TERM,GO_ON},
/*  [3,0]  */	{"fprem",TERM,GO_ON},	{"fyl2xp1",TERM,GO_ON},	{"fsqrt",TERM,GO_ON},	INVALID,
/*  [3,4]  */	{"frndint",TERM,GO_ON},	{"fscale",TERM,GO_ON},	INVALID,	INVALID,
};

struct instable opfp5[8] = {
/* bit pattern:	1101 1011 1110 0xxx */
/*  [0]  */	INVALID,	INVALID,	{"fnclex",TERM,GO_ON},	{"fninit",TERM,GO_ON},
/*  [4]  */	{"fsetpm",TERM,GO_ON},	INVALID,	INVALID,	INVALID,
};

#endif

/*
 *	Main decode table for the op codes.  The first two nibbles
 *	will be used as an index into the table.  If there is a
 *	a need to further decode an instruction, the array to be
 *	referenced is indicated with the other two entries being
 *	empty.
 */

struct instable distable[256] = {

/* [0,0] */	{"addb",TERM,RMMR},	{"add",TERM,RMMR},	{"addb",TERM,RMMR},	{"add",TERM,RMMR},
/* [0,4] */	{"addb",TERM,IA},	{"add",TERM,IA},	{"push",TERM,SEG},	{"pop",TERM,SEG},
/* [0,8] */	{"orb",TERM,RMMR},	{"or",TERM,RMMR},	{"orb",TERM,RMMR},	{"or",TERM,RMMR},
/* [0,C] */	{"orb",TERM,IA},	{"or",TERM,IA},		{"push",TERM,SEG},	{"",op0F,TERM},

/* [1,0] */	{"adcb",TERM,RMMR},	{"adc",TERM,RMMR},	{"adcb",TERM,RMMR},	{"adc",TERM,RMMR},
/* [1,4] */	{"adcb",TERM,IA},	{"adc",TERM,IA},	{"push",TERM,SEG},	{"pop",TERM,SEG},
/* [1,8] */	{"sbbb",TERM,RMMR},	{"sbb",TERM,RMMR},	{"sbbb",TERM,RMMR},	{"sbb",TERM,RMMR},
/* [1,C] */	{"sbbb",TERM,IA},	{"sbb",TERM,IA},	{"push",TERM,SEG},	{"pop",TERM,SEG},

/* [2,0] */	{"andb",TERM,RMMR},	{"and",TERM,RMMR},	{"andb",TERM,RMMR},	{"and",TERM,RMMR},
/* [2,4] */	{"andb",TERM,IA},	{"and",TERM,IA},	{"%es:",TERM,OVERRIDE},	{"daa",TERM,GO_ON},
/* [2,8] */	{"subb",TERM,RMMR},	{"sub",TERM,RMMR},	{"subb",TERM,RMMR},	{"sub",TERM,RMMR},
/* [2,C] */	{"subb",TERM,IA},	{"sub",TERM,IA},	{"%cs:",TERM,OVERRIDE},	{"das",TERM,GO_ON},

/* [3,0] */	{"xorb",TERM,RMMR},	{"xor",TERM,RMMR},	{"xorb",TERM,RMMR},	{"xor",TERM,RMMR},
/* [3,4] */	{"xorb",TERM,IA},	{"xor",TERM,IA},	{"%ss:",TERM,OVERRIDE},	{"aaa",TERM,GO_ON},
/* [3,8] */	{"cmpb",TERM,RMMR},	{"cmp",TERM,RMMR},	{"cmpb",TERM,RMMR},	{"cmp",TERM,RMMR},
/* [3,C] */	{"cmpb",TERM,IA},	{"cmp",TERM,IA},	{"%ds:",TERM,OVERRIDE},	{"aas",TERM,GO_ON},

/* [4,0] */	{"inc",TERM,R},		{"inc",TERM,R},		{"inc",TERM,R},		{"inc",TERM,R},
/* [4,4] */	{"inc",TERM,R},		{"inc",TERM,R},		{"inc",TERM,R},		{"inc",TERM,R},
/* [4,8] */	{"dec",TERM,R},		{"dec",TERM,R},		{"dec",TERM,R},		{"dec",TERM,R},
/* [4,C] */	{"dec",TERM,R},		{"dec",TERM,R},		{"dec",TERM,R},		{"dec",TERM,R},

/* [5,0] */	{"push",TERM,R},	{"push",TERM,R},	{"push",TERM,R},	{"push",TERM,R},
/* [5,4] */	{"push",TERM,R},	{"push",TERM,R},	{"push",TERM,R},	{"push",TERM,R},
/* [5,8] */	{"pop",TERM,R},		{"pop",TERM,R},		{"pop",TERM,R},		{"pop",TERM,R},
/* [5,C] */	{"pop",TERM,R},		{"pop",TERM,R},		{"pop",TERM,R},		{"pop",TERM,R},

/* [6,0] */	{"pusha",TERM,GO_ON},	{"popa",TERM,GO_ON}	,	{"bound",TERM,MR},		{"arpl",TERM,RMw},
/* [6,4] */	INVALID,		INVALID,		INVALID,		INVALID,
/* [6,8] */	{"push",TERM,I},		{"imul",TERM,RMMRI},		{"push",TERM,Ib},		{"imul",TERM,RMMRI},
/* [6,C] */	{"insb",TERM,GO_ON},	{"ins",TERM,GO_ON},	{"outsb",TERM,GO_ON},	{"outs",TERM,GO_ON},

/* [7,0] */	{"jo",TERM,BD},		{"jno",TERM,BD},	{"jb",TERM,BD},		{"jae",TERM,BD},
/* [7,4] */	{"je",TERM,BD},		{"jne",TERM,BD},	{"jbe",TERM,BD},	{"ja",TERM,BD},
/* [7,8] */	{"js",TERM,BD},		{"jns",TERM,BD},	{"jp",TERM,BD},		{"jnp",TERM,BD},
/* [7,C] */	{"jl",TERM,BD},		{"jge",TERM,BD},	{"jle",TERM,BD},	{"jg",TERM,BD},
/* [8,0] */	{"",op80,TERM},		{"",op81,TERM},		{"",op82,TERM},		{"",op83,TERM},
/* [8,4] */	{"testb",TERM,MRw},	{"test",TERM,MRw},	{"xchgb",TERM,MRw},	{"xchg",TERM,MRw},
/* [8,8] */	{"movb",TERM,RMMR},	{"mov",TERM,RMMR},	{"movb",TERM,RMMR},	{"mov",TERM,RMMR},
/* [8,C] */	{"mov",TERM,SM},	{"lea",TERM,MR},	{"mov",TERM,MS},	{"pop",TERM,M},

/* [9,0] */	{"xchg",TERM,RA},	{"xchg",TERM,RA},	{"xchg",TERM,RA},	{"xchg",TERM,RA},
/* [9,4] */	{"xchg",TERM,RA},	{"xchg",TERM,RA},	{"xchg",TERM,RA},	{"xchg",TERM,RA},
/* [9,8] */	{"cbw",TERM,GO_ON},	{"cwd",TERM,GO_ON},	{"lcall",TERM,SO},	{"wait",TERM,GO_ON},
/* [9,C] */	{"pushf",TERM,GO_ON},	{"popf",TERM,GO_ON},	{"sahf",TERM,GO_ON},	{"lahf",TERM,GO_ON},

/* [A,0] */	{"movb",TERM,OA},	{"mov",TERM,OA},	{"movb",TERM,AO},	{"mov",TERM,AO},
/* [A,4] */	{"smovb",TERM,SD},	{"smov",TERM,SD},	{"scmpb",TERM,SD},	{"scmp",TERM,SD},
/* [A,8] */	{"testb",TERM,IA},	{"test",TERM,IA},	{"sstob",TERM,AD},	{"ssto",TERM,AD},
/* [A,C] */	{"slodb",TERM,SA},	{"slod",TERM,SA},	{"scab",TERM,AD},	{"sca",TERM,AD},

/* [B,0] */	{"movb",TERM,IR},	{"movb",TERM,IR},	{"movb",TERM,IR},	{"movb",TERM,IR},
/* [B,4] */	{"movb",TERM,IR},	{"movb",TERM,IR},	{"movb",TERM,IR},	{"movb",TERM,IR},
/* [B,8] */	{"mov",TERM,IR},	{"mov",TERM,IR},	{"mov",TERM,IR},	{"mov",TERM,IR},
/* [B,C] */	{"mov",TERM,IR},	{"mov",TERM,IR},	{"mov",TERM,IR},	{"mov",TERM,IR},

/* [C,0] */	{"",opC0,TERM},		{"",opC1,TERM},		{"ret",TERM,I},		{"ret",TERM,GO_ON},
/* [C,4] */	{"les",TERM,MR},	{"lds",TERM,MR},	{"movb",TERM,IMw},	{"mov",TERM,IMw},
/* [C,8] */	{"enter",TERM,II},		{"leave",TERM,GO_ON},		{"lret",TERM,I},	{"lret",TERM,GO_ON},
/* [C,C] */	{"int",TERM,Iv},	{"int",TERM,Iv},	{"into",TERM,GO_ON},	{"iret",TERM,GO_ON},

/* [D,0] */	{"",opD0,TERM},		{"",opD1,TERM},		{"",opD2,TERM},		{"",opD3,TERM},
/* [D,4] */	{"aam",TERM,U},		{"aad",TERM,U},		{"falc",TERM,GO_ON},	{"xlat",TERM,GO_ON},

#if iAPX286
/* 287 instructions.  Note that although the indirect field		*/
/* indicates opfp1n2 for further decoding, this is not necessarily	*/
/* the case since the opfp arrays are not partitioned according to key1	*/
/* and key2.  opfp1n2 is given only to indicate that we haven't		*/
/* finished decoding the instruction.					*/
/* [D,8] */	{"",opfp1n2,TERM},		{"",opfp1n2,TERM},		{"",opfp1n2,TERM},		{"",opfp1n2,TERM},
/* [D,C] */	{"",opfp1n2,TERM},		{"",opfp1n2,TERM},		{"",opfp1n2,TERM},		{"",opfp1n2,TERM},
#else
/* [D,8] */	{"esc",TERM,U},		{"esc",TERM,U},		{"esc",TERM,U},		{"esc",TERM,U},
/* [D,C] */	{"esc",TERM,U},		{"esc",TERM,U},		{"esc",TERM,U},		{"esc",TERM,U},
#endif

/* [E,0] */	{"loopnz",TERM,BD},	{"loopz",TERM,BD},	{"loop",TERM,BD},	{"jcxz",TERM,BD},
/* [E,4] */	{"inb",TERM,P},		{"in",TERM,P},		{"outb",TERM,P},	{"out",TERM,P},
/* [E,8] */	{"call",TERM,D},	{"jmp",TERM,D},		{"ljmp",TERM,SO},	{"jmp",TERM,BD},
/* [E,C] */	{"inb",TERM,V},		{"in",TERM,V},		{"outb",TERM,V},	{"out",TERM,V},

/* [F,0] */	{"lock",TERM,GO_ON},	{"",TERM,JTAB},	{"repnz",TERM,GO_ON},	{"rep",TERM,GO_ON},
/* [F,4] */	{"hlt",TERM,GO_ON},	{"cmc",TERM,GO_ON},	{"",opF6,TERM},		{"",opF7,TERM},
/* [F,8] */	{"clc",TERM,GO_ON},	{"stc",TERM,GO_ON},	{"cli",TERM,GO_ON},	{"sti",TERM,GO_ON},
/* [F,C] */	{"cld",TERM,GO_ON},	{"std",TERM,GO_ON},	{"",opFE,TERM},		{"",opFF,TERM},
};

