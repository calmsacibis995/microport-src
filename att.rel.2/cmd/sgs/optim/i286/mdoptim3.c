/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   %W% - %E% */


/*
**	mdoptim3.c
**
**	This module contains support routines for all of the machine
**	dependant routines of the optimiser.
**
**
*/

#include "optim.h"



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
extern int numauto;
extern int nspinc;	/* useless sp (negative) increments */
extern int nmcmm;	/* char c; c--; */
extern int nmcm;	/* multiply by constant */
extern int nmlas;	/* load of same reg after store */
extern int nmlsp;	/* useless x -> reg -> x */
extern int nmmpc1;	/* move pairs collapsed to 1 */
extern int nmnuax;	/* needless use of %ax */
extern int nmlea;	/* replace 2 instructions with lea */
extern int nmimd;	/* can inc (dec) after fetching value */
extern int nmshf;	/* replace 3 shifts/rotates with multi-bit version */
extern int nmashf;	/* additional shift/rotate collapsed into multi-bit */
extern int nmdeadr;	/* useless operations on dead registers */
extern int nmenop;	/* instructions which are effectively no-ops */
extern int nmloads;	/* redundant loads of registers */
extern int nmcop;	/* changed to more efficient instruction */
extern int ndisc;

unsigned uses();
unsigned sets();
unsigned scanreg();
unsigned setreg();
unsigned getreg();
unsigned getifreg();
NODE *generate() ;

# define numregs 20

char *regtbl[numregs] = {"ah","al","ax","bh","bl","bp","bx","ch","cl","cs",
			 "cx","dh","di","dl","ds","dx","es","si","sp","ss"};

char *regvals[13];

/*
**
**
**
**
**
*/

dprintf(i,s) int i; char *s; { /* print non-zero stats on stderr */

	if (i) fprintf(stderr, "%d %s\n",i,s);
}

/*
**
**
**
**
**
*/

setauto(str) char *str; { /* set indicator for number of autos */

	numauto = 0;
	while (*str++ != ',') ;
	if (*str != '0')
		numauto = 1;
}

/*
**
**	addruse
**
**	find register use in addressing modes
**	don't forget implied use by some operands
*/

int model = 0; /* 1 means large model.
		** If a (long) lret is encountered in
		** lookopc then model is set to1 
		*/
unsigned
addruse(cp) register char *cp;  
{

	char firstchar ;
	unsigned override = 0 ;
	unsigned using = 0;

	if (cp == NULL)
		return(0);

	/* 
	 * check for segment override
	 */

	if ( *cp == '*')
		cp++ ;	/* inc over indirection (mainly lcall * for data) */

	if (*cp == '%' && cp[3] == ':') {
		override = getreg(++cp);
		cp += 3;
	}
	else 
		firstchar = *cp ;

	while (*cp != '\0')
		if (*cp == '(') {
			using |= getreg(cp+2);
			if (*(cp+4) == ',')
				using |= getreg(cp+6);
			break ;
		} else
			cp++;

	if( override )
		return ( using | override ) ;

	if( using )
	{
		if( using & (BP | SP) )
			return ( using | SS ) ;

		if ( using & DI )
			return ( using | (ES | DS ) ) ;

		return ( using | DS ) ;
	}

	if( (firstchar == '%' ) || (firstchar == '$') )
		return(using);

	if(model) return(using); /* large model */
	return ( using | DS ) ;
}

/*
**
**
**
**
**
*/

unsigned
dirref(cp) char *cp; { /* find any direct use of register */

	register char *cpc;

	if (cp == NULL)
		return(0);
	if (*cp != '%')
		return(0);
	cpc = cp + 3;
	if (*cpc == ':')
		return(0);
	cp++;
	return(getreg(cp));
}

/*
**
**
**
**
**
*/

