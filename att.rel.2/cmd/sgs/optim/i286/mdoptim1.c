/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   %W% - %E% */

/*
**	mdoptim1.c
**
**	This module contains the routines referenced from the machin
**	independant section of the Portable Code Improver
**
**	Other required functions are implement as macros in defs, or
**	as suuport routines in local.l.
**
**	The file mdoptim2.c contains the machine dependant optimising
**	routines.
**
**	The file mdoptim3.c contains support rouctines for the machine
**	dependant functions.
**
**
**	Functions supported in this module are:-
**
**	1. Predicate functions.
**
**	ishlp
**	isret
**
**	2. Information gathering.
**
**	newlab
**	getp
**	uses
**	sets
**
**	3. Modifying
**
**	putp
**	revbr
**
**	4. Miscellaneous
**
**	dstats
**
**
*/

# include "optim.h"

# define VAL	1	/* normal constant: $7 */
# define SEG	2	/* segment constant: $<s> */
# define OFF	3	/* offset constant: $<o> */

extern char *lookopr();
extern char *dst() ;
extern int mflag;
extern int mbits;
extern int mvalue;
extern char *optbl[];
extern char *oprtbl[];
extern int m;
int numauto;
int nspinc = 0;		/* useless sp (negative) increments */
int nmcmm = 0;		/* char c; c--; */
int nmcm = 0;		/* multiply by constant */
int nmlas = 0;		/* load of same reg after store */
int nmlsp = 0;		/* useless x -> reg -> x */
int nmmpc1 = 0;		/* move pairs collapsed to 1 */
int nmnuax = 0;		/* needless use of %ax */
int nmldes = 0;		/* replace 2 instructions with lds/les */
int nmlea = 0;		/* replace 2 instructions with lea */
int nmimd = 0;		/* can inc (dec) after fetching value */
int nmsh = 0 ;		/* shifts by constant in cx */
int nmshf = 0;		/* replace 3 shifts/rotates with multi-bit version */
int nmashf = 0;		/* additional shift/rotate collapsed into multi-bit */
int nmdeadr = 0;	/* useless operations on dead registers */
int nmenop = 0;		/* instructions which are effectively no-ops */
int nmlloads = 0;	/* redundant long word loads */
int nmloads = 0;	/* redundant loads of registers */
int nmsloads = 0;	/* redundant loads of segement registers */
int nmcop = 0;		/* changed to more efficient instruction */
int nminds = 0;		/* removal of needless adds to index register */
int nmcontst = 0;	/* change to jump after test on constant */
extern int ndisc;
unsigned scanreg();
unsigned setreg();
unsigned getreg();
unsigned getifreg();
NODE *generate() ;


/* *************************************************************************
** *************************************************************************
**
**	1. Predicate functions.
**
*/

/*
**	ishlp
**
**
**
*/

ishlp(p) NODE *p; { /* return true if a fixed label present */

	for (; p->op == LABEL || p->op == HLABEL; p=p->forw)
		if (ishl(p))
			return(true);
	return(false);
	}

/*
**	isret
**
**
**
*/

boolean
isret(p) NODE *p; { /* return true if p is a return */

	return ( p->op == RET ) || ( p->op == LRET ) ;
}


/* *************************************************************************
** *************************************************************************
**
**	2. Information gathering.
**
*/

/*
**	newlab
**
**
**
*/

char *
newlab() { /* generate a new label */

	static int lbn = 0;
	char *c;

	c = (char *) getspace(lbn < 100 ? 6 : (lbn < 1000 ? 7 : 8));
	sprintf(c, "..%d", lbn);
	lbn++;
	return(c);
}

/*
**	getp
**
**
**
*/

char *
getp(p) NODE *p; { /* return pointer to jump destination operand */

	if (p->op == RET || p->op == LRET || p->op == LJMP)
		return(NULL);
	else
		return(p->op1);
}

/*
**	uses
**
**
**
*/

