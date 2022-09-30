%{
/*	Copyright (c) 1985 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

%{
/* %W% - %E% */
%}

%{

# include "optim.h"
# include "sgs.h"

# define numops 279
# define numoprs 49

extern boolean
	pothole(),
	blackhole(),
	slithole(),
	peephole();

extern int
	maxm,
	sflag;

boolean	inswitch = false;

char	*deflab,
	*operand;

char	*valcat(),
	*fugdef = NULL,
	*newlab(),
	*lookopr();

char	fixstr[256];

NODE	*savep;

int	mflag = false,
	mbits,
	mvalue;

char *optbl[numops] =
	{ "aaa", "aad", "aam", "aas", "adc", "adcb", "add", "addb",
	"and", "andb", "arpl", "bound", "call", "cbw", "clc", "cld",
	"cli", "clr", "clrb", "cmc", "cmp", "cmpb", "cts", "cwd",
	"daa", "das", "dec", "decb", "div", "divb", "enter", "esc",
	"f2xm1", "fabs", "fadd", "faddl", "faddp", "fadds", "fbld", "fbstp",
	"fchs", "fclex", "fcom", "fcoml", "fcomp", "fcompl", "fcompp", "fcomps",
	"fcoms", "fdecstp", "fdiv", "fdivl", "fdivp", "fdivr", "fdivrl", "fdivrp",
	"fdivrs", "fdivs", "ffree", "fiadd", "fiaddl", "ficom", "ficoml", "ficomp",
	"ficompl", "fidiv", "fidivl", "fidivr", "fidivrl", "fild", "fildl", "fildll",
	"fimul", "fimull", "fincstp", "finit", "fist", "fistl", "fistp", "fistpl",
	"fistpll", "fisub", "fisubl", "fisubr", "fisubrl", "fld", "fld1", "fldcw",
	"fldenv", "fldl", "fldl2e", "fldl2t", "fldlg2", "fldln2", "fldpi", "flds",
	"fldt", "fldz", "fmul", "fmull", "fmulp", "fmuls", "fnclex", "fninit",
	"fnop", "fnsave", "fnstcw", "fnstenv", "fnstsw", "fpatan", "fprem", "fptan",
	"frndint", "frstor", "fsave", "fscale", "fsetpm", "fsqrt", "fst", "fstcw",
	"fstenv", "fstl", "fstp", "fstpl", "fstps", "fstpt", "fsts", "fstsw",
	"fsub", "fsubl", "fsubp", "fsubr", "fsubrl", "fsubrp", "fsubrs", "fsubs",
	"ftst", "fwait", "fxam", "fxch", "fxtract", "fyl2x", "fyl2xp1", "hlt",
	"idiv", "idivb", "imul", "imulb", "in", "inb", "inc", "incb",
	"ins", "int", "into", "iret", "ja", "jae", "jb", "jbe",
	"jcxz", "je", "jg", "jge", "jl", "jle", "jmp", "jna",
	"jnae", "jnb", "jnbe", "jne", "jng", "jnge", "jnl", "jnle",
	"jno", "jnp", "jns", "jnz", "jo", "jp", "jpe", "jpo",
	"js", "jz", "lahf", "lar", "lcall", "lds", "lea", "leave",
	"les", "lgdt", "lidt", "ljmp", "lldt", "lmsw", "lock", "loop",
	"loope", "loopne", "loopnz", "loopz", "lret", "lsl", "ltr", "mov",
	"movb", "mul", "mulb", "neg", "negb", "not", "notb", "or",
	"orb", "out", "outb", "outs", "pop", "popa", "popf", "push",
	"pusha", "pushf", "rcl", "rclb", "rcr", "rcrb", "rep", "repnz",
	"repz", "ret", "rol", "rolb", "ror", "rorb", "sahf", "sal",
	"salb", "sar", "sarb", "sbb", "sbbb", "sca", "scab", "scmp",
	"scmpb", "sgdt", "shl", "shlb", "shr", "shrb", "sidt", "sldt",
	"slod", "slodb", "smov", "smovb", "smsw", "ssto", "sstob", "st",
	"stc", "std", "sti", "str", "sub", "subb", "test", "testb",
	"verr", "wait", "xchg", "xchgb", "xlat", "xor", "xorb"};

char *oprtbl[] = {
		"$0", "$1", "$16", "$2", "$3",
		"$4", "$5", "$6", "$8", "$9",
		"%ah", "%al", "%ax", "%bh", "%bl",
		"%bp", "%bx", "%ch", "%cl", "%cs",
		"%cx", "%dh", "%di", "%dl", "%ds",
		"%dx", "%es", "%si", "%sp", "%ss",
		"(%di)", "(%si)", "-10(%bp)", "-12(%bp)", "-14(%bp)",
		"-16(%bp)", "-18(%bp)", "-2(%bp)", "-20(%bp)", "-22(%bp)",
		"-24(%bp)", "-4(%bp)", "-6(%bp)", "-8(%bp)", "10(%bp)",
		"2(%di)", "2(%si)", "4(%bp)", "6(%bp)", "8(%bp)",
		NULL};

int	fixlen,
	opn,
	m;

int	ln = IDVAL,
	retreg = AX|DX;	/* assume the worst! */

%}
%START T TT T1 T2 D TS TS1 TN
%%
<T>^[ \t]*\.set[ \t]**\.F.*\n	{fprintf(yyout, "	.text\n");
				 setauto(yytext);
				 endfunct() ; 
				 ECHO; }
