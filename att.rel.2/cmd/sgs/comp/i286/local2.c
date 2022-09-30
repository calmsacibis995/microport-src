/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)local2.c	1.6 - 85/09/25 */

# include "mfile2"

extern char *strcpy();

# ifndef ONEPASS
short revrel[] = {EQ, NE, GE, GT, LE, LT, UGE, UGT, ULE, ULT};
# endif

extern long stcount;/* used to determine time of push onto coprocessor stack*/

where(c)

int c;
{	/* print the location of an error */

	fprintf(stderr, "%s, line %d: ", filename, lineno);
}

lineid(l, fn)

int l;
char *fn;
{	/* identify line l and file fn
	 * this is activated by the -l flag to back
	 * not the same as the .ln pseudos, this is editor line number
	 */

	printf("/\tline %d, file %s\n", l, fn);
}


eobl2()
{	/* this is calculated at the end of every function, the .F# is unique.
	 * spoff is the sum of the number of autos and temporaries used
	 * in the function
	 */

	OFFSZ spoff;		/* offset from stack pointer */
	register int nregs;	/* number of registers used to */
	int i, svalue;
	char *cp;
				/* contain the result of this function */
#ifdef ONEPASS
	extern TWORD ftntype;
#else
	extern CONSZ rdin();
	TWORD ftntype;

	ftntype = (TWORD) rdin(8);
#endif

	if( ( (model >= MDL_LARGE) &&
	      ( ISPTR(ftntype) || (ftntype == STRTY) ) ) ||
	    ( (model == MDL_MIDDLE) &&
	      ISPTR(ftntype) && ISFTN(ftntype >> TSHIFT) ) )
		nregs = 2;
	else
		nregs = szty(ftntype);

	printf("\t.set\t.T%d,%d\n", ftnno, nregs);

	if((fregs == 4) || (model >= MDL_LARGE))
		svalue = 0;
	if(fregs == 5)
		svalue = 040;
	if(fregs == 6)
		svalue = 060;
	printf("\t.set\t.S%d,%d\n", ftnno, svalue);
	spoff = maxoff;
	spoff /= SZCHAR;
	SETOFF(spoff, ALSTACK/SZCHAR);
	printf( "\t.set\t.F%d,%ld\n", ftnno, spoff);
}

struct	hoptab
	{
	int opmask;
	char * opstring;
	} ioptab[] = {

	/*
	 * these entries produce mnemonics for ASG OPSIMP entries
	 * in the table using the O macro
	 */

	ASG PLUS, "add",
	ASG MINUS, "sub",
	ASG OR, "or",
	ASG AND, "and",
	ASG ER, "xor",
	ASG MUL, "mul",
	ASG DIV, "div",
	ASG MOD, "div",
	-1, ""    };

	/*
	 * output the appropriate string from the above table
	 * o is the op, f is the second letter of the O macro
	 * f is I (integer), L (long), or F (float).
	 */

hopcode(f, o)

int f, o;
{
	register struct hoptab *q;

	for(q = ioptab; q->opmask >= 0; ++q) {
		if(q->opmask == o) {
			printf("%s", q->opstring);
			return;
		}
	}
	cerror("no hoptab for %s", opst[o]);
}

#ifdef ONEPASS
extern int Sflag;
extern int tvflag;	/* 0 => direct, 1 => transfer vectors */
extern char *rnames[];

#else

int Sflag = 0;
int tvflag = 0;
char *rnames[] =
	{  /* keyed to register number tokens */

	"%ax", "%dx",
	"%si", "%bx",
	"%cx", "%di",
	"%sp", "%bp",
	"%ss", "%ds",
	"%es", "%cs",
	"%st", "%st(1)"
	};
#endif

int rstatus[] = {	/* use as scratch if not reg var */

	STAREG|SAREG,	STAREG|SAREG,			/* ax, dx */
	STBREG|SBREG,	STAREG|STBREG|SAREG|SBREG,	/* si, bx */
	STAREG|SAREG,	STBREG|SBREG,			/* cx, di */
	SBREG,		SBREG,				/* sp, bp */
	SBREG,		SBREG,				/* ss, ds */
	SBREG,		SBREG,				/* es, cs */
	SAREG,		SAREG,				/* st0, st1 */
	};

int fdepth;	/* current depth of 287 register stack */

NODE *brnode;
int brcase;

zzzcode(p, c, cook)