unsigned
uses(p) NODE *p; { /* set register use bits */

	unsigned using;

	if (p->op == MISC) {
		return(0);
	}
	using = addruse(p->op1) | addruse(p->op2);
	switch (p->op) {
	case RCL:  case RCLB:  case RCR:  case RCRB:
	case ROL:  case ROLB:  case ROR:  case RORB:
	case SAL:  case SALB:  case SAR:  case SARB:
	case SHL:  case SHLB:  case SHR:  case SHRB:
			return(using | dirref(p->op1) | dirref(p->op2));
	case SAHF:
		return(AH);
	case AAM:  case CBW:  case DAA:  case DAS:
		return(AL);
	case XLAT:
		return(AL | BX);
	case SCAB:  case SSTOB:
		return(AL | DI | ES);
	case OUTB:
		if (p->op1 == NULL)
			return(AL | DX);
		else
			return(AL);
	case IMULB:  case MULB:
		using |= (AL | dirref(p->op1));
		break;
	case AAA:  case AAD:  case AAS:  case CWD:
		return(AX);
	case SCA:  case SSTO:
		return(AX | DI | ES);
	case OUT:
		if (p->op1 == NULL)
			return(AX | DX);
		else
			return(AX);
	case DIV:  case IDIV:
		using |= (AX | DX | dirref(p->op1));
		break;
	case IMUL3A: case IMUL3B: case IMUL3C: case IMUL3D:
	case IMUL3S: case IMUL3DI:
		break;
	case IMUL:
		if(p->op2 == NULL || !iscon(p->op1))
			using |= (AX | dirref(p->op1));
		else using |= dirref(p->op2); /* imul const,reg */
		break;
	case DIVB:  case IDIVB:  case MUL:
		using |= (AX | dirref(p->op1));
		break;
	case LOOP:  case LOOPE:  case LOOPNE:  case LOOPNZ:
	case LOOPZ:  case REP:  case REPNZ:  case REPZ:
		return(CX);
	case INB:  case IN:
		if (p->op1 == NULL)
			return(DX);
		else
			return(0);
	case SLODB:  case SLOD:
		return(SI);
	case SCMP:  case SCMPB:  case SMOV:  case SMOVB:
		return(SI | DI | ES | DS);
	case DEC:  case DECB:  case INC:  case INCB:
	case NEG:  case NEGB:  case NOT:  case NOTB:
	case LEA:  case MOV:  case MOVB:  case LDS:
	case LES:  case PUSH:
		using |= dirref(p->op1);
		break;
	case CMP:  case CMPB:  case XCHG:  case XCHGB:
	case ADC:  case ADCB:  case ADD:  case ADDB:
	case AND:  case ANDB:  case OR:  case ORB:
	case SBB:  case SBBB:  case SUB:  case SUBB:
		using |= (dirref(p->op1) | dirref(p->op2));
		break;
	case XOR:  case XORB:
		/* if xor %ax,%ax (say) then value in %ax is irrelevent */ 
		if(dirref(p->op1) != dirref(p->op2))
			using |= (dirref(p->op1) | dirref(p->op2));
		break;
	case TEST:  case TESTB:
		using |= dirref(p->op1);
		if (p->op2 != NULL)
			using |= dirref(p->op2);
		break;
	case CALL:  case LCALL:
		/*
		 * All arithmetic regs are used for
		 * long arith calls
		 */
		if( !strcmp("_lmul",p->op1) ||
		    !strcmp("_ldiv",p->op1) || 
		    !strcmp("_lmod",p->op1) || 
		    !strcmp("_uldiv",p->op1) || 
		    !strcmp("_ulmod",p->op1) ) 
			using |= (AX|BX|CX|DX);
		else if( !strcmp("_mcount",p->op1)) {
			if( p->op == CALL) using |= SI;
			else using |= (SI|BX);
		}
		break;
	case JMP:
	case LJMP:
		break;
	case LEAVE:
		using = (BP) ;
		break ;
	case ENTER:
		using = (BP | SP) ;
		break ;

	/*
	 * The following instructions are for the iAPX287 math
	 * co-processor. These instructions can use the iAPX286
	 * registers for memory reference. Therefore return 'usage'.
	 */
	case FIADD:	case FIADDL:	case FICOM:	case FICOML:
	case FICOMP:	case FICOMPL:	case FIDIV:	case FIDIVL:
	case FIDIVR:	case FIDIVRL:	case FILD:	case FILDL:
	case FILDLL:	case FIMUL:	case FIMULL:	case FIST:
	case FISTL:	case FISTP:	case FISTPL:	case FISTPLL:
	case FISUB:	case FISUBL:	case FISUBR:	case FISUBRL:
	case FLDCW:	case FSTCW:	case FNSTCW:	case FSTSW:
	case FNSTSW:	case FSTENV:	case FNSTENV:	case FLDENV:
	case FSAVE:	case FNSAVE:	case FRSTOR:
	case FBLD:	case FBSTP:
	case FADDS:	case FADDL:	case FCOMS:	case FCOML:
	case FCOMPS:	case FCOMPL:	case FDIVS:	case FDIVL:
	case FDIVRS:	case FDIVRL:	case FLDS:	case FLDL:
	case FLDT:	case FMULS:	case FMULL:	case FSTS:
	case FSTL:	case FSTPS:	case FSTPL:	case FSTPT:
	case FSUBS:	case FSUBL:	case FSUBRS:	case FSUBRL:
		break;
	
	
	case ARPL:   case BOUND:  case CLC:   case CLD:     case CLI:   
	case CLR:    case CLRB:   case CMC:   case CTS:     case ESC:   
	/*
	 * The instructions beginning with an F are for the iAPX287
	 * math co-processor. These instructions make no use of
	 * iAPX286 registers. The ones up to the next comment take
	 * no operands.
	 */

	case FINIT:	case FCLEX:	case F2XM1:	case FABS:
	case FCHS:	case FCOMPP:	case FDECSTP:	case FINCSTP:
	case FLD1:	case FLDL2E:	case FLDL2T:	case FLDLG2:
	case FLDLN2:	case FLDPI:	case FLDZ:	case FNCLEX:
	case FNINIT:	case FNOP:	case FPATAN:	case FPREM:
	case FPTAN:	case FRNDINT:	case FSCALE:	case FSETPM:
	case FSQRT:	case FTST:	case FWAIT:	case FXAM:
	case FXTRACT:	case FYL2X:	case FYL2XP1:
	/*
	 * The instructions beginning with an F are for the iAPX287
	 * math co-processor. These instructions make no use of
	 * iAPX286 registers. These instructions all use the
	 * iAPX287 stack.
	 */
	case FADD:	case FADDP:	case FCOM:
	case FCOMP:	case FDIV:	case FDIVP:
	case FDIVR:	case FDIVRP:	case FFREE:
	case FLD:	case FMUL:	case FMULP:
	case FST:	case FSTP:	case FSUB:
	case FSUBP:	case FSUBR:	case FSUBRP:
	case FXCH:
	case HLT:    case INS:    case INT:   case INTO:    case IRET:   
	case JA:     case JAE:    case JB:    case JBE:     case JCXZ:   
	case JE:     case JG:     case JGE:   case JL:      case JLE:   
	case JNA:    case JNAE:   case JNB:   case JNBE:    case JNE:   
	case JNG:    case JNGE:   case JNL:   case JNLE:    case JNO:   
	case JNP:    case JNS:    case JNZ:   case JO:      case JP:   
	case JPE:    case JPO:    case JS:    case JZ:      case LAHF:   
	case LAR:    case LGDT:   case LIDT:  case LLDT:    case LMSW:   
	case LOCK:   case LRET:   case LSL:   case LTR:     case OUTS:   
	case POP:    case POPA:   case POPF:  case PUSHA:   case PUSHF:   
	case RET:    case SGDT:   case SIDT:  case SLDT:    case SMSW:   
	case STC:    case STD:    case STI:   case STR:     case VERR:   
	case WAIT:   
		using = 0 ;
		break ;
	default:
		if (using != 0)
			fprintf(stderr, "bad operator in uses():%d\n", p->op);
	}
	return(using);
}

