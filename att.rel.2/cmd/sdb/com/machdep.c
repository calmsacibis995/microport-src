/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
	/*  machdep.c: 1.10 9/7/83 */

/*
 ****		MACHINE and OPERATING SYSTEM DEPENDENT
 ****		Routines which deal with the run-time stack
 */

#include "head.h"
#include "coff.h"

extern BKPTR	bkpthead;
extern MSG		NOPCS;

long rdsym();			/* in symt.c */
extern SYMENT syment;		/* rdsym() stores symbol table entry */
extern AUXENT auxent[];		/* rdsym() stores auxiliary entry */
extern int gflag;		/* initfp() in symt.c sets */
extern FILHDR filhdr;
static int frameflg;		/* set if now past last frame */

#define EVEN	(-2)

/* initialize frame pointers to top of call stack */
/*  MACHINE DEPENDENT */
struct proct *
initframe() {
	register struct proct *procp;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "initfram();\n");
	if(debugflag > 1) {
#if vax || u3b
		fprintf(FPRT2,	"   &SDBREG(AP)=%#x", &SDBREG(AP) );
		fprintf(FPRT2, "   uu=%#x; AP=%#x;\n", uu, AP);
#else
#if u3b5
		fprintf(FPRT2,"  fp = %#x, ap=%#x\n",regvals[9],regvals[10]);
#endif
#endif
	}
#endif
	if ((pid == 0) && (fcor < 0 || fakecor)) {
		frameflg = 1;
		return(badproc);
	}

#if vax || u3b
	argp = SDBREG(AP);
	frame = SDBREG(FP);
#else
#if u3b5
	argp = regvals[10];
	frame = regvals[9];
#else
#if iAPX286
	frame = FP;
	if (MODEL(model) == M_SMALL)
	{
		argp = frame + 4;
	}
	else
	{
		argp = frame + 6;
	}
#endif
#endif
#endif
#if vax || u3b
	callpc = SDBREG(PC);
#else
#if u3b5
	callpc = regvals[15];
#else
#if iAPX286
	callpc = USERPC;
#endif
#endif
#endif

#if u3b || u3b5

#if DEBUG > 1
	if ( debugflag > 1 )
	{
		printf( "frame = %#x	USRSTACK = %#x\n", frame, USRSTACK );
		printf( "frameflg = %d\n", frameflg );
	}
#endif

	/* FP in 3B starts at USRSTACK and increases as stack grows */
	if (frame < USRSTACK) {	/* 3B ?? other checks ?? */
		frameflg = 1;
		return(badproc);
	}

#else
#if vax
#if DEBUG > 1
	if ( debugflag > 1 )
	{
		printf( "frame = %#x\n", frame );
		printf( "frameflg = %d\n", frameflg );
	}
#endif

	/* FP in VAX starts at maxstor = 1L << 31 and decreases */
	if ((frame & 0xf0000000) != 0x70000000) {
		frameflg = 1;
		return(badproc);
	}
#endif
#endif

	procp = adrtoprocp(callpc);
#if DEBUG > 1
	if(debugflag > 1)
	    fprintf(FPRT2,
		"	procp->pname=%s; argp=%#x; frame=%#x; callpc=%#x;\n",
			procp->pname, argp, frame, callpc);
#endif
	frameflg = 0;
	return(procp);
}

struct proct *
nextframe() {
#if iAPX286
	register struct proct *procp;
	unsigned int stackdisp,nstackdisp;
#if DEBUG > 1
	if(debugflag)
		fprint(FPRT2,
			"nextframe():   prev frame=%#lx;\n",frame);
#endif
	if (MODEL(model) == M_SMALL)
	{
		callpc = ((SDBREG(CS)<<16)+(((long)get(frame+2,DSP))&0xffff));
	}
	else
	{
		callpc = (((long)get(frame+4,DSP))<<16)+
				(((long)get(frame+2,DSP))&0xffff);	
	}
	stackdisp = frame; /* get the bottom 16 bits */
	nstackdisp = get(frame,DSP);
	if (nstackdisp <= stackdisp)
	{
		frameflg = 1;
		return(badproc);
	}
	frame = ((SDBREG(SS)<<16)+(((long)nstackdisp)&0xffff));
	if (MODEL(model) == M_SMALL)
	{
		argp = frame + 4;
	}
	else
	{
		argp = frame + 6;
	}
	procp = adrtoprocp(callpc);
	return(procp);
}
#else
	register struct proct *procp;
	union word word;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2,
			"nextframe():   prev frame=%#x(%#o);\n", frame, frame);