<T>^[ \t]*\.set[ \t]*\.T.*	{setret(yytext); appfl(yytext,yyleng+1);}
<T>^[ \t]*\.set[ \t]*\.S.*	{		 appfl(yytext,yyleng+1);}
<T>^[ \t]*\.set.*		{appfl(yytext,yyleng+1);}
<T>^[ \t]*\.data.*\n	{ECHO; BEGIN D;}
<T>^[ \t]*\.bss.*\n	{ECHO; BEGIN D;}
<T>^[ \t]*\.file.*\n	{ECHO;}
<T>^[ \t]*\.tv.*\n		{ECHO;}
<T>^[ \t]*\.jmpbeg.*	{applbl(yytext,yyleng+1);
			lastnode->op = HLABEL;
			inswitch = true ;
			lastnode->uniqid = IDVAL;}
<T>^[ \t]*\.jmpend.*	{inswitch=false;appmisc(yytext+1,yyleng);}
<T>^[ \t]*\.text.*		;
<T>^[ \t]*\.globl.*		{appfl(yytext,yyleng+1);}
<T>^[ \t]*\.ln.*		{getln(yytext);}
<T>^[ \t]*\.value[ \t]*		{BEGIN TS;}
<T>^[ \t]*\.even.*		{applbl(yytext,yyleng+1); lastnode->op = HLABEL;
			 lastnode->uniqid = IDVAL;}
<T>^[^ \t:\n]+:		{yytext[yyleng-1] = '\0'; applbl(yytext,yyleng);
			 if(inswitch)
				lastnode->op = HLABEL ;
			 lastnode->uniqid = IDVAL;}
<T>^[ \t]+\/.*\n		;
<T>^\/.*\n			;
<T>^[ \t]+		{BEGIN TT;}
<T>\n			;
<TT>\.def.*scl[ \t]*-1;.*	{strcpy(fugdef = (char *)getspace(yyleng+1),
				yytext); BEGIN T;}
<TT>\.def.*val[ \t]*\.;.*	{deflab = newlab(); deffix(yytext,deflab);
			 appfl(valcat("\t",fixstr),fixlen+2); 
			 applbl(deflab,length(deflab)); lastnode->op = HLABEL;
			 lastnode->uniqid = IDVAL; BEGIN T;}
<TT>\.def.*		{appfl(valcat("\t",yytext),yyleng+2); BEGIN T;}
<TT>jmp\t\*\%cs:	{saveop(0,optbl[166],0,JMP); BEGIN TS1;}
<TT>ljmp\t\*\%cs:	{saveop(0,optbl[195],0,LJMP);BEGIN TS1;}
<TT>[a-z12]+		{opn = lookopc(yytext);
			 if (opn == OTHER) saveop(0,yytext,yyleng+1,opn);
			else {saveop(0,optbl[m],0,opn);}
			 opn = 1; BEGIN T1;}
<T1>[ \t]*?-?[0-9]*"(%bp,%"[ds]"i)"		{squash(); saveop(opn,yytext,yyleng+1,0); opn++;
			 BEGIN T2;}