NODE *p;
register int c;
int cook;
{	/* these are the expansions for the user defined Z macros
	 * appearing in the table.  p is the node that has matched
	 * c is the second letter of the Z macro
	 */

	register m;
	NODE *n;
	char *s;
	int shiftend;
	CONSZ save;
	static char shiftop[] = "shl";

	switch(c) {

	case 'H':
	case 'L':
		/*
		 * display byte form of register argument
		 * normal address output for anything else
		 * should be A registers only
		 */
		if(p->in.op == REG)
			printf("%.2s%c", rnames[p->tn.rval], c + 'a'-'A');
		else
			adrput(p);
		return;

	case 'O':	/* output register as OREG */
		p->in.op = OREG;
		if(model >= MDL_LARGE)
			p->in.segment = SS;
		adrput(p);
		p->in.op = REG;
		p->in.segment = AX;
		return;

	case 'N':
		/* logical ops, resulting in a 0-1 value
		 * use register given by resource 1
		 * code generated looks like:
		 *	mov	$1,resc1
		 *	jmp	.n+1
		 * .n:	mov	$0,resc1
		 * .n+1:......
		 */

		cbgen(0, m = getlab(), 'I');
		deflab(p->bn.label);
		expand(p, FOREFF, "\txor\tA1,A1\n");
		if( fundtype(p->in.type) == LONG )
			expand(p, FOREFF, "\txor\tU1,U1\n");
		deflab(m);
		return;

	case 'P':
		/* generate branches for longs and floats */
		    /* reverse comparison if operands were PUSHed
		       onto the stack in wrong order */
		if( p->in.left->tn.rval >= ST0 &&
		    p->in.right->tn.rval >= ST0 &&
			/* stcount shows left on before right */
		    p->in.left->tn.lval < p->in.right->tn.lval ) {
			extern int negrel[];

			cbgen( negrel[ p->in.op - EQ ], p->bn.label, c);
		} else {
			cbgen(p->in.op, p->bn.label, c);
		}
		return;

	case 'A':
	case 'C':
		/* logical operators for longs and floats, remember the
		 * node and defer comparisons until branch occurs
		 * C means there is a comparison (two operands)
		 * A means there is a test of a single operand
		 */

		brnode = tcopy(p);
		brcase = c;
		return;

	case 'T':	/* weird sconv ops */
		resc[1] = resc[0];

		if( fundtype(p->in.type) == FLOAT ) {
			if(p->in.left->in.type == ULONG) {
				resc[1].tn.lval += 4;
				s = "\tmov\tAL,A1\n\tmov\tUL,U1\n\tmov\t$0,A2\n\tmov\t$0,U2\n\tfildll\tA1";
			}
			else {
				resc[1].tn.lval += 2;
				s ="\tmov\tAL,A1\n\tmov\t$0,A2\n\tfildl\tA1";
			}

			expand(p, FOREFF, s);
			return;
		}

		if(p->in.op == SCONV) {
			resc[1].tn.lval += 2;
			if( ISPTR(p->in.type) ||
			    (p->in.type == UNSIGNED) || (p->in.type == LONG) )
				resc[1].tn.lval += 2;
			else if(p->in.type == ULONG)
				resc[1].tn.lval += 6;
			resc[0].in.type = p->in.type;
			p = resc;
		}
		else if(p->in.op == ASSIGN)
			p = p->in.left;

		expand(p, FOREFF,
			"\tfstcw\tAL\n\tfstcw\tA2\n\tor\t$0x0c00,AL\n\tfldcw\tAL\n\tfistpZK\tAL\n\tfldcw\tA2\n\tfwait" );

		return;

	case 'V':	/* setup lower part of int to long conversion */
		if( (p->tn.op != REG) ||
		    (p->tn.rval != resc[0].tn.rval) )
			expand(p, FOREFF, "\tmov\tAL,A1\n");
		return;

	case 'B':	/* push a float onto the register stack */
		if((fdepth < 0) || (fdepth++ >= 7)) {
			uerror("floating stack overflow - simplify expression");
			fdepth = 0;
		}

		reclaim(p, RNULL, 0);
		p->tn.op = REG;
		p->tn.lval = ++stcount;
		p->tn.type = DOUBLE;
		busy[ST0] = 1;
		p->tn.rval = ST0;
		return;

	case 'D':	/* pop a float off of the register stack */
		busy[ST0] = 0;

	case 'E':
		fdepth--;
		p->tn.rval++;
		rbusy(ST1, DOUBLE);
		return;

	case 'I':	/* mul, div, mod ops */
		resc[0] = *p->in.left;
		if((p->in.op == ASG MOD) || (p->in.op == MOD))
			resc[0].tn.rval++;
		if( UNSIGNABLE(p->in.type) )
			PUTCHAR('i');
		return;

	case 'J':	/* call instruction prefix */
		if(model >= MDL_MIDDLE)
			PUTCHAR('l');
		return;

	case 'K': /* generate type postfix for operand of float instruction */
		switch(p->in.type)
			{
			case ULONG:
				PUTCHAR('l');

			case UNSIGNED:
			case LONG:
			case DOUBLE:
				PUTCHAR('l');
				return;

			case FLOAT:
				PUTCHAR('s');
				return;

			default:
				if( ISPTR(p->in.type) )
					PUTCHAR('l');
				return;
			}

	case 'F':	/* field shifts */
		m = (1 << fldsz) - 1;
		if(p->in.op == ICON) {
			printf("$%d", (p->tn.lval & m) << fldshf);
			return;
		}

		printf("\tand\t$%d,", m);
		adrput(p);
		PUTCHAR('\n');
		if(cook & INTAREG) {
			expand(p, FOREFF, "\tmov\tAL,A1\n");
			p = resc;
		}

		if(fldshf == 0)
			return;

		printf("\tsal\t");
		if(fldshf > 1)
			printf("$%d,", fldshf);
		adrput(p);
		PUTCHAR('\n');
		return;

	case 'G':	/* constant and variable shifts */
		m = p->in.right->tn.lval;
		if(m < 0) {
			p->in.op = (p->in.op == ASG LS)
					? ASG RS : ASG LS;
			m = -m;
		}

		shiftop[1] = ISUNSIGNED(p->in.type) ? 'h' : 'a';
		shiftop[2] = (p->in.op == ASG LS) ? 'l' : 'r';

		if(p->in.right->in.op == REG) {
			if( fundtype(p->in.type) == INT) {
				printf("\t%s\t", shiftop);
				expand(p, FOREFF, "ZRL,AL\n");
				return;
			}

			shiftend = getlab();
			printf("\tjcxz\t.%d\n", shiftend);
			m = getlab();
			deflab(m);
			printf("\t%s\t", shiftop);
			expand(p, FOREFF, (p->in.op == ASG LS) ?
				"AL\n\trcl\tUL" : "UL\n\trcr\tAL" );
			printf("\n\tloop\t.%d\n", m);
			deflab(shiftend);
			return;
		}

		if( fundtype(p->in.type) == INT) {
			m %= SZINT;
			if(m == 0)
				return;
			printf("\t%s\t", shiftop);
			if(m != 1) {
				adrput(p->in.right);
				PUTCHAR(',');
			}
			adrput(p->in.left);
			PUTCHAR('\n');
			return;
		}

		m %= SZLONG;
		if(m == 0)
			return;
		if(m < LTHRESH) {
			while(--m >= 0) {
				printf("\t%s\t", shiftop);
				expand(p, FOREFF, (p->in.op == ASG LS) ?
					"AL\n\trcl\tUL" : "UL\n\trcr\tAL");
				PUTCHAR('\n');
			}
			return;
		}

		if(m >= SZINT) {
			m -= SZINT;
			if(p->in.op == ASG LS) {
				expand(p, FOREFF,
					"\tmov\tAL,UL\n\txor\tAL,AL");
				if(m != 0) {
					printf("\n\tshl\t");
					if(m > 1)
						printf("$%d,", m);
					upput(p->in.left);
				}
			} else if ( p->in.type == LONG) {
				expand(p, FOREFF,
					"\tmov\tUL,AL\n\tsar\t$15,UL");
				if(m != 0) {
					printf("\n\tsar\t$%d,", m);
					adrput(p->in.left);
				}
			} else {
				expand(p, FOREFF,
					"\tmov\tUL,AL\n\txor\tUL,UL");
				if(m != 0) {
					printf("\n\tshr\t");
					if(m > 1)
						printf("$%d,", m);
					adrput(p->in.left);
				}
			}
			PUTCHAR('\n');
			return;
		}

		resc[0].tn.op = REG;
		resc[0].tn.type = INT;
		resc[0].tn.rall = NOPREF;
		if( ( (resc[0].tn.rval = freereg(resc, NAREG)) < 0 ) &&
		    ( (resc[0].tn.rval = freereg(resc, NBREG)) < 0 ) )
			cerror("no temp reg for shift - get help");
		busy[resc[0].tn.rval] = 0;

		if(p->in.op == ASG LS) {
			expand(p, FOREFF, "\tmov\tAL,A1\n\tshl\tAR,AL\n\tshl\tAR,UL\n");
			p->in.right->tn.lval = SZINT - m;
			expand(p, FOREFF, "\tshr\tAR,A1\n\tor\tA1,UL\n");
		}
		else {
			expand(p, FOREFF, " \tmov\tUL,A1\n\tshr\tAR,AL\n\t");
			printf(shiftop);
			expand(p, FOREFF, "\tAR,UL\n");
			p->in.right->tn.lval = SZINT - m;
			expand(p, FOREFF, "\tshl\tAR,A1\n\tor\tA1,AL\n");
		}
		return;

	case 'S':  /* structure assignment */
		m = p->stn.stsize;
		n = talloc();
		n->in.op = STASG;
		if(p->in.op == STARG) {
			n->in.right = p->in.left;
			n->in.left = resc + 1;
			if(model >= MDL_LARGE) {
				printf("\tmov\t%%ss,%%di\n\tmov\t%%di,%%es\n");
				n->in.left->in.segment = ES;
			}
			printf("\tsub\t$%d,%%sp\n\tmov\t%%sp,%%di\n", m);
		}
		else {
			n->in.left = p->in.left;
			n->in.right = p->in.right;
		}
		if(m > 3) {
			printf("\tmov\t$%d,", m >> 1);
			adrput(resc);
			printf("\n\trep\n");
		}
		n->in.left->in.op = OREG;
		n->in.right->in.op = OREG;
		expand(n, FOREFF, "\tsmov\tAR,AL");
		n->in.left->in.op = REG;
		n->in.right->in.op = REG;
		if(cook & INTBREG)
			printf("\n\tsub\t$%d,%%si", m);
		n->in.op = FREE;
		return;

# ifdef SSPLIT
	case 'Q':	/* expand and free lastseg element */
		switch(p->in.op)
			{
			case REG:
			case OREG:
				if( (segenv->lastseg.in.op != FREE) &&
				    istreg(p->tn.rval) ) {
					if(segenv->lastseg.in.op != ICON)
						adrput(&segenv->lastseg);
					else
						upput(&segenv->lastseg);
					if( !(cook & (INTAREG|INTBREG)) ) {
						segfree(segenv);
						segenv->lastseg.tn.op = FREE;
					}
					return;
				}
				break;

			case ICON:
				p->in.segment = BX;
				upput(p);
				p->in.segment = AX;
				return;

			case INIT:
				p = p->in.left;
				save = p->tn.lval;
				if(p->in.name[0] != 0) {
					printf("<s>");
					if(model == MDL_LARGE)
						p->tn.lval = 0;
				}
				else
					p->tn.lval >>= SZINT;
				conput(p);
				p->tn.lval = save;
				return;
			}
		upput(p);
		return;

	case 'X':	/* large pointer entry macro */
		switch(p->in.op)
			{
			case SCONV:
				p = p->in.left;
				segset(segenv, p->in.op, p);
				return;

			case OREG:
			case NAME:
				m = segenv->prefseg;
				if( (m == CX) &&
				    ( (p->tn.op != OREG) ||
				      (p->tn.rval != BP) ) ) {
					busy[resc[0].tn.rval]++;
					segenv->lastseg.tn.rall |= p->tn.rall & NEEDREGS;
					if( (m = freereg(&segenv->lastseg, NAREG)) < 0)
						cerror("zzzcode - no free reg for segment - get help");
					busy[resc[0].tn.rval]--;
					busy[m] = 0;
					resc[1].tn.op = REG;
					resc[1].tn.lval = 0;
					resc[1].tn.rval = m;
					expand(p, FOREFF, "\tmov\tZQ,A2\n");
					segset(segenv, FREE, &resc[1]);
					printf("\tmov\t");
				}

				else if( (m == ES) || (m == DS) ) {
					printf("\tl%cs\t",
					    (m == DS) ? 'd' : 'e');
					m = p->tn.rval;
					p->tn.rval = segenv->prefseg - 1;
					segset(segenv, REG, p);
					p->tn.rval = m;
				}

				else {
					segset(segenv, p->in.op, p);
					printf("\tmov\t");
				}

				expand(p, FOREFF, "AL,A1");
				return;

			case ICON:
				segset(segenv, ICON, p);
				expand(p, FOREFF,
					(p->tn.lval || p->tn.name[0]) ?
						"\tmov\tAL,A1" :
						"\txor\tA1,A1" );
				return;

			case REG:
				if(p->tn.rval == BP) {
					m = p->tn.rval;
					p->tn.rval = SS;
					segset(segenv, FREE, p);
					p->tn.rval = m;
					return;
				}
				return;

			case ASG PLUS:
				p = p->in.right;
				segset(segenv, p->in.op, p);
				return;

			case FORCE:
				segfree(segenv);
				segbind(segenv, DS|MUSTDO);
				return;

			case UNARY AND:
				p = p->in.left;
				if((p->tn.op == OREG) && (p->tn.rval == BP)) {
					p->tn.rval = SS - 1;
					segset(segenv, REG, p);
					p->tn.rval = BP;
				}
				return;

			default:
				cerror( "illegal node type for X macro" );
			}
		/* NOTREACHED */

	case 'Y':	/* large pointer exit macro */
		switch(p->in.op)
			{
			case ASSIGN:
				p = p->in.right;

			case REG:
				if( (segenv->lastseg.in.op != REG) &&
				    (segenv->lastseg.in.op != ICON) &&
				    (segenv->lastseg.in.op != FREE) ) {
					if(cook & FOREFF)
						resc[0] = *p;
					expand(p, FOREFF, "\tmov\tZQ,A1\n");
					segset(segenv, FREE, &resc[0]);
				}
				return;

			case SCONV:
				if(p->in.left->in.op != REG) {
					p = p->in.left;
					if(p->in.op == ICON)
						p->in.segment = BX;
					return;
				}
				if( fundtype(p->in.type) == LONG ) {
					if(p->in.left->tn.rval != resc[0].tn.rval)
						expand(p, FOREFF, "\tmov\tAL,A1\n");
					if( (segenv->lastseg.in.op != REG) ||
					    (segenv->lastseg.tn.rval != resc[0].tn.rval + 1) )
						expand(p->in.left, FOREFF, "\tmov\tZQ,U1\n");
				}
				segfree(segenv);
				segenv->lastseg.in.op = FREE;
				return;

			default:
				cerror("illegal node type for Y macro");
			}
# endif

	default:
		cerror("illegal zzzcode");
	}
}