/*
**	sets
**
**
**
*/

unsigned
sets(p) NODE *p; { /* set register destination bits */

	char *cp;

	switch (p->op) {
	case LAHF:  case CBW:
		return(AH);
	case DAA:  case DAS:  case INB:  case XLAT:
		return(AL);
	case SLODB:
		return(AL | SI);
	case AAA:  case AAD:  case AAM:  case AAS:
	case DIVB:  case IDIVB:  case IMULB:  case IN:
	case MULB:
		return(AX);
	case IMUL:
		if(p->op2 == NULL) return(AX|DX);
		else return(dirref(p->op2));
	case IMUL3A:
		return(AX);
	case IMUL3B:
		return(BX);
	case IMUL3C:
		return(CX);
	case IMUL3D:
		return(DX);
	case IMUL3S:
		return(SI);
	case IMUL3DI:
		return(DI);
	case DIV:  case IDIV:  case MUL:
		return(AX | DX);
	case SLOD:
		return(AX | SI);
	case LOOP:  case LOOPE:  case LOOPNE:  case LOOPNZ:
	case LOOPZ:  case REP:  case REPNZ:  case REPZ:
		return(CX);
	case SCA:  case SCAB:  case SSTO:  case SSTOB:
		return(DI);
	case CWD:
		return(DX);
	case XCHG:  case XCHGB:
		return(dirref(p->op1) | dirref(p->op2));
	case LDS:
		return(DS | dirref(p->op2));
	case LES:
		return(ES | dirref(p->op2));
	case SCMP:  case SCMPB:  case SMOV:  case SMOVB:
		return(SI | DI);
	/*
	 * small model
	 */
	case CALL:
		return( AX | BX | CX | DX | SI | DI ) ;
  	case LCALL:
		return( AX | BX | CX | DX | SI | DI | ES | DS ) ;
	default:
		if ((cp = dst(p)) != NULL)
			return(dirref(cp));
		else
			return(0);
	}
}