<T1>[^,/\n]*		{squash(); operand=lookopr(yytext);
			 if(opn == 3 && mul3(operand)) ;
			 else if (operand != NULL) saveop(opn,operand,0,0);
			 else saveop(opn,yytext,yyleng+1,0);
			 opn++; BEGIN T2;}
<T1>(\/.*)?\n		{for(;opn <= 2; opn++) saveop(opn, NULL, 0, 0);
			 setln(); BEGIN T;}
<T2>","			{BEGIN T1;}
<T2>(\/.*)?\n		{for(;opn <= 2; opn++) saveop(opn, NULL, 0, 0);
			 setln(); BEGIN T;}
<D>^[ \t]*\.text		{BEGIN T;}
<D>^[ \t]*\.string[ \t]*	{ECHO;BEGIN TN;}
<D>\/.*			;
<TS>\.[0-9]+		{appmisc(valcat(".value\t",yytext), yyleng+8);
			 addref(yytext, yyleng+1); lastnode->uniqid = IDVAL; BEGIN T;}
<TS>.			{appmisc(valcat(".value\t", yytext), yyleng+8);
			  lastnode->uniqid = IDVAL; BEGIN T;}
<TS1>\.[0-9]+[^,/\n]*	{squash(); saveop(1, valcat("*%cs:", yytext), yyleng+6, 0);
			 saveop(2, NULL, 0, 0); fndref(yytext);
			 lastnode->uniqid = IDVAL; BEGIN T2;}
<TN>\n 		 {ECHO;
			 BEGIN D;}
%%
yyinit(flags) char * flags; {

	BEGIN T;
	for (; *flags != '\0'; flags++)
		if (*flags == 'V')
			fprintf( stderr, "%s: optimizer - %s\n",
				SGSNAME, RELEASE );
		else if (*flags == 'x') {/* mult strength reduction */
			int i;
			mflag = true;
			mvalue = 0;
			while ( isdigit( *++flags ))
				mvalue = mvalue * 10 + *flags - '0';
			flags--;
			if ( mvalue == 0 )
				mvalue = 15;
			for ( i = mvalue, mbits = 0; i; i >>= 1, mbits++);
		}
		else
			fprintf(stderr, "Optimizer: invalid flag '%c'\n",*flags);
	}

extern int model;

lookopc(op) char *op; { /* look up op code ordinal */
	register int f,l;
	int om,x;

	static short opcodes[numops] = 
	{ AAA, AAD, AAM, AAS, ADC, ADCB, ADD, ADDB,
	AND, ANDB, ARPL, BOUND, CALL, CBW, CLC, CLD,
	CLI, CLR, CLRB, CMC, CMP, CMPB, CTS, CWD,
	DAA, DAS, DEC, DECB, DIV, DIVB, ENTER, ESC,
	F2XM1, FABS, FADD, FADDL, FADDP, FADDS, FBLD, FBSTP,
	FCHS, FCLEX, FCOM, FCOML, FCOMP, FCOMPL, FCOMPP, FCOMPS,
	FCOMS, FDECSTP, FDIV, FDIVL, FDIVP, FDIVR, FDIVRL, FDIVRP,
	FDIVRS, FDIVS, FFREE, FIADD, FIADDL, FICOM, FICOML, FICOMP,
	FICOMPL, FIDIV, FIDIVL, FIDIVR, FIDIVRL, FILD, FILDL, FILDLL,
	FIMUL, FIMULL, FINCSTP, FINIT, FIST, FISTL, FISTP, FISTPL,
	FISTPLL, FISUB, FISUBL, FISUBR, FISUBRL, FLD, FLD1, FLDCW,
	FLDENV, FLDL, FLDL2E, FLDL2T, FLDLG2, FLDLN2, FLDPI, FLDS,
	FLDT, FLDZ, FMUL, FMULL, FMULP, FMULS, FNCLEX, FNINIT,
	FNOP, FNSAVE, FNSTCW, FNSTENV, FNSTSW, FPATAN, FPREM, FPTAN,
	FRNDINT, FRSTOR, FSAVE, FSCALE, FSETPM, FSQRT, FST, FSTCW,
	FSTENV, FSTL, FSTP, FSTPL, FSTPS, FSTPT, FSTS, FSTSW,
	FSUB, FSUBL, FSUBP, FSUBR, FSUBRL, FSUBRP, FSUBRS, FSUBS,
	FTST, FWAIT, FXAM, FXCH, FXTRACT, FYL2X, FYL2XP1, HLT,
	IDIV, IDIVB, IMUL, IMULB, IN, INB, INC, INCB,
	INS, INT, INTO, IRET, JA, JAE, JB, JBE,
	JCXZ, JE, JG, JGE, JL, JLE, JMP, JNA,
	JNAE, JNB, JNBE, JNE, JNG, JNGE, JNL, JNLE,
	JNO, JNP, JNS, JNZ, JO, JP, JPE, JPO,
	JS, JZ, LAHF, LAR, LCALL, LDS, LEA, LEAVE,
	LES, LGDT, LIDT, LJMP, LLDT, LMSW, LOCK, LOOP,
	LOOPE, LOOPNE, LOOPNZ, LOOPZ, LRET, LSL, LTR, MOV,
	MOVB, MUL, MULB, NEG, NEGB, NOT, NOTB, OR,
	ORB, OUT, OUTB, OUTS, POP, POPA, POPF, PUSH,
	PUSHA, PUSHF, RCL, RCLB, RCR, RCRB, REP, REPNZ,
	REPZ, RET, ROL, ROLB, ROR, RORB, SAHF, SAL,
	SALB, SAR, SARB, SBB, SBBB, SCA, SCAB, SCMP,
	SCMPB, SGDT, SHL, SHLB, SHR, SHRB, SIDT, SLDT,
	SLOD, SLODB, SMOV, SMOVB, SMSW, SSTO, SSTOB, ST,
	STC, STD, STI, STR, SUB, SUBB, TEST, TESTB,
	VERR, WAIT, XCHG, XCHGB, XLAT, XOR, XORB};

	f = 0;
	l = numops;
	om = 0;
	m = (f+l)/2;
	while (m != om) {
		x = strcmp(op,optbl[m]);
		if (x == 0){
			if(opcodes[m] == LRET) model = 1; /* get model type from return */
			return(opcodes[m]);
			}
		else if (x < 0)
			l = m-1;
		    else
			f = m+1;
		om = m;
		m = (f+l)/2;
		}
	return(OTHER);
	}