rmove(rt, rs, t)

int rt, rs;
TWORD t;
{	/* called by reclaim
	 * last chance attempt to get thing into correct register */

	printf("\tmov\t%s,%s\n", rnames[rs], rnames[rt]);
	if(szty(t) == 2) 
		printf("\tmov\t%s,%s\n", rnames[rs+1], rnames[rt+1]);
}

struct respref
respref[] = {	/* cookie to use for reclaim given cookie from order call */

	INTAREG,		INTAREG,
	INTBREG,		INTBREG,
	INTEMP,			INTEMP,
	FORARG,			FORARG,
	INTAREG|INTBREG,	INTAREG|INTBREG,
	INTAREG|INTBREG|INTEMP,	SCON,
	INTAREG|INTBREG|INTEMP,	SNAME,
	INTAREG|INTBREG|INTEMP,	SOREG,
	INTEMP,			INTAREG|INTBREG,
	0,	0 };

setregs()

{ /* set up temporary registers */
	register int i;

	/* use any unused variable registers as scratch registers
	 * fregs is the highest numbered scratch register available
	 */
	fregs = 4;

	/* set up the status of the register variables for the current block */
	for( i=MINRVAR; i<=MAXRVAR; i++ )
		if ( i == CX )
			rstatus[i] = i<fregs ? SAREG|STAREG : SAREG;
		else
			rstatus[i] = i < fregs ? SBREG|STBREG : SBREG;

	/* delayed initialization for special purpose registers */
	rstatus[ES] = rstatus[DS] = STBREG|SBREG;
	rstatus[ST0] = rstatus[ST1] = STAREG|SAREG;
}