#endif
	callpc = get(NEXTCALLPC, DSP);
	argp = get(NEXTARGP, DSP);
#if vax || u3b
	frame = get(NEXTFRAME, DSP) & EVEN;
#else
#if u3b5
	frame = get(NEXTFRAME,DSP);
#endif
#endif
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "  :  callpc=%#x; argp=%#x; frame=%#x;\n",
				callpc, argp, frame);
#endif
#if vax || u3b
#if u3b
	if (callpc > USRSTACK)	/* 3B ?? other checks ?? */
#else
	if (callpc > 0x70000000)	/* error handler kludge */
#endif
	{
		fprintf( FPRT1, "nextframe(): callpc = %#x; 3B ??\n", callpc );
		callpc = get(argp+12, DSP);
		argp = get(NEXTARGP, DSP);
		frame = get(NEXTFRAME, DSP) & EVEN;
	}
#endif
#if u3b || u3b5

#ifdef DEBUG
	if ( debugflag > 1 )
	{
		printf( "frame = %#x	USRSTACK = %#x\n", frame, USRSTACK );
		printf( "frameflg = %d\n", frameflg );
	}
#endif

	if (frame < USRSTACK) {	/* 3B ?? other checks ?? */
		frameflg = 1;
		return(badproc);
	}
#else
#if vax
#if DEBUG > 1
	if ( debugflag > 1 )
	{
		printf( "frame = %#x\n", frame );
		printf( "frameflg = %d\n", frameflg );
	}
#endif

	if ((frame & 0xf0000000) != 0x70000000) {
		frameflg = 1;
		return(badproc);
	}
#endif
#endif
	procp = adrtoprocp(callpc-1);

#if u3b || u3b5
	word.w = get( procp->paddress, ISP );
#if u3b
	if( word.c[ 0 ] != 0x7A )	/* opcode for 3B 'save' instr */
#else
	if (word.c[0] != 0x10)		/* opcode for 3B5 'save' instr */
#endif
	{
		frameflg = 1;
		return( badproc );	/* cannot chain back anymore */
	}
#endif

#if DEBUG > 1
	if(debugflag > 1) {
		fprintf(FPRT2, "	nxtframe:\n	");
		fprintf(FPRT2,
			"procp->pname=%s; argp=0%o; frame=%#x; callpc=%#x;\n",
					procp->pname, argp, frame, callpc);
	}
#endif
	return(procp);
}
#endif