char *
lookopr(op) char *op; {	/* lookup operands - save space by matching common ones */
	register f,l,m;
	int om,x;

	f = 0;
	l = numoprs;
	om = 0;
	m = (f + l)/2;
	while (m != om) {
		x = strcmp(op, oprtbl[m]);
		if (x == 0)
			return(oprtbl[m]);
		else if (x < 0)
			l = m - 1;
		     else
			f = m + 1;
		om = m;
		m = (f + l)/2;
		}
	return(NULL);
	}

squash() { /* remove spaces and tabs from yytext and fix yyleng */

	register char *f,*t;

	f = yytext;
	t = yytext;
	while (*f != '\0') {
		if (*f == ' ' || *f == '\t')
			yyleng--;
		    else {
			*t++ = *f;
			}
		f++;
		}
	*t = '\0';
	}

deffix(str,lbl) register char *str, *lbl; { /* change "val .;" to "val ..n;" */

	register char *nc;
	boolean scanning;

	scanning = true;
	nc = fixstr;
	fixlen = 0;
	while (scanning)
		if (*str == '.' && *(str+1) == ';') {
			str++;
			scanning = false;
			}
		    else {
			*nc++ = *str++;
			fixlen++;
			}
	while (*nc++ = *lbl++)
		fixlen++;
	nc--;
	while (*nc++ = *str++)
		fixlen++;
	}


length(str) register char *str; { /* return length of string (including \0) */

	register int l;

	l = 1;
	while (*str++)
		l++;
	return (l);
	}

getln(str)
char *str;
{
	if (ln != IDVAL && ln != 1)
		fprintf(stderr, "Optimizer: Warning: failed to attach line number %d\n", ln);
	if ( !sscanf(str + 4, "%d", &ln) ) {
		fprintf(stderr, "Optimizer: can't parse:%s\n", str);
		ln = 0;
	}
}

setln()
{
	lastnode->uniqid = ln;
	ln = IDVAL;
}

prdef()		/* print possible trailing .def of fcn for field update */
{
	if( fugdef != NULL )
		fprintf(yyout,"	%s\n", fugdef);
}

wrapup() { /* print unprocessed text and update statistics file */

	if (n0.forw != NULL) {
		addtail(lastnode);
		fprintf(yyout, "	.text\n");
		prtext();
	}
}