szty(t)

TWORD t;
{
	/*
	 * size in words needed to hold a thing of type t
	 * really is the number of registers to hold type t
	 */

	if( (model == MDL_MIDDLE) && ISPTR(t) && ISFTN(t >> TSHIFT) )
		return(2);

	switch(t)
		{
		case UNDEF:
			return(0);

		case FLOAT:
		case DOUBLE:
			return(1);			/* 1 register */

		case LONG:
		case ULONG:
			return(SZLONG / SZINT);		/* 2 words */

		default:
			return(1);			/* 1 word */
		}
}

rewfld(p)

NODE *p;
{	 /* is it ok to rewrite a field node ? 1 = YES, 0 = NO */
	return(1);
}

callreg(p)

NODE *p;
{	/* return the register to be used for function return */
	switch(p->in.type)
		{
		case FLOAT:
		case DOUBLE:
			return(ST0);

		default:
			return(AX);
		}
}

/*ARGSUSED*/
shltype(o, p)

int o;
NODE *p;
{	/* does p->in.op = o have the shape of a leaf type ?
	 * REG's are leaf types, but we want to use this function
	 * to determine when something is a leaf but not
	 * in a register
	 */

	return((o == NAME) || (o == ICON) || (o == OREG));
}

flshape(p)

register NODE *p;
{	/* does p have the shape of a field type ? */
	return((p->in.op == NAME) || (p->in.op == OREG));
}

shtemp(p)

register NODE *p;
{	/* does p have the shape of a temporary ? */
	if( p->in.op == STARG )
		p = p->in.left;
	return( (p->in.op == OREG) && (-p->tn.lval > BITOOR(baseoff)) );
}

shumul(p)

NODE *p;
{	/* look for situations where indirection may be done */

	if((p->in.op == PLUS) || (p->in.op == MINUS)) {
		if( (p->in.left->in.op == REG) &&
		    (p->in.right->in.op == ICON) &&
		    ( (p->in.op == PLUS) ||
		      (p->in.right->in.name[0] == 0) ) )
			p = p->in.left;
	}

	if( (p->tn.op == REG) &&
	    (p->tn.rval == BP) )
		return(STARREG);

	return(0);
}

adrcon(val)

CONSZ val;
{	/* print an address constant */

	printf("$");
	printf(CONFMT, val);
}

conput(p)

register NODE *p;
{	/* print a constant operand */

	switch(p->in.op)
		{
		case ICON:
			acon(p, (p->tn.type == LONG) ?
					p->tn.lval : (short) p->tn.lval );
			return;

		case REG:
			printf("%s", rnames[p->tn.rval]);
			return;

		default:
			cerror("illegal conput");
		}
}

/* ARGSUSED */
insput(p)

NODE *p;
{
	cerror("insput");
}

upput(p)

register NODE *p;
{	/* output the address of the second word in the
	 * pair pointed to by p (for longs or floats)
	 * on the 8086, the upper half has the higher address
	 * or the higher register number
	 */

	register CONSZ save;

	if(p->in.op == FLD)
		p = p->in.left;

	switch(p->in.op)
		{
		case OREG:
		case NAME:
			save = p->tn.lval;
			p->tn.lval = save + 2;
			adrput(p);
			p->tn.lval = save;
			return;

		case ICON:
			/* addressable value of the constant
			 * print the high order value
			 */

			save = p->tn.lval;
			PUTCHAR('$');
			if((p->in.segment == BX) && (p->in.name[0] != 0)) {
				if(model == MDL_LARGE)
					save = 0;
				printf("<s>");
			}
			else
				save = (save >> SZINT);
			acon(p, save);
			return;

		case REG:
			printf("%s", rnames[p->tn.rval + 1]);
			return;

		default:
			cerror("illegal upper address");
			return;
		}
}

adrput(p)

register NODE *p;
{	/* output an address, with offsets, from p
	 * this does the lower half for longs and floats
	 *  and all addresses for other types
	 */
	register CONSZ v;

	if(p->in.op == FLD)
		p = p->in.left;

	v = (short) p->tn.lval;
	switch(p->in.op)
		{
		case OREG:
			segput(p->in.segment);
			if(p->tn.rval == BP)	/* in the argument region */
				if(p->in.name[0] != '\0')
					werror("bad arg temp");
			if( (p->tn.lval != 0) || (p->in.name[0] != '\0') ||
			    (p->tn.rval == BP) )
				acon(p, v);
			if((p->in.name[0] & 0200) == 0)
				if( R2TEST(p->tn.rval) )
					printf("(%s,%s)",
						rnames[R2UPK2(p->tn.rval)],
						rnames[R2UPK1(p->tn.rval)]);
				else
					printf("(%s)", rnames[p->tn.rval]);
			return;

		case NAME:
			segput(p->in.segment);
			acon(p, v);
			return;

		case ICON:
			PUTCHAR('$');
			acon(p, v);
			return;

		case REG:
			printf("%s", rnames[p->tn.rval]);
			return;

		default:
			cerror("illegal address");
			return;
		}
}

acon(p, v)