/* *************************************************************************
** *************************************************************************
**
**	3. Modifying
**
*/

/*
**	putp
**
**
**
*/

putp(p,c) NODE *p; char *c; { /* insert pointer into node */

	p->op1 = c;
}

/*
**	revbr
**
**
**
*/

revbr(p) register NODE *p; { /* reverse branch in node p */

	switch (p->op) {
		case JA: p->op = JNA; p->opcode = "jna"; break;
		case JAE: p->op = JNAE; p->opcode = "jnae"; break;
		case JB: p->op = JNB; p->opcode = "jnb"; break;
		case JBE: p->op = JNBE; p->opcode = "jnbe"; break;
		case JE: p->op = JNE; p->opcode = "jne"; break;
		case JG:  p->op = JNG; p->opcode = "jng"; break;
		case JGE: p->op = JNGE; p->opcode = "jnge"; break;
		case JL: p->op = JNL; p->opcode = "jnl"; break;
		case JLE: p->op = JNLE; p->opcode = "jnle"; break;
		case JNA: p->op = JA; p->opcode = "ja"; break;
		case JNAE: p->op = JAE; p->opcode = "jae"; break;
		case JNB: p->op = JB; p->opcode = "jb"; break;
		case JNBE: p->op = JBE; p->opcode = "jbe"; break;
		case JNE: p->op = JE; p->opcode = "je"; break;
		case JNG: p->op = JG; p->opcode = "jg"; break;
		case JNGE: p->op = JGE; p->opcode = "jge"; break;
		case JNL: p->op = JL; p->opcode = "jl"; break;
		case JNLE: p->op = JLE; p->opcode = "jle"; break;
		case JNO: p->op = JO; p->opcode = "jo"; break;
		case JNP: p->op = JP; p->opcode = "jp"; break;
		case JNS: p->op = JS; p->opcode = "js"; break;
		case JNZ: p->op = JZ; p->opcode = "jz"; break;
		case JO: p->op = JNO; p->opcode = "jno"; break;
		case JP: p->op = JNP; p->opcode = "jnp"; break;
		case JPE: p->op = JPO; p->opcode = "jpo"; break;
		case JPO: p->op = JPE; p->opcode = "jpe"; break;
		case JS: p->op = JNS; p->opcode = "jns"; break;
		case JZ: p->op = JNZ; p->opcode = "jnz"; break;
	}
}


/* *************************************************************************
** *************************************************************************
**
**	4. Miscellaneous
**
*/

/*
**	dstsats	
**
**
**
*/

extern int model;
dstats() { /* print stats on machine dependent optimizations */

	fprintf(stderr,(model?"large":"small"));
	fprintf(stderr," model assumed for assembler\n");
	dprintf(nspinc,"enter(s) with no local data");
	dprintf(nmcmm,"useless instructions on decr char");
	dprintf(nmcm,"instructions saved on constant multiply");
	dprintf(nmlas,"loads after store");
	dprintf(nmlsp,"useless load/store instructions");
	dprintf(nmmpc1,"move pairs collapsed to one");
	dprintf(nmnuax,"needless uses of %ax");
	dprintf(nmldes,"conversions to lds/les");
	dprintf(nmlea,"conversions to lea");
	dprintf(nmimd,"needless dec (inc) after move");
	dprintf(nmsh,"needless shifts by cl for constants");
	dprintf(nmshf,"triple single-bit shifts converted to multi-bit");
	dprintf(nmashf,"additional single-bit shifts collapsed into multi-bit");
	dprintf(nmdeadr,"operations on dead registers");
	dprintf(nmenop,"effective no-ops");
	dprintf(nmcop,"changed to another more efficient instruction");
	dprintf(nmlloads,"redundant long word loads");
	dprintf(nmloads,"redundant register loads");
	dprintf(nmsloads,"redundant segment register loads");
	dprintf(nminds,"needless adds to index register");
	dprintf(nmcontst,"jump changed after test on constant");
}