char *
valcat(cp1,cp2) char *cp1, *cp2; { /* return *cp1 in front of *cp2 */

	static char t[YYLMAX];
	register char *tp;

	tp = t;
	while (*tp++ = *cp1++) ;
	tp--;
	while (*tp++ = *cp2++) ;
	return(t);
}

/* reference label hidden in .nn(%...) */
fndref(cp)
	register char *cp;
{
	char t[10];
	register char *tp;

	tp = t;
	while ((*tp++ = *cp++) != '(');
	*--tp = '\0';
	addref(t, tp - t + 1);
}

/*
**	endfunct
**
**	called after parsing each complete function
**
*/

extern int ndisc ;	/* count (held in optim.c) of number of discarded
			   instructions */
extern int nodeswap;	/* number of nodes swapped */

endfunct ()
{
	int locallast = -1;
	int lastndisc;
	int lastnodeswap;

	filter () ;

	do {
		lastndisc = ndisc ;

		if( locallast != ndisc) {
			ldanal () ;
			locallast = ndisc;
		}
		optim () ;			/* will also call  bboptim */
		rtanal () ;			/* register tracking */
		if( locallast != ndisc) {/* don't do ldanal if no */
			ldanal () ;	  /* instructions discarded */
					  /* since last ldanal */
			locallast = ndisc;
		}
		window ( 4 , pothole  ) ;
		if( locallast != ndisc) {
			ldanal () ;
			locallast = ndisc;
		}
		window ( 1 , slithole ) ;
		if( locallast != ndisc) {
			ldanal () ;
			locallast = ndisc;
		}
		window ( 2 ,blackhole ) ;
		if( locallast != ndisc || lastnodeswap != nodeswap) {
			ldanal () ;
			locallast = ndisc;
			lastnodeswap = nodeswap;
		}
		window ( 3 , peephole ) ;

	} while ( ndisc > lastndisc ) ; /* ie until no more discarded instr. */

	prtext () ;
	prdef () ;
	init () ;

	ln = IDVAL ;
	retreg = AX | DX ;
	fugdef = NULL ;
}

yywrap()
{
	return(1);
}

/*
**	prinst
**
**	print instruction
**
*/

prinst(p) NODE *p; 
{
	if (p->uniqid != IDVAL)
		fprintf(yyout, "	.ln	%d\n", p->uniqid);
	switch (p->op) {
	case LABEL:
		fprintf(yyout, "%s:\n", p->opcode);
		break;
	case HLABEL:
		if ( !strcmp ( p->opcode , "	.jmpbeg") )
			fprintf(yyout, "%s\n", p->opcode);
		else
			fprintf(yyout, "%s:\n", p->opcode);
		break;
	case MISC:
		fprintf(yyout, "	%s\n", p->opcode);
		break;
	case JMP:
		if (p->op1 == NULL) {
			p->op = RET;
		}
	default:
		fprintf(yyout, "	%s	", p->opcode);
		if (p->op1 != NULL)
			fprintf(yyout, "%s", p->op1);
		if (p->op2 != NULL)
			fprintf(yyout, ",%s", p->op2);
		if (p->op == IMUL3A )
			fprintf(yyout,",%%ax");
		else if (p->op == IMUL3B )
			fprintf(yyout,",%%bx");
		else if (p->op == IMUL3C )
			fprintf(yyout,",%%cx");
		else if (p->op == IMUL3D )
			fprintf(yyout,",%%dx");
		else if (p->op == IMUL3S )
			fprintf(yyout,",%%si");
		else if (p->op == IMUL3DI )
			fprintf(yyout,",%%di");
		fprintf(yyout, "\n");
	}
}

mul3(oprnd) char *oprnd; 
{
	if( lastnode->op == IMUL ) {
		if(!strcmp("%ax",oprnd) )
			lastnode->op = IMUL3A;
		else if(!strcmp("%bx",oprnd) )
			lastnode->op = IMUL3B;
		else if(!strcmp("%cx",oprnd) )
			lastnode->op = IMUL3C;
		else if(!strcmp("%dx",oprnd) )
			lastnode->op = IMUL3D;
		else if(!strcmp("%si",oprnd) )
			lastnode->op = IMUL3S;
		else if(!strcmp("%di",oprnd) )
			lastnode->op = IMUL3DI;
		else return(0);
		return(1);
	}
	return(0);
}