register NODE *p;
register CONSZ v;
{	/* print out a constant address */

	if(p->in.name[0] == '\0')	/* constant only */
		printf(CONFMT, v);
	else if(v == 0)	/* name only */
# ifndef FLEXNAMES
		printf("%.8s", p->in.name);
# else
		printf("%s", p->in.name);
# endif
	else {				/* name + offset */
# ifndef FLEXNAMES
		printf("%.8s+", p->in.name);
# else
		printf("%s+", p->in.name);
# endif
		printf(CONFMT, v);
	}
}

genscall(p, cookie)

register NODE *p;
{	/* structure valued call -- same as regular call */
	return( gencall(p, cookie) );
}

gencall(p, cookie)

register NODE *p;
int cookie;
{	/* generate the call given by p */
	register NODE *n;
	int off;
	register int m;
	int save;

# ifdef SSPLIT
	struct segenv res;

	if(model >= MDL_LARGE) {
		segsetenv(&res, AX);
		segpushenv(&res);
	}
# endif

	off = 0;
	if(p->in.op == HARDOP) {
		n = p->in.right;
		doact(n, INTAREG, INTAREG);
		reclaim(n, RNULL, 0);
	}
	else if(p->in.right != NULL) {
		off = argsize(p->in.right);
		genargs(p->in.right);
	}

	/* compute the function address if necessary */
	n = p->in.left;
	if( couldaddr(n) ) {
		doact1(n, INBREG);
		canon(n);
	}
	else if( !canaddr(n) ) {
		if(model == MDL_MIDDLE)
			n->in.type = LONG;
		doact1(n, INTEMP);
	}

	/* RE-ENTRANT STRUCTURE RETURN LINKAGE */
	/* load linkage registers for structure return, if any */
	if (Sflag && p->in.op == UNARY STCALL) {
		printf("\tlea\t%d(%%bp),%%ax\n",
			freetemp((SZCHAR*p->stn.stsize+(SZINT-1))/SZINT)/SZCHAR);
		if(model >= MDL_LARGE) printf("\tmov\t%%ss,%%dx\n");
	}

	/* do the code generation for the call */
	p->in.op = UNARY CALL;
	m = match(p, INTAREG);

# ifdef SSPLIT
	if(model >= MDL_LARGE) {
		segpopenv(&res);
		if( (cookie != FOREFF) && ISPTR(p->in.type) ) {
			save = p->tn.rval;
			p->tn.rval = DX;
			segset(segenv, FREE, p);
			p->tn.rval = save;
		}
	}
# endif

	/* clean up the stack after the return */
	if(off > 0)
		printf("\tadd\t$%d,%%sp\n", off);

	return(m != MDONE);
}

char *
ccbranches[] = {
	/* code for conditional branches
	 * the ordering here must match the manifest values
	 */

	"\tje\t.%d\n",
	"\tjne\t.%d\n",
	"\tjle\t.%d\n",
	"\tjl\t.%d\n",
	"\tjge\t.%d\n",
	"\tjg\t.%d\n",
	"\tjbe\t.%d\n",
	"\tjb\t.%d\n",
	"\tjae\t.%d\n",
	"\tja\t.%d\n",
	};

/*	long branch table
 *
 * This table, when indexed by a logical operator,
 * selects a set of three logical conditions required
 * to generate long comparisons and branches.  A zero
 * entry indicates that no branch is required.
 * E.G.:  The <= operator would generate:
 *	cmp	AR,AL
 *	jl	label	/ 1st entry LT -> label
 *	jg	local	/ 2nd entry GT -> local
 *	cmp	UR,UL
 *	jlos	label	/ 3rd entry ULE -> label
 * local: ....
 */

int lbranches[][3] = {
	/*EQ*/	0,	NE,	EQ,
	/*NE*/	NE,	0,	NE,
	/*LE*/	LT,	GT,	ULE,
	/*LT*/	LT,	GT,	ULT,
	/*GE*/	GT,	LT,	UGE,
	/*GT*/	GT,	LT,	UGT,
	/*ULE*/	ULT,	UGT,	ULE,
	/*ULT*/	ULT,	UGT,	ULT,
	/*UGE*/	UGT,	ULT,	UGE,
	/*UGT*/	UGT,	ULT,	UGT,
	};

/*ARGSUSED*/
cbgen(o, lab, mode)

int o, lab, mode;
{	/* print conditional and unconditions branches */
	register *plb;
	int lab1f;

	if(o == 0)
		printf("\tjmp\t.%d\n", lab);
	else if(o > UGT)
		cerror("bad conditional branch: %s", opst[o]);
	else {	/* long comparison has been delayed to here
		 * A means there is just one operand (compare to zero)
		 * C is a comparison of two operands
		 */

		switch(brcase)
			{
			case 'A':
			case 'C':
				/* get the lbranch[] entry */
				plb = lbranches[o - EQ];
				lab1f = getlab();

				/* now do the five line test-jump sequence */
				expand(brnode, FORCC, brcase=='C' ?
					"\tcmp\tUR,UL\n" : "\ttest\tUR\n");
				if(*plb != 0)
					printf(ccbranches[*plb - EQ], lab);
				if(*++plb != 0)
					printf(ccbranches[*plb - EQ], lab1f);
				expand(brnode, FORCC, (brcase == 'C') ?
					"\tcmp\tAR,AL\n" : "\ttest\tAR\n");
				printf(ccbranches[*++plb - EQ], lab);
				deflab(lab1f);

				/* and clean up */
				reclaim(brnode, RNULL, 0);
				break;

			default:
				/* tests that were not long are already done, just branch */
				printf(ccbranches[o - EQ], lab);
				break;
			}

		brcase = 0;
		brnode = 0;
	}
}

nextcook(p, cookie)

NODE *p;
int cookie;
{
	/* try to do match with a different cookie */

	if( cookie == FORREW )
		return( 0 );  /* hopeless! */

	if( (cookie & (INTBREG|INBREG)) == cookie )
		return( INTAREG|INTBREG );

	if( !(cookie&(INTAREG|INTBREG)) )
		return( INTAREG|INTBREG );

	else if ( !(cookie & INTEMP) )
		return( cookie|INTEMP );

	return( FORREW );
}

/* ARGSUSED */
lastchance(p, cook)

NODE *p;
{
	return(0);
}

optim2(p, lastop)