char *
dst(p) register NODE *p; { /* return pointer to dst operand string */

	switch (p->op) {
	case DEC: case DECB: case INC: case INCB:
	case CLR: case CLRB: case NEG: case NEGB:
	case NOT: case NOTB: case POP: case FSTSW:
		return(p->op1);

	case ADD: case ADDB: case DIVB: case IDIVB:
	case IMULB: case MULB: case SBB: case SBBB:
	case SUB: case SUBB: case AND: case ANDB:
	case MOV: case MOVB: case LDS: case LEA: case LES:
	case OR: case ORB: case XOR: case XORB:
		return(p->op2);

	case IMUL3A:
		return("%ax");
	case IMUL3B:
		return("%bx");
	case IMUL3C:
		return("%cx");
	case IMUL3D:
		return("%dx");
	case IMUL3S:
		return("%si");
	case IMUL3DI:
		return("%di");

	case RCL: case RCLB: case RCR: case RCRB:
	case ROL: case ROLB: case ROR: case RORB:
	case SAL: case SALB: case SAR: case SARB:
	case SHL: case SHLB: case SHR: case SHRB:
		if (p->op2 == NULL)
			return(p->op1);
		return(p->op2);

	default:
		return(NULL);
	}
}

/*
**
**
**
**
**
*/

unsigned
getreg(cp) char *cp; { /* return bit encoding of 8 or 16 bit register */

	register int i;
	register char c1,c2;
	static unsigned regbits[numregs] = {AH,AL,AX,BH,BL,BP,BX,CH,CL,CS,
					    CX,DH,DI,DL,DS,DX,ES,SI,SP,SS};

	c1 = *cp++;
	c2 = *cp;
	for (i = 0; i < numregs; i++)
		if (regtbl[i][0] == c1 && regtbl[i][1] == c2)
			return(regbits[i]);
	return(0);
}

/*
**
**
**
**
**
*/

unsigned
regnum(cp) char *cp; { /* return index into value array of register named */

	register int i;
	register char c1,c2;
	static unsigned regnums[numregs] = {1+RH, 1+RL, 1,   2+RH,
					    2+RL, 8,    2,   3+RH,
					    3+RL, 9,    3,   4+RH,
					    6,    4+RL, 10,  4,
					    11,   5,    7,   12};

	if (cp[0] != '%' || cp[3] != '\0')
		return(0);
	c1 = cp[1];
	c2 = cp[2];
	for (i = 0; i < numregs; i++)
		if (regtbl[i][0] == c1 && regtbl[i][1] == c2)
			return(regnums[i]);
	return(0);
}

/*
**
**
**
**
**
*/

boolean
isdead(cp,p) char *cp; NODE *p; { /* true iff *cp is dead after p */

	if (*cp == '%' && cp[3] != ':' && (p->nlive & getreg(++cp)) == 0)
		return(true);
	else
		return(false);
}

/*
**
**
**
**
**
*/

boolean
iscon(cp) char *cp; { /* true if arg is constant (1st char is $) */

	return(*cp == '$');
}

/*
**
**
**
**
**
*/

int
contype(cp) char *cp; { /* return class of constant */

	if (*cp++ != '$')
		return(0);
	if (*cp++ != '<')
		return(VAL);
	if (*cp == 's')
		return(SEG);
	else
		return(OFF);
}

/*
**
**
**
**
**
*/

long
conval(cp) char *cp; { /* returns value of constant if possible, else -1l */

	long i;
	char *fmt;
	char key = '$';

	if (*cp++ != '$')
		return(false);
	if (*cp == '<') {
		cp++;
		key = *cp++;
		cp++;
	}
	switch (*cp) {
	case '0':
		if (cp[1] == 'x')
			fmt = "0x%lx%*c";
		else
			fmt = "%lo%*c";
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
		fmt = "%ld%*c";
		break;
	case '[':
		fmt = "[%ld]%*c";
		break;
	default:
		return(-1l);
	}
	if (sscanf(cp,fmt,&i) != 1)
		return(-1l);
	switch (key) {
	case '$':
		return(i);
	case 'o':
		return(i & 0xffl);
	case 's':
		return(i & 0xfff00l);
	}
}

/*
**
**
**
**
**
*/

int
ispow2(cp) char *cp; { /* return log 2 if cp is power of two, else -1 */

	register i;
	long l;

	if (iscon(cp))
		l = conval(cp);
	else
		return(-1);
	if (l <= 0 || (l & (l - 1)))
		return(-1);
	for (i = 0; l > 1; ++i)
		l >>= 1;
	return(i);
}

/*
**
**
**
**
**
*/

moveln(to, id)
NODE *to;
IDTYPE id;
{
	if (id == IDVAL)
		return;
	if (to->uniqid == IDVAL || to->uniqid > id) {
		to->uniqid = id;
		return;
	}
}

/*
**
**
**
**
**
*/