/* returns core image address for variable */
/* formaddr() should be in symt.c ?? */
ADDR
formaddr(class, addr)
register char class;
ADDR addr; {
#if DEBUG
	if (debug)
#if iAPX286
		fprintf(FPRT2, "formaddr(class=%d(%#x), addr=%#lx(%ld))\n",
#else
		fprintf(FPRT2, "formaddr(class=%d(%#o), addr=%#x(%d))\n",
#endif
					class,class, addr,addr);
#endif
	switch(class) {
	case C_REG:
	case C_REGPARM:
#if iAPX286
		printf("register variables not yet supported by iAPX286 SDB");
		return(addr);
#else
		return(stackreg(addr));
#endif
	case C_EXT:
	case C_STAT:
		return(addr);
	case C_MOS:
	case C_MOU:
	case C_MOE:
	case C_FIELD:			/* ?? */
		return(addr);
		
	case C_ARG:
#if iAPX286
		return(frame+addr);
#else
		return(argp+addr);
#endif
		
	case C_AUTO:
		return(frame+addr);	/* addr was negated in outvar() */

	default:
		fprintf(FPRT1,"Bad class in formaddr: %d(%#x)\n", class, class);
		return(0);
	}
}

/*
 *  stackreg(reg):
 * This routine is intended to be used to find the address of a register
 * variable.  The stack is searched backwards from the current frame
 * until the procedure with the register variable is found.  If the
 * register assigned to the variable appears on the stack, then its
 * address is returned.  Otherwise, the register variable is still in
 * the named register, so the register number is returned.  We distinguish
 * addresses from register numbers by noting that register numbers are less
 * than NUMREGS (16) and that stack addresses are greater.
 *
 *  MACHINE DEPENDENT
 */
/* new stackreg() for 3B Unix
 *	C stores 'n' registers starting with %R8 and going backwards
 *	can only get 'n' by looking at text -- bits 8->11 from beginning of proc
 */

#if u3b || u3b5
#define REGSAV1		8
#define ISREGSAV(xr,xn)	(((REGSAV1-xn) < xr) && (xr <= REGSAV1))
#if u3b
#define NUMREGSOFF	1
#define REGADDR(xr)	(frame - (REGSAV1-xr+1)*REGSIZE)
#else
#define NUMREGSOFF	(9 - (getval(procp->st_offset+1,'bu',DSP)&0x0f))
#define REGADDR(xr,xn)	(frame - (ADDR)(15-xr-xn)*REGSIZE)
#endif
ADDR
stackreg(reg)
ADDR reg;
{
	register struct proct *procp;
	ADDR regaddr;
	unsigned int nrs;
	/* unsigned int nn;	don't adjust addresses - see below */

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "stackreg(reg=%d);\n", reg);
#endif
	/*  if no procedure to search down to, return reg */
	if (proc[0] == '\0')
		return(reg);


	regaddr = reg;
	for (procp=initframe(); procp != badproc; procp=nextframe()) {
#if DEBUG > 1
		if (debugflag)
		    fprintf(FPRT2,
			"procp->pname=%s, sl_procp->pname=%s, proc=%s\n",
			procp->pname, sl_procp->pname, proc);
#endif
		if (sl_procp == procp) break;
#if u3b
		nrs = getval(procp->paddress +NUMREGSOFF, "bu", DSP);
		nrs >>= 4;	/* get high four bits */
		if (ISREGSAV(reg,nrs)) {
			regaddr = REGADDR(reg);
#else
		nrs = NUMREGSOFF;
		if (ISREGSAV(reg,nrs)) {
			regaddr = REGADDR(reg,nrs);
#endif
			/*  3B stores chars and shorts in registers
			 * right justified, even though they are stored
			 * left justified in core.  Register variables
			 * are stored in register image on the stack,
			 * and that is the way they should be treated,
			 * so that the calling function doesn't have
			 * to check again whether they are actually
			 * in the registers or not.
			 * nn = WORDSIZE - typetosize(sl_type, sl_size);
			 * if (nn > 0)
			 * 	regaddr += nn;
			 */
		}
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2,
				"  : frame=%#x, nrs=%#x, regaddr=%#x;\n",
					frame, nrs, regaddr);
#endif
	}
	if (procp == badproc) {
		fprintf(FPRT1, "Stackreg() error: frame=%#x\n", frame);
		regaddr = ERROR;
	}
	return(regaddr);
}

#else
#if vax
/*  VAX version - C stores 'n' registers arbitrarily.
 *	A mask (16 bits) is stored as the high order short in the word which is
 *	offset one word from the beginning of the frame (frame + WORDSIZE).
 *	The 0'th bit of this mask tells whether register 0 has been saved,
 *	the 1'st bit tells wheter register 1 has been saved, etc.
 *	Each saved register in the frame occupies another word of space,
 *	and they are stored in order, from lowest to highest numbered.
 *	The first saved register is at frame + 5 * WORDSIZE.
 */

#define REGOFF 5 * WORDSIZE
#define MASKOFF WORDSIZE
ADDR
stackreg(reg) {
	register int regfl, mask, i;
	register struct proct *procp;
	ADDR regaddr;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "stackreg(reg=%d);\n", reg);
#endif
	if (proc[0] == '\0')
		return(reg);
	regaddr = reg;
	regfl = 0x10000 << reg;
	for (procp=initframe(); procp!=badproc; procp=nextframe()) {
#if DEBUG > 1
		if (debugflag)
		    fprintf(FPRT2,
			"procp->pname=%s, sl_procp->pname=%s, proc=%s\n",
			procp->pname, sl_procp->pname, proc);
#endif
		if (sl_procp == procp)
			break;
		if ((mask = get(frame + MASKOFF)) & regfl) {
			regaddr = frame + REGOFF;
			for (i=0; i < reg; i++) {
				if (mask & 0x10000) regaddr += WORDSIZE;
				mask >>= 1;
			}
		}
#if DEBUG > 1
		if(debugflag > 1)
			fprintf(FPRT2,
				"	pname=%s; mask=%d; regaddr=0%o;\n",
					    procp->pname, mask<<reg, regaddr);
#endif
	}
	if (procp == badproc) {
		fprintf(FPRT1, "Stackreg() error: frame=%#x\n", frame);
		regaddr = ERROR;
	}
	return(regaddr);
}
#endif
#if iAPX286
ADDR
stackreg(reg)
ADDR reg;
{
ADDR regaddr;

	regaddr = ERROR;
	return(regaddr);
}
#endif
#endif