register NODE *p;
int lastop;
{
	int l, r;
	TWORD t;

	p->in.segment = AX;

	/* prewalk transformations */
	l = r = p->in.op;
	switch(p->in.op)
		{
		case INIT:
			if(p->in.left->in.op == SCONV)
				consconv(p->in.left);
			break;

		case SCONV:
			sconvop(p);
			break;

		case ASG PLUS:
		case ASG MINUS:
			if( fundtype(p->in.type) == FLOAT )
				unasgof(p);
			break;

		case ASG LS:
		case ASG RS:
			if( fundtype(p->in.type) == INT )
				break;
		case ASG MUL:
		case ASG DIV:
		case ASG MOD:
			unasgof(p);
			break;

		case UNARY MUL:
			if (model >= MDL_HUGE)
				break;
			umulop2(p->in.left, 1, 1, NIL);
			break;

		case PLUS:
			comop(p);
		case MINUS:
			if( fundtype(p->in.type) == FLOAT ) {
				floatop(p);
				break;
			}

			if( (lastop != UNARY MUL) && shumul(p) ) {
				t = p->in.type;
				unandof(p);
				andof(p);
				p->in.type = t;
				l = r = p->in.op;
			}
			break;

		case AND:
		case OR:
		case ER:
			comop(p);
			break;

		case MUL:
			comop(p);
			switch( fundtype(p->in.type) )
				{
				case LONG:
					hardop(p);
					break;

				case FLOAT:
					floatop(p);
					break;
				}
			break;

		case DIV:
		case MOD:
			switch( fundtype(p->in.type) )
				{
				default:
		/*If dividing a 32 bit value by a 16 bit value, the quotient
		 *may not fit in 16 bits.  If this quotient is to be stored
		 *into a 16 bit word, then the result will be incorrect.  The
		 *problem here is that on the 80286 if the idiv instruction
		 *is used, a trap is generated if the quotient won't fit in
		 *16 bits.  The change made by INTERACTIVE forces the compiler
		 *to generate a call to the long divide routine rather than
		 *allowing it to generate an idiv instruction.  Thus, the
		 *quotient may be incorrect, but no trap will be generated.
		 */
				 if(fundtype(p->in.left->in.type) != LONG
				  && fundtype(p->in.right->in.type) != LONG)
				  {
				   divop(p);
				   break;
				  }
				case LONG:
					hardop(p);
					break;

				case FLOAT:
					floatop(p);
					break;
				}
			break;

		case LE:
		case LT:
		case GE:
		case GT:
			if( fundtype(p->in.left->in.type) == FLOAT )
				p->in.op += ULE - LE;
		case EQ:
		case NE:
			comop(p);
			if(p->in.left->in.type == DOUBLE)
				floatop(p);
			break;

		case ULE:
		case ULT:
		case UGE:
		case UGT:
			comop(p);
			break;

		case UNARY CALL:
		case CALL:
		case INCR:
		case DECR:
		case QUEST:
			if(p->in.type == FLOAT)
				p->in.type = DOUBLE;
			break;

		case STASG:
			andof(p->in.left);
			break;
		}

	switch( optype(p->in.op) )
		{
		case BITYPE:
			optim2(p->in.right, r);

		case UTYPE:
			optim2(p->in.left, l);
		}

	/* postwalk transformations */
	switch(p->in.op)
		{
		case ASSIGN:
			floatop(p);
			if( !lastop && (p->in.op == UNARY MUL) )
				andof(p);
			break;

		case CALL:
		case STCALL:
			argop(p->in.right);
			break;
		}
}

andof(p)

register NODE *p;
{
	register NODE *n;

	if(p->in.op == NAME) {
		p->in.op = ICON;
		p->in.type = INCREF(p->in.type);
		return;
	}

	if(p->in.op == UNARY MUL) {
		n = p->in.left;
		if( !shumul(n) ) {
			*p = *n;
			n->in.op = FREE;
			return;
		}
	}

	n = talloc();
	*n = *p;
	p->in.op = UNARY AND;
	p->in.type = INCREF(p->in.type);
	p->in.left = n;
	p->tn.rval = 0;
}

unandof(p)

register NODE *p;
{
	register NODE *n;

	n = talloc();
	*n = *p;
	p->in.op = UNARY MUL;
	if( ISPTR(p->in.type) )
		p->in.type = DECREF(p->in.type);
	p->in.left = n;
	p->tn.rval = 0;
}

unasgof(p)

register NODE *p;
{
	register NODE *n;
	int recl2();

	n = talloc();
	*n = *p;
	n->in.op -= ASG 0;
	p->in.op = ASSIGN;
	p->in.right = n;
	p->in.left = tcopy(p->in.left);
	walkf(p->in.left, recl2);

	/* transmute char asgops */
	if( fundtype(p->in.type) == CHAR ) {
		sconvof(n, n->in.type);

		p = n->in.left;
		p->in.type = (p->in.type == CHAR) ? INT : UNSIGNED;
		sconvof(p->in.left, p->in.type);

		if( (p->in.op != LS) &&
		    (p->in.op != RS) )
			sconvof(p->in.right, p->in.type);
	}
}

sconvof(p, t)

register NODE *p;
TWORD t;
{
	register NODE *n;

	if(p->in.op == ICON) {
		p->in.type = t;
		return;
	}

	n = talloc();
	*n = *p;
	p->in.op = SCONV;
	p->in.left = n;
	p->in.type = t;
}

consconv(p)

register NODE *p;
{
	register NODE *n;
	NODE *q;
	TWORD t;

	q = n = p->in.left;
	while(n->in.op == SCONV)
		n = n->in.left;

	if(n->in.op != ICON)
		return;

	t = p->in.type;
	*p = *n;
	p->in.type = t;

# ifdef SSPLIT
	if( (model >= MDL_LARGE) && ISPTR(n->in.type) &&
	    (p->in.name[0] != 0) )
		p->in.segment = BX;
# endif

	n->in.op = FREE;
	while(q != n) {
		q->in.op = FREE;
		q = q->in.left;
	}
}

	/*
	 * change hard to do operator into function calls.
	 * for right now do long mul, div, and mod
	 * and assignment versions of them
	 */

struct hoptab hoptab[] = {

	MUL,	"_lmul",
	DIV,	"_ldiv",
	MOD,	"_lmod",
	-1
	};

/* need different calls for unsigned long DIV and MOD */
struct hoptab uhoptab[] = {
	MUL,	"_lmul",
	DIV,	"_uldiv",
	MOD,	"_ulmod",
	-1
	};

hardop(p)

register NODE *p;
{
	register NODE *q;
	struct hoptab *h;

	/* need different calls for unsigned long DIV and MOD */
	if (ISUNSIGNED(p->in.type))
		h = uhoptab;
	else
		h = hoptab;
	for(; h->opmask >= 0; ++h)
		if(h->opmask == p->in.op)
			break;

	if(h->opmask < 0)
		cerror("no hardop for %s", opst[p->in.op]);

	/* build comma op for args to function */
	q = talloc();
	q->in.op = CM;
	q->in.type = (h->opmask == MUL) ? ULONG : LONG;
	q->in.left = p->in.left;
	q->in.right = p->in.right;
	p->in.op = HARDOP;
	p->in.right = q;

	/* put function name in left node of call */
	p->in.left = q = talloc();
	q->in.op = ICON;
	q->in.type = INCREF( FTN + p->in.type );
# ifndef FLEXNAMES
	strcpy( q->in.name, h->opstring );
# else
	q->in.name = h->opstring;
# endif
	q->tn.lval = 0;
	q->tn.rval = 0;
}