zapinst(p, n)
NODE *p;
int n;
{
	int i;
	NODE *f,*l;
	IDTYPE id;

	f = p;
	id = IDVAL;
	for (i = 0; i < n; i++) {
		if (id == IDVAL || (p->uniqid != IDVAL && p->uniqid < id))
			id = p->uniqid;
		l = p;
		p = p->forw;
	}
	moveln(l->forw, id);
	f->back->forw = l->forw;
	l->forw->back = f->back;
	ndisc += n;
}

/*
**
**
**
**
**
*/

NODE *
generate (p, oc, o1, o2) /* generate a new instruction node */
	NODE *p; /* -> node after which insertion is to be made */
	char *oc, *o1, *o2;
{
	register NODE *t;

	t = GETSTR(NODE);
	t->op = lookopc(oc);
	t->opcode = COPY(optbl[m], 0);
	t->op1 = (o1 == NULL) ? NULL : lookopr(o1);
	t->op2 = (o2 == NULL) ? NULL : lookopr(o2);
#ifdef LIVEDEAD
	t->nlive = uses ( t ) | LIVEREGS ;
	t->ndead = sets ( t ) ;
#endif
#ifdef IDVAL
	t->uniqid = p->uniqid ;
#endif
	APPNODE(t, p);
	return(t);
}

/*
**
**
**
**
**
*/

asmult (p, m) /* replace mult by adds and shifts */
	NODE *p;
	int m; /* the (small) constant multiplier */
{
	int i;
	boolean firstadd = true;


	p->op1 = p->forw->op1;
	zapinst(p->forw, 1);
	p->op2 = "%dx";

	for (i = 0; i < mbits; i++) {
		if (m & 1) {
			if (firstadd) {
				firstadd = false;
				p = generate(p, "mov", "%dx", "%ax");
				p->nlive |= (AX | DX);
			}
			else {
				p = generate(p, "add", "%dx", "%ax");
				p->nlive |= (AX | DX);
			}
			ndisc--;
		}
		if (m >>= 1) {
			p = generate(p, "shl", "%dx", (char *) NULL);
			p->nlive |= (AX | DX);
			ndisc--;
		}
		else
			break;
	}
}

/*
**
**
**
**
**
*/

unsigned
getifreg(cp) char *cp; { /* return encoding of reg if reg, else 0 */

	if (*cp == '%' && cp[3] != ':')
		return(getreg(++cp));
	return(0);
}

/*
**
**
**
**
**
*/

setret(str)
char *str;
{
	int n;

	if ( !sscanf(str + 8, "%*d,%d\n", &n) ) {
		fprintf(stderr, "can't parse:%s\n", str);
		retreg = AX|DX;
	} else switch (n) {
		case 0:
			retreg = 0;
			break;
		case 1:
			retreg = AX;
			break;
		default:
			fprintf(stderr, "invalid return register count:%d\n", n);
		case 2:
			retreg = AX|DX;
			break;
	}
}


/*
**	lowestaddr
**
**	returns lowest of two address specifications that differ by 2 bytes
**	otherwise NULL
**
**	CHECK
**		must check that the lowest of the two variables selected is
**	the offset part of the combo thats what the flag is used for - if
**	it is true (non-zero) then the first operand selects the segment
**	part (and should therefor be at the higher memory address.
**	
**	used in optimising to lds/les
**
**	address forms will be of the form :-
**
**	name / name+2
**	X(%bp) / X-2(%bp)
**
*/