/* Print the stack trace: 't' and 'T' commands
 * set top (i.e. top != 0) to print just the top procedure
 * modified to print something even if a.out stripped
 */

#if u3b

/*  The 3B saves 13 words worth of registers before saving arguments */
#define NARGSTACK	( ((frame - argp) / WORDSIZE) -13)
#else
#if u3b5
#define NARGSTACK	(((frame - argp) / WORDSIZE) - 9)
#else
#if vax
/*  The number of words stored as arguments is in the first byte
 *	of the zero'th argument.  The remaining bytes of the word should
 *	be zero.  Argp is set to point to the first argument.
 */
#define NARGSTACK    (argp += WORDSIZE, \
			(narg = get(argp-WORDSIZE, DSP)) & ~0xff ? 0 : narg\
		     )
#else
#if iAPX286
#define NARGSTACK    6
#endif
#endif
#endif
#endif

prfrx(top) {
	int narg;		/* number of words that arguments occupy */
	long offs;		/* offset into symbol table */
	register char class;	/*storage class of symbol */
	register int endflg;	/* 1 iff cannot find symbolic names of
					more arguments */
	int subsproc = 0;	/* 1 iff not top function on stack */
	register char *p;	/* pointer to procedure (function) name */
	int line_num;		/* line number in calling function */
	register struct proct *procp;
	SYMENT *syp = &syment;
	AUXENT *axp = auxent;
	
	procp = initframe();
	if (frameflg) {		/*  no initial frame */
		if (pid == 0 && (fcor < 0 || fakecor))	{  /* usual error */
			errflg = "No process and no core file.";
			chkerr();
		}
		else {				/* unusual initial frame */
			return;
		}
	}
	do {
#if u3b || u3b5
		/*  3B crt0 (start) has an old fp of zero */
		if (get(NEXTFRAME, DSP) == 0)
			return;
#else
#if vax
		/*  VAX crt0 (start) gets a current fp of zero */
		if (frame == 0) return;
#endif
#endif
		p = procp->pname;
		if (eqstr("__dbsubc", p))	/*  3B ?? */
			return;
		if (procp == badproc) {		/*  if stripped a.out */
			printf("pc: 0x%lx;	args: (", callpc);
			endflg = 1;
		}
#if vax
		else if (p[0] == '_') {
			printf("%s(", p+1);
			endflg = 0;
		}
#endif
		else {
			printf("%s(", p);
			endflg = 0;
		}
		if (endflg == 0) {
			offs = procp->st_offset;
			do {		/*  in COFF, always just 1 ? */
				if( (offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
			} while (ISFCN(syp->n_type));
			class = syp->n_sclass;
			while (! ISARGV(class)) {
				if ((offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
				class = syp->n_sclass;
				if (ISFCN(syp->n_type)) {
					endflg++;
					break;
				}
			}
		}

		narg = NARGSTACK;
#if DEBUG > 1
		if(debugflag > 1)
			fprintf(FPRT2, "prfrx(): pname=%s; narg=%d;\n",
						procp->pname, narg);
#endif
		while (narg) {
			if (endflg) {
				printf("%d", get(argp, DSP));
				argp += WORDSIZE;
			} else {
				int length;
				if ((syp->n_type == T_STRUCT) ||
				    (syp->n_type == T_UNION))   {
				    /* print address of structure
				     * (so that structures of, e.g.
				     * 100 element arrays, are not dumped)
				     */
#if u3b || u3b5 || iAPX286
				    printf( "&%s=", syp->n_nptr );
#else
#if vax
				    /* VAX: skip leading underscore */
				    if (syp->n_nptr[0] == '_')
					printf("&%s=", syp->n_nptr+1);
				    else
					printf("&%s=", syp->n_nptr);
#endif
#endif
				    dispx(argp, "x", C_EXT, 
						(short) (-1), 0, DSP);
				    length = axp->x_sym.x_misc.x_lnsz.x_size;
				}
				else {
				    if (syp->n_type == T_ENUM)
					length =
					    axp->x_sym.x_misc.x_lnsz.x_size;
				    else
					length = typetosize(syp->n_type, 0);
#if u3b || u3b5 || iAPX286
				    printf("%s=", syp->n_nptr);
				    /* The address of a short or char is
				     * expected to be the left byte of the
				     * variable.  However, argp points to
				     * the left end of the WORD containing
				     * the parameter, which is right justified.
				     * The address is thus adjusted accordingly.
				     * length<WORDSIZE test needed, since length
				     * could be sizeof(double) > WORDSIZE.
				     *
				     * Alternate code:
				     *  dispx(argp, "", C_REGPARAM,
				     *	    (short)syp->n_type, 0, DSP);
				     * This has the same effect, since dispx
				     * sees a "register" variable (hence
				     * right justified), on the stack, and
				     * extracts the right part.  This is
				     * more efficient, but less clean, since
				     * parameters are not necessarily
				     * register variables, they only look that
				     * way.
				     */
				    if (length < WORDSIZE) {
					dispx(argp+WORDSIZE-length,"",
					    C_EXT,(short)syp->n_type,0,DSP);
				    } else {
					dispx(argp, "", C_EXT,
					    (short)syp->n_type, 0, DSP);
				    }
#else
#if vax
				    /* VAX: skip leading underscore */
				    if (syp->n_nptr[0] == '_')
				    {
					printf( "%s=", syp->n_nptr + 1 );
				    }
				    else
				    {
					printf( "%s=", syp->n_nptr );
				    }
				    dispx(argp, "", C_EXT, 
					    (short) syp->n_type, 0, DSP);
#endif
#endif
				}
				if (length > WORDSIZE) {
					argp += length;
					/*  adjust for more than 1 word */
					narg -= length/WORDSIZE -1;
				}
				/* bytes, shorts, longs pushed as ints */
				else {
					argp += WORDSIZE;
				}
			}
			do {
				if (endflg) break;
#if iAPX286
/* don't know the number of args so use narg as flag			*/
/* set to 2 does not matter if it goes negative as endflag cannot be	*/
/*	set in the inner loop of the above portion			*/
				narg = 2; 
#endif
				if ((offs = rdsym(offs)) == ERROR) {
					endflg++;
#if iAPX286
/* we dont know the number of parameters so when not found set narg = 1	*/
					narg = 1;
#endif
					break;
				}
				class = syp->n_sclass;
				if (ISFCN(syp->n_type)) {
					endflg++;
#if iAPX286
/* we dont know the number of parameters so when not found set narg = 1	*/
					narg = 1;
#endif
					break;
				}
			} while (! ISARGV(class));
			if (--narg != 0) printf(",");
		}
		printf(")");
#if DEBUG
		if (debug) fprintf(FPRT2, "  @ 0x%lx ", callpc);
#endif
		if (procp->sfptr != badfile)
			printf("   [%s",
				(procp->sfptr)->sfilename);
		if(gflag) {
			if ((line_num = adrtolineno(callpc-subsproc,procp)) > 0)
				printf(":%d", line_num);
		}
		if(procp->sfptr != badfile)
			printf("]");
		printf("\n");
		subsproc = 1;
		procp = nextframe();
	} while (!top && !frameflg);	/*  only one frame desired, or
						no frames left in backtrace */
/* Vax:	} while (((procp = nextframe()) != badproc) && !top);*/
}


/* machine dependent initialization */
sdbenter(xflag) {
#if vax
	mkioptab();
#endif
}

/* machine dependent exit code */
sdbexit() {
	exit(0);
}

#if u3b || u3b5 || iAPX286
/*  isubcall() to replace SUBCALL #define (more complicated on 3B) */
isubcall(loc, space)
long loc;
int space;
{
	register int opcode;
	union word word;

	word.w = get(loc,space);
	opcode = word.c[0];
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "isubcall(loc=%#lx, space=%d);	opcode=%#x\n",
				loc, space, opcode);
#endif
#if u3b
	return(opcode == 0x79 || opcode == 0x78 || opcode == 0x77);
#else
#if iAPX286
	return(opcode == 0xe8 || opcode == 0x8a || ((opcode == 0xff) && (((word.c[1]&0x10)==0x10)||((word.c[1]&0x18)==0x18))));
#else
	return(opcode == 0x2c);		/* opcode for call instruction */
#endif
#endif
}
#endif