comop(p)

register NODE *p;
{
	register NODE *l;
	register NODE *r;

	l = p->in.left;
	r = p->in.right;

	if(r->in.op == ICON)
		return;
	if(l->in.op == ICON)
		goto swap;

	if( (p->in.op == PLUS) &&
	    ISPTR(p->in.type) ) {
		if( ISPTR(l->in.type) )
			goto swap;
		return;
	}

	if( (p->in.op == MUL) &&
	    ( fundtype(p->in.type) == INT ) ) {
		if( needrchk(r) && !needrchk(l) )
			goto swap;
		return;
	}

	if( ( fundtype(p->in.type) == FLOAT) ||
	    ( logop(p->in.op) && (l->in.type == DOUBLE) ) ) {
		if( (l->in.op == SCONV) &&
		    ISFLCONV(l->in.left->in.type) )
			l = l->in.left;

		if( (r->in.op == SCONV) &&
		    ISFLCONV(r->in.left->in.type) )
			r = r->in.left;
	}

	if( canaddr(r) )
		return;
	if( canaddr(l) )
		goto swap;
	if( couldaddr(r) )
		return;
	if( couldaddr(l) )
		goto swap;
	return;

swap:
	if( logop(p->in.op) )
		p->in.op = revrel[p->in.op - EQ];
	l = p->in.left;
	p->in.left = p->in.right;
	p->in.right = l;
	return;
}

/*
 *	Look for things like like:
 *	UNARY MUL long
 *		PLUS PTR long
 *			SCONV int
 *				<< long
 *					NAME long
 *					ICON 2
 *			ICON addr
 * 	And change to something like:
 *	UNARY MUL long
 *		PLUS PTR long
 *			<< long
 *				SCONV int
 *					NAME long
 *				ICON 2
 *			ICON addr
 *	Not only is this unnecessary in SMALL through LARGE
 *	model, but many sub classes of trees get confused as well.
 */

umulop2(p, root, left, prevnode)
register NODE *p, *prevnode;
{
	register NODE *p1;

	switch (p->in.op) {
		default:
			return;
		case PLUS:
			if (!ISPTR(p->in.type))
				return;		/* double checking */
			if (optype(p->in.left->in.op) != LTYPE)
				umulop2(p->in.left, 0, 1, p);
			if (optype(p->in.right->in.op) != LTYPE)
				umulop2(p->in.right, 0, 0, p);
			return;
		case SCONV:
			if (root) {
				if (optype(p->in.left->in.op) != LTYPE)
					umulop2(p->in.left, 0, 1, p);
				return;
			}
			if (p->in.type == INT &&
			    p->in.left->in.type == LONG) {
				p1 = p->in.left;
				/* move the SCONV as far down p1 as possible */
				if (umopconv(p1, p, LONG, INT)) {
					/* Re-link up the tree */
					if (left)
						prevnode->in.left = p1;
					else
						prevnode->in.right = p1;
				}
			}
			return;
	}
}

/*
 * Follow the given tree converting nodes to int type as far
 * as possible, and then install an SCONV node.
 *	p - tree to follow, sc = the sconv node to move.
 *	from - looking for, to - change to.
 *	Return 1 if the SCONV was moved, 0 if not
 */

umopconv(p, sc, from, to)
register NODE *p, *sc;
{
	if (p->in.type != from) {
		return(0);
	}
	/* See if I can traverse further ... */
	if (optype(p->in.op) != BITYPE)
		return(0);
	switch (p->in.op) {
		default:
			return(0);
		case LS:
			p->in.type = to;
			if (umopconv(p->in.left, sc, from, to) == 0) {
				sc->in.left = p->in.left;
				p->in.left = sc;
			}
			return(1);
	}
}

divop(p)

register NODE *p;
{
	register NODE *n;

	n = p->in.left;
	if( (n->in.op == SCONV) &&
	    (fundtype(n->in.left->in.type) != CHAR) &&
	    !ISPTR(n->in.left->in.type) ) {
		if( fundtype(n->in.left->in.type) == LONG ) {
			n->in.op = FREE;
			p->in.left = n->in.left;
		}
	}
	else {
		n = talloc();
		n->in.op = SCONV;
		n->in.left = p->in.left;
		p->in.left = n;
	}

	n->in.type = UNSIGNABLE(p->in.type) ? LONG : ULONG;
}

sconvop(p)

register NODE *p;
{
	register NODE *n;
	int t, tl;
	NODE *t1,*t2;

	n = p->in.left;
	t = fundtype(p->in.type);
	tl = fundtype(n->in.type);

	/*If dividing a 32 bit value by a 16 bit value, the quotient
	 *may not fit in 16 bits.  If this quotient is to be stored
	 *into a 16 bit word, then the result will be incorrect.  The
	 *problem here is that on the 80286 if the idiv instruction
	 *is used, a trap is generated if the quotient won't fit in
	 *16 bits.  The change made by INTERACTIVE forces the compiler
	 *to generate a call to the long divide routine rather than
	 *allowing it to generate an idiv instruction.  Thus, the
	 *quotient may be incorrect, but no trap will be generated.
	 *
	 *There was a special case here that caused the idiv instruction
	 *to be generated.  That special case was deleted.
	 */

	if( (n->in.op == MUL) &&
	    (t == LONG) && (tl == INT) ) {
		comop(n);
		if(n->in.type == INT) {
			if(n->in.right->in.op == ICON)
				n->in.right->in.type = LONG;
			n->in.type = LONG;
		}
		else
			n->in.type = ULONG;
		*p = *n;
		n->in.op = FREE;
		return;
	}
	/*The following code inserts to SCONV nodes into a tree that wants to
	 *pass a 16 bit value as a pointer.  The problem is that code is
	 *generated to push dx onto the stack as the segment part of the
	 *pointer, but dx contains an unknown value.  The fix ensures that dx
	 * is cleared before it is pushed.
	 */
	if(model>=MDL_LARGE) /*Only for large model...    */
	 if (ISPTR(p->in.type) && szty(tl)<=1)
	  { /*Converting a small thing to a 32 bit pointer?  */
	   t1=talloc();  /* Make 2 copies of the SCONV node. */
	   *t1= *p;
	   t2=talloc();
	   *t2= *p;
	   t2->in.type=UNSIGNED; /*Convert the small thing to unsigned. */
	   t1->in.type=ULONG; /*Convert the unsigned to an unsigned long.*/
	   t2->in.left=n;
	   t1->in.left=t2;
	   p->in.left=t1; /*Convert the unsigned long to a pointer.*/
	  }
}