char *
lowestaddr ( p1 , p2 , flag )
char *p1 , *p2 ;
{
	unsigned offreg ;
	char *tp1 , *tp2 ;
	int val1 ,v01,v11,v21,v31,v41,v51 ,
	    val2 ,v02,v12,v22,v32,v42,v52 ;

	if((dirref(p1) != dirref(p2)) || ((offreg=addruse(p1)) != addruse(p2) ))
		return( NULL ) ;

	tp1 = p1 ;
	tp2 = p2 ;

	if( (offreg & (BP | BX | SI | DI) ) )
	{
		/*
		 *	both of form [-]number(%reg)
		 */

	}
	else
	{
		/*
		 *	could both be of form name [+ something]
		 *
		 */

		while ( *tp1 == *tp2 )
		{
			if (*tp1 == '+' )
				break ;
			tp1++;tp2++;
		}

		if ( !((!*tp1 || (*tp1=='+')) && (!*tp2 || (*tp2=='+')) ) )
			return (NULL) ;

	}

	val1=v01=v11=v21=v31=v41=v51 = 0 ;
	val2=v02=v12=v22=v32=v42=v52 = 0 ;

	if( *tp1 )
		sscanf ( tp1 , "%d+%d+%d+%d+%d" , &v01,&v11,&v21,&v31,&v41,&v51 ) ;

	if( *tp2 )
		sscanf ( tp2 , "%d+%d+%d+%d+%d" , &v02,&v12,&v22,&v32,&v42,&v52 ) ;

	val1=v01+v11+v21+v31+v41+v51 ;
	val2=v02+v12+v22+v32+v42+v52 ;

	if ( (val1<val2) && ((val2-val1)==2) )
	{
		/*
		 *	the first operand is the lower and therefor
		 *	it should be offset part ie flag not set
		 */

		if(flag)
			return NULL ;
		else
			return p1 ;
	}
	else
	if ( (val2<val1) && ((val1-val2)==2) )
	{
		if(flag)
			return p2 ;
		else
			return NULL ;
	}

	return( NULL ) ;
}

/*
**	sideeffect
**
**	returns true if instruction can change memory location
**
*/

boolean 
sideeffect( p )
NODE *p;
{
	switch(p->op) {

	/* unconditionally have side effects */

	case CALL: case LCALL: case SMOV: case SMOVB: case SSTO: case SSTOB:
		return( true ) ;

	/* only op1 */

	case DEC:    case DECB:   case INC:   case INCB: case NEG: case NEGB:
	case NOT:    case NOTB:   case POP:   case POPA: case POPF:
	case FSTCW:  case FNSTCW: case FSTENV:  case FNSTENV:  case FSAVE:
	case FNSAVE: case FBSTP:  case FSTSW:  case FNSTSW:  case FIST:
	case FISTL:  case FISTP:  case FISTPL:  case FISTPLL:  case FSTS:
	case FSTL:   case FSTPS:  case FSTPL:  case FSTPT:  
		return( (addruse(p->op1) ? true : false ) ) ;

	/* only op2 */

	case MOV: case MOVB: case AAD: case ADC: case ADCB: case ADD:
	case ADDB: case DIV: case DIVB: case IDIV: case IDIVB: case IMUL:
	case IMULB: case MUL: case MULB: case SBB: case SBBB: case SUB:
	case SUBB: case AND: case ANDB: case OR: case ORB: case XOR:
	case XORB:
		return( (addruse(p->op2) ? true : false ) ) ;

	/*either*/

	case RCL: case RCLB: case RCR: case RCRB: case ROL: case ROLB:
	case ROR: case RORB: case SAL: case SALB: case SAR: case SARB:
	case SHL: case SHLB: case SHR: case SHRB: case XCHGB: case XCHG:

		return( (addruse(p->op1)|addruse(p->op2)) ? true : false) ;
	default:
		return( false ) ;
	}
}

/*
**	addtogether
**
**	t is a pointer to a pointer to an op involving register indirection
**	a is a pointer to  a constant op to be added to t
*/

addtogether( t , a )
char **t ;
char *a ;
{
	char *p ;
	char first ;
	char indirect[2] ;
	char override[5] ;
	int i ;

	p = *t ;
	first = *p ;
	indirect[0] = override[0] = '\0' ;

	if( first == '*' )
	{
		indirect[0] = '*' ;
		indirect[1] = '\0' ;
		first = *(++p) ;
	}

	if ( first== '%')
	{
		for(i=0;i<4;i++)
			override[i] = *p++ ;
		override[i] = '\0' ;
		first = *p ;
	}

	*t = getspace(20) ;

	if ( first == '(' )
		sprintf( *t , "%s%s%s%s" , indirect,override,a+1,p);
	else
		sprintf( *t , "%s%s%s+%s" , indirect,override,a+1,p);
}


/*
**	xsets
**
**	like set but returns whole register for ah/al bh/bl etc
*/

unsigned
xsets( p ) 
NODE *p ;
{
	unsigned t ;

	t = sets ( p ) ;

	return(  t | 
		( (t&AX) ? AX : 0 ) |
		( (t&BX) ? BX : 0 ) |
		( (t&CX) ? CX : 0 ) |
		( (t&DX) ? DX : 0 )    ) ;
}