argop(p)

register NODE *p;
{
	register NODE *n;

	if(p->in.op == CM) {
		argop(p->in.left);
		p = p->in.right;
	}

	if(p->in.type == FLOAT) {
		sconvof(p, (TWORD)DOUBLE);
		return;
	}

	if( (p->in.type == DOUBLE) &&
	    ( canaddr(p) || couldaddr(p) ) ) {
		n = talloc();
		*n = *p;
		p->in.op = STARG;
		p->in.left = n;
		p->in.type = DOUBLE;
		p->stn.stsize = SZDOUBLE / SZCHAR;
		p->stn.stalign = ALDOUBLE / SZCHAR;
		andof(n);
	}
}

floatop(p)

register NODE *p;
{
	register NODE *n;

	if(p->in.op == ASSIGN) {
		n = p->in.right;
		if( (p->in.type == FLOAT) &&
		    (n->in.op == SCONV) ) {
			p->in.type = DOUBLE;
			if(n->in.left->in.type == DOUBLE) {
				n->in.op = FREE;
				p->in.right = n->in.left;
			}
			else
				n->in.type = DOUBLE;
			return;
		}

		if( ( (p->in.type == INT) ||
		      (p->in.type == LONG) ) &&
		    (n->in.op == SCONV) &&
		    ( fundtype(n->in.left->in.type) == FLOAT ) ) {
			n->in.op = FREE;
			p->in.right = n->in.left;
			return;
		}

		if( (p->in.type == DOUBLE) &&
		    ( canaddr(p->in.right) || couldaddr(p->in.right) ) ) {
			p->in.op = STASG;
			p->in.type += PTR;
			p->stn.stsize = SZDOUBLE / SZCHAR;
			p->stn.stalign = ALDOUBLE / SZCHAR;
			andof(p->in.left);
			andof(p->in.right);
			unandof(p);
			return;
		}

		return;
	}

	if(p->in.type == FLOAT) {
		p->in.type = DOUBLE;
		n = p->in.left;
		if(n->in.op == SCONV)
			n->in.type = DOUBLE;
		n = p->in.right;
		if(n->in.op == SCONV)
			n->in.type = DOUBLE;
	}

	n = p->in.left;
	if( (n->in.op == SCONV) &&
	    ( fundtype(n->in.left->in.type) == FLOAT) ) {
		n->in.op = FREE;
		p->in.left = n->in.left;
	}

	n = p->in.right;
	if( (n->in.op == SCONV) &&
	    ISFLCONV(n->in.left->in.type) &&
	    ( canaddr(n->in.left) || couldaddr(n->in.left) ) ) {
		n->in.op = FREE;
		p->in.right = n->in.left;
	}
}

myreader(p)

register NODE *p;
{
	optim2(p, 0);
# ifdef SSPLIT
	if(model >= MDL_LARGE) {
		splitops(p);
		segsetenv(segenv, AX);
	}
# endif
}

mycallchk()
{
	register int i;

	REGLOOP(i) {
		if( istreg(i) && busy[i] )
			cerror("register allocation error");
	}
}

special(p, shape)

NODE *p;
int shape;
{
	int shift;

# ifdef SSPLIT
	/* continue processing shape if model is large or huge */
	if(shape & (SLARGE & ~SPECIAL)) {
		if(model < MDL_LARGE)
			return(0);
		return( tshape(p, shape & ~(SPECIAL|SLARGE)) );
	}
# endif

	if( (p->in.op != ICON) ||
	    ( (p->in.name != 0) && (p->in.name[0] != 0) ) )
		return(0);

	/* match for long shifts that need temp reg */
	if(shape == SLSHFT) {
		shift = abs(p->tn.lval);
		if((LTHRESH <= shift) && (shift < SZINT))
			return(1);
	}
	return(0);
}

myo2reg(p)

register NODE *p;
{
	register NODE *q, *ql, *qr;
	CONSZ temp;
	int r;

	return;
}

mycanon(p)

register NODE *p;
{
	/* fix up tree after each reduction or modificiation */
	walkf(p, myo2reg);	/* look for special case double indexing */
}

#ifndef ONEPASS
main(argc, argv)

int argc;
char *argv[];
{
	register int i;
	int retcode;
	int nargc;
	char **nargv;
	/* Code to collect performance stats */
	long ttime;
	struct tbuffer {
		long proc_user_time;
		long proc_system_time;
		long child_user_time;
		long child_system_time;
	} ptimes;
	extern long times();

	ttime = times(&ptimes);
#ifndef NARGS
	nargc = 1;
	nargv = argv;

	for (i = 1; (i < argc); i++) {
		if(argv[i][0] != '-') {
			nargv[nargc++] = argv[i];
			continue;
		}
		switch (argv[i][1]) {
			case 'i': {	/* input file name */
				if( freopen(argv[++i],"r",stdin) == NULL ) {
					fprintf( stderr, "back: can't open %s\n", argv[i] );
					exit( 2 );
				}
				continue;
			}
	
			case 'o': {	/* output file name */
				if( freopen(argv[++i],"w",stdout) == NULL ){
					fprintf( stderr, "back: can't open %s\n", argv[i] );
					exit( 2 );
				}
				continue;
			}

			case 't': {
				tvflag++;
				continue;
			}

			case 'S': {
				Sflag++;
				continue;
			}

/* used only in the first pass
			case 'n':
				fartab = &argv[++i];
				continue;
*/

			case 'Y': {
				argv[i]++;
				argv[i][0] = '-';

			case 'M':
# ifdef VARALIGN
			case 'w':
# endif
				nargv[nargc++] = argv[i];
				continue;
			}
		}
	}
	nargv[nargc] = 0;
	retcode = mainp2(nargc, nargv);
#else /* defined( NARGS ) */
	retcode = mainp2(argc, argv);
#endif /* defined( NARGS ) */
	/* Write performance stats to file */
	{
	FILE *perfile;
	
		ttime = times(&ptimes) - ttime;
		if ((perfile = fopen("comp.perf.info", "r")) != NULL) {
			fclose(perfile);
			if ((perfile = fopen("comp.perf.info", "a")) != NULL) {
				fprintf(perfile,"back\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\n",ttime,ptimes);
				fclose(perfile);
			}
		}
	}

	return(retcode);
}
#endif


