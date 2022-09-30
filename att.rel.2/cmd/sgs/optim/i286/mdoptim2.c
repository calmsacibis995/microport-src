/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   %W% - %E% */

/*
**	mdoptim2.c
**
**	This module contains the machine dependant optimisation routines.
**
**	They are:-
**
**	bboptim
**
**	Called once per basic block by the machine independant optimiser
**	These calls are made ebefore live/dead register analysis has been
**	performed.
**
**	slithole
**
**	Called by window ( 1 , slithole ) after machine independant
**	optimisations are complete.
**
**	blackhole
**
**	Called by window ( 2 , blackhole ) after machine independant
**	optimisations are complete.
**
**	peephole
**
**	Called by window ( 3 , peephole ) after all the above optimisations
**	are complete.
**
**	pothole
**
**	Called by window ( 4 , pothole ) after all above optimisations are
**	complete.
**
**	There is probably considerable room for improvement in this process
**	investigations should be carried out into the ordering of the 
**	optimisations and possible itterations - but, as usual, there
**	will probably not be time.
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
extern int nmldes;	/* replace 2 instructions with lds/les */
extern int nmlea;	/* replace 2 instructions with lea */
extern int nmimd;	/* can inc (dec) after fetching value */
extern int nmsh;	/* constant shifts */
extern int nmshf;	/* replace 3 shifts/rotates with multi-bit version */
extern int nmashf;	/* additional shift/rotate collapsed into multi-bit */
extern int nmdeadr;	/* useless operations on dead registers */
extern int nmenop;	/* instructions which are effectively no-ops */
extern int nmlloads;	/* redundant loads of long words */
extern int nmloads;	/* redundant loads of registers */
extern int nmsloads;	/* redundant loads of segment registers */
extern int nmcop;	/* changed to more efficient instruction */
extern int nminds;	/* removal of needless add to index register */
extern int nmcontst;	/* jump after test on constant removed/changed to jmp */
extern int ndisc;
unsigned movpat();
unsigned short globpt();
int nodeswap;


/* ***************************************************************************
** ***************************************************************************
**
**
**			BBOPTIM
**
*/

int
bboptim(pf, pl)
NODE *pf, *pl;	/* ptrs to first and last nodes of block */
{
	/*
	 *	killed - counts number of instructions zapped
	 *		 this is returned as the result of bboptim
	 */

	int		killed = 0 ;
	NODE	*p ;

	/*
	 *	look for final redundant adjust of sp
	 */
	
	p = pl ;
	if ( (p->op == RET) || (p->op == LRET) )
	while ( p != pf )
	{
		if( islabel(p) || (p->op == CALL) || (p->op == LCALL))
			break ;
		if(( p->op == ADD) && iscon(p->op1) && !strcmp(p->op2,"%sp"))
		{
			DELNODE(p) ;
			killed++ ;
			break ;
		}
		if( (p->op == INC) && !strcmp(p->op1,"%sp") )
		{
			if((p->back->op == INC) && !strcmp(p->back->op1,"%sp"))
				DELNODE(p->back) ;
			DELNODE(p);
			killed++;
			break;
		}
		p = p->back;
	}

	/*
	 *	look for lds of same value as previous
	 */

	p = pf ;
	while ( p != pl )
	{
		if (p->op == LDS || p->op == LES){
			killed += search( p ,  pl , p->op ) ;
			killed += search1( p ,  pl , p->op ) ;
		}
		p = p->forw ;
	}

	/*
	 *	look for redundant mov reg,segreg
	 */

	p = pf ;
	while ( p != pl )
	{
		if((p->op==MOV) && dirref(p->op1) && (dirref(p->op2) & (ES|DS)))
			killed += search2 ( p ,  pl ) ;
		p = p->forw ;
	}

	/*
	** look for mov of %ss into ES or DS and kill it if in block use
	*/
	p = pf ;
	while ( p != pl )
	{
		unsigned int sr;
		if( ( sr = movpat(p) ) && !strcmp("%ss",p->op1 ))
			killed += search3 ( p ,  pl ,sr) ;
		p = p->forw ;
	}

	/*
	** look for mov segadd,reg
	**	    mov reg,segreg
	** followed similar, with segadd same and segreg different
	** and save segreg load if possible
	*/
	p = pf ;
	while ( p != pl )
	{
		unsigned int sr;
		if(p->forw == pl) break;
		if( (sr = movpat(p)) && contype(p->op1) == SEG )
			killed += search4 ( p ,  pl ,sr) ;
		p = p->forw ;
	}
	/*
	** find assembler produced by dereferencing a global
	** pointer to a structure
	** p->a = p->b;
	*/
	p = pf ;
	while ( p != pl && numinst(p,pl,6) )
	{
		unsigned short inst;
		NODE *pp;
		if( (inst = globpt(p)) && sets(p->forw)&((inst == LDS)?DS:ES) )
			killed += search5 ( p ,  pl ,inst) ;
		p = p->forw ;
	}
	return ( killed ) ;
}

/*
** search for lds or les
*/

search( f,l,inst )
NODE *f, *l ;
unsigned short inst;
{
	NODE	*p ;
	int killed = 0 ;
	unsigned fusessets ;

	fusessets = uses ( f ) | sets ( f ) ;

	p = f->forw ;

	while ( p != l )
	{
		if ( p->op == inst )
		{
			if (strcmp(p->op1,f->op1) == 0 &&
			    strcmp(p->op2,f->op2) == 0 &&
			    (sets(f) & uses(p)) == 0)
			{
				DELNODE( p ) ;
				nmsloads++ ;
				killed++ ;
			}
			else
				return( killed ) ;
		}
		else if ( sideeffect(p) || ( sets(p) & fusessets ) )
		{
			return( killed ) ;
		}

		p = p->forw ;
	}

	return( killed ) ;
}

/*
** search for lds / les pairs
*/

search1( f,l ,inst )
NODE *f, *l ;
unsigned short inst;
{
	NODE	*p ;
	int killed = 0 ;
	unsigned fusessets ;
	unsigned reg;
	char *regstr,*regstr1;
	unsigned short insti ;

	if (inst == LDS)
	{
		reg = ES ;
		regstr = "%ds" ;
		regstr1 = "%es:" ;
		insti = LES ;
	}
	else
	{
		reg = DS ;
		regstr = "%es" ;
		regstr1 = "%ds:" ;
		insti = LDS ;
	}

	fusessets = uses ( f ) | sets ( f ) ;
	if(uses(f) & sets(f) & (ES|DS| INDX) ) return(killed);

	p = f->forw ;

	while ( p != l )
	{
		if ( p->op == insti )
		{
			if( !strcmp(p->op1,f->op1) && !strcmp(p->op2,f->op2))	
			{
				NODE *pl;
				pl = p->forw;
				do {
					if( (sets(pl) & fusessets) || sinst(p->op) ) return(killed);
					if(sets(pl) & reg || !(pl->nlive & reg)){
						DELNODE( p ) ; nmsloads++ ; killed++ ;
						do {
						p=p->forw;
						if(!strncmp(p->op1,regstr1,4)) strncpy(p->op1,regstr,3);
						else if(!strncmp(p->op2,regstr1,4)) strncpy(p->op2,regstr,3);

						} while(p != pl);
						return(killed);
					}
					pl = pl->forw;
				} while(pl != l);
			return(killed);
			}
		}
		else if ( sideeffect(p) || ( sets(p) & fusessets ) )
		{
			return( killed ) ;
		}

		p = p->forw ;
	}

	return( killed ) ;
}

/*
** search for mov into segment 
*/

search2( f,l )
NODE *f, *l ;
{
	NODE	*p ;
	int killed = 0 ;
	unsigned fusessets ;

	fusessets = uses ( f ) | sets ( f ) ;

	p = f->forw ;

	while ( p != l )
	{
		if( (p->op==MOV) && !strcmp(p->op1,f->op1) && 
		    !strcmp(p->op2,f->op2) )
		{
			DELNODE ( p ) ;
			nmsloads++ ;
			killed++ ;
		}
		else if ( (sets(p) & fusessets) )
			return (killed) ;

		p = p->forw ;
	}

	return( killed ) ;
}
/*	have found
		mov %ss,reg
		mov reg,%es/%ds
	check that use of es/ds ends in this block
	if so,can use ss and scrap es/ds load
	zapinst used to kill node fully for ldanal
*/
search3( f,l,reg1 )
NODE *f, *l ;
unsigned int reg1;
{
	NODE	*p ;
	int killed = 0 ;
	int tag = 0;
	char *regstr;

	p = f->forw;
	regstr = (reg1 == ES)?"%es:":"%ds:";

	do {
		p = p->forw;
		if(sinst(p->op)) return(killed); /* string instruction */
		if ( (sets(p) & reg1) || !(p->nlive & reg1) ) {
			tag = 1;
			break;
		}
	}while( p != l);
	{
	NODE *pl;
	pl = f->forw;
	do{/* use ss instead of segreg in block regardless of tag */
		pl = pl->forw;
		if( !strncmp(regstr,pl->op1,3) ) strncpy(pl->op1,"%ss",3);
		if(pl->op2 != NULL && !strncmp(regstr,pl->op2,4) ) strncpy(pl->op2,"%ss",3);/* right op can only have segreg as overide */
	}while( pl != p);
	}
	if(tag) { /* remove load only if dead or reloaded in block */
		zapinst(f->forw,1); /* remove mov reg,%es/%ds only .. reg may be needed , else found dead */
		nmsloads++;
		killed++;
	}

	return( killed ) ;
}
/*
** have found mov of segaddr to segreg1
** look for mov of same segaddr to segreg2
** if segreg1 is not changed before segreg2
** can save segreg2 load by using segreg1
** zapinst used to kill node fully for ldanal
*/
search4( f,l ,reg1 )
NODE *f, *l ;
unsigned reg1;
{
	NODE	*p ;
	int killed = 0 ;
	unsigned reg;
	char *regstr,*regstr1;

	reg = (reg1 == DS)?ES:DS;
	regstr = (reg1 == DS)?"%ds":"%es";
	regstr1	= (reg1 == ES)?"%ds:":"%es:";

	p = f->forw->forw ;

	while ( p != l )
	{
		if ( p->forw == l) return(killed);
		if ( movpat(p) == reg)
		{
			if( p->op1 && f->op1 && !strcmp(p->op1,f->op1) )
			{
				NODE *pl;
				pl = p->forw;
				do {
					pl = pl->forw;
					if((sets(pl) & reg1) || sinst(pl->op)) return(killed);
					if(sets(pl) & reg || !(pl->nlive & reg)){
						zapinst( p->forw ,1) ;
						do {
						p = p->forw;	
						if(!strncmp(p->op1,regstr1,4)) strncpy(p->op1,regstr,3);
						else if(p->op2 && !strncmp(p->op2,regstr1,4)) strncpy(p->op2,regstr,3);

						} while(p != pl);
						nmsloads++ ; 
						killed++;
						return(killed);
					}
				} while(pl != l);
			return(killed);
			}
		}
		else if ( ( sets(p) & reg1 ) )
		{
			return( killed ) ;
		}

		p = p->forw ;
	}

	return( killed ) ;
}
/* have found (using globpt())
** 	mov segaddr,reg
**	mov reg,%ds
**	lds %ds:off,indxreg
**  or same with es
** can we find same other with seg reg ?
** and save 2 seg loads
*/
search5( f,l ,inst )
NODE *f, *l ;
unsigned short inst;
{
	NODE	*p ;
	int killed = 0 ;
	unsigned ireg,reg,reg1;
	char *regstr,*regstr1;

	ireg = sets(f) ;

	if(inst==LDS)
	{
		reg1 = DS ;
		reg =  ES ;
		regstr = "%ds" ;
		regstr1 = "%es:" ;
	}
	else
	{
		reg1 = ES ;
		reg = DS ;
		regstr = "%es" ;
		regstr1 = "%ds:" ;
	}

	p = f->forw->forw->forw ;

	if ( p == l || p->forw == l)
		return(killed);

	while ( p->forw->forw != l )
	{
		if ( globpt(p) != inst && sets(p->forw )&reg1)
		{
			if( !strcmp(p->op1,f->op1) )
			{
				NODE *pl;
				pl = p->forw->forw;
				do {
					pl = pl->forw;
					if((sets(pl) & reg1) || sinst(pl->op)) return(killed);
					if(sets(pl) & reg || !(pl->nlive & reg)){
						zapinst( p->forw ,2) ;
						do {
						p = p->forw;	
						if(!strncmp(p->op1,regstr1,4)) strncpy(p->op1,regstr,3);
						else if(!strncmp(p->op2,regstr1,4)) strncpy(p->op2,regstr,3);
						} while(p != pl);
						nmsloads++ ; 
						killed++;
						return(killed);
					}
				} while(pl != l);
			return(killed);
			}
		}
		else if ( sideeffect(p) || ( sets(p) & ireg ) )
		{
			return( killed ) ;
		}

		p = p->forw ;
	}

	return( killed ) ;
}

/* ***************************************************************************
** ***************************************************************************
**
**
**			SLITHOLE
**
**
**	optimisations here are :-
**
**	1.) look for some useless instructions (live/dead analysis has been
**	    performed.
**	2.) look for some slightly more efficient way of performing some
**	    instructions.
**	3.) enter where no local data
**	4.) ADC segment selector to register
*/

extern NODE n0;
extern FILE *yyout;
boolean
slithole(p)
register NODE *p;
{
	register int cop;

	cop = p->op;

	wchange();

	switch (cop) {

	case MOV:
	case MOVB:

		/* useless operations on a dead register */

		if (getifreg(p->op2) && isdead(p->op2,p)) 
		{
				zapinst(p, 1);
				nmdeadr++;
				return(true);
		}

		if (iscon(p->op1) && conval(p->op1) == 0 && getifreg(p->op2)) {
			if (p->op == MOV) {
				p->op = CLR;
				p->opcode = "clr";
			} else {
				p->op = CLRB;
				p->opcode = "clrb";
			}
			p->op1 = p->op2;
			p->op2 = NULL;
			nmcop++;
			return(true) ;
		} else if (getifreg(p->op2) && !strcmp(p->op1, p->op2)) {
			zapinst(p, 1);
			nmenop++;
			p = p->back;
			return(true) ;
		}
		break ;

	case CLR:
	case CLRB:

		if (getifreg(p->op1) && isdead(p->op1,p)) {
			zapinst(p, 1);
			nmdeadr++;
			return(true);
		}
		break ;

	case CWD:

		if( isdead("%dx",p) ) 
		{
			zapinst(p, 1);
			nmdeadr++ ;
			return(true);
		}
		break ;


	case ADD:
	case SUB:
	case ADDB:
	case SUBB:

		if (iscon(p->op1) && conval(p->op1) == 0 && 
			!isbr(p->forw) && !islabel(p->forw)) 
		{
			if( (p->forw->op==ADC) || (p->forw->op==SBB))
			{
				if (iscon(p->forw->op1) && 
					conval(p->forw->op1) == 0)
				{
					zapinst(p,2) ;
					nmenop += 2 ;
					return(true);
				}
			}
			else
			{
				zapinst(p, 1);
				nmenop++;
				return(true);
			}
		}

		if (iscon(p->op1) && conval(p->op1) == 1 &&
			!isbr(p->forw) && !islabel(p->forw) &&
			!( (p->forw->op==ADC) || (p->forw->op==SBB)) )
		{
			switch(cop){
				case ADD:	p->op = INC ;
						p->opcode = "inc" ;
					break ;
				case ADDB:	p->op = INCB;
						p->opcode = "incb" ;
					break ;
				case SUB:	p->op = DEC ;
						p->opcode = "dec" ;
					break ;
				case SUBB:	p->op = DECB;
						p->opcode = "decb" ;
					break ;
			}
			p->op1 = p->op2 ;
			p->op2 = NULL ;
			return true ;
		}

		if (iscon(p->op1) && conval(p->op1) == 2 && 
			((cop==ADD) || (cop==SUB)) && getifreg(p->op2) &&
			!isbr(p->forw) && !islabel(p->forw) &&
			!( (p->forw->op==ADC) || (p->forw->op==SBB)) )
		{
			generate(p,(cop==ADD?"inc":"dec"),p->op2,(char *) NULL);
			generate(p,(cop==ADD?"inc":"dec"),p->op2,(char *) NULL);
			zapinst(p,1);
			return true ;
		}

		break;

	case AND:
		if (iscon(p->op1) && (conval(p->op1)&0xffffl) == 0xffffl) {
			zapinst(p, 1);
			nmenop++;
			p = p->back;
			return(true);
		} else if (iscon(p->op1) && (conval(p->op1)& 0xffffl) == 0xff00l &&
			   (getifreg(p->op2) & AR) != 0) {
			p->op = CLRB;
			p->opcode = "clrb";
			p->op1 = (char *) getspace(4);
			p->op1[0] = '%';
			p->op1[1] = p->op2[1];
			p->op1[2] = 'l';
			p->op1[3] = '\0';
			p->op2 = NULL;
			return(true);
		} else if (iscon(p->op1) && conval(p->op1) == 0xffl &&
			   (getifreg(p->op2) & AR) != 0) {
			p->op = CLRB;
			p->opcode = "clrb";
			p->op1 = (char *) getspace(4);
			p->op1[0] = '%';
			p->op1[1] = p->op2[1];
			p->op1[2] = 'h';
			p->op1[3] = '\0';
			p->op2 = NULL;
			return(true);
		}
		break;

	case ANDB:

	 	if (iscon(p->op1) && conval(p->op1) == 0xffl) {
			zapinst(p, 1);
			nmenop++;
			p = p->back;
			return(true);
		}
		break;

	case OR:
	case ORB:

		if (iscon(p->op1) && conval(p->op1) == 0) {
			zapinst(p, 1);
			nmenop++;
			p = p->back;
			return(true);
		}
		break;

	case JCXZ:
		if ((p->back->op == MOV || p->back->op == MOVB) &&
		    iscon(p->back->op1) && conval(p->back->op1) != 0 &&
		    (!strcmp(p->back->op2, "%cx") || !strcmp(p->back->op2, "%cl"))) {
			zapinst(p, 1);
			nmenop++;
			p = p->back;
			return(true);
		}
		break;

	case ENTER:

		/*
		 *	push %bp	1 byte	3 beats
		 *	mov sp , bp	2 bytes 2 beats	=	3 bytes 5 beats
		 *
		 *	enter $0 , $0			=	4 bytes 11 beats
		 */
	
		if (numauto == 0) {
	
			generate ( p , "mov" , "%sp" , "%bp" ) ;
			generate ( p , "push" , "%bp" , (char *) NULL ) ;
			zapinst ( p , 1 ) ;
			nspinc++ ;
			return(true) ;
		}
		break ;


	case TEST:

		/*
		 *	test	mem-access
		 *	( equiv. test $0xffff,mem-access )
		 *	shorter to :-
		 *	cmp	$0,mem-access
		 */

		if ( !( (p->op2) || (p->op1[0]=='%') || p->op1[0] == '$'))
		{
			p->op = CMP ;
			p->opcode = "cmp" ;
			p->op2 = p->op1 ;
			p->op1 = "$0" ;
			nmcop++ ;
			return(true) ;
		}
		break ;
	case IMUL3A:
	case IMUL3B:
	case IMUL3C:
	case IMUL3D:
	case IMUL3S: case IMUL3DI:
		if(iscon(p->op1) && conval(p->op1) == 0) {
			p->opcode = "mov";
			p->op = MOV;
			if(p->op == IMUL3A)
				  p->op2 = "%ax";
			else if(p->op == IMUL3B)
				  p->op2 = "%bx";
			else if(p->op == IMUL3C)
				  p->op2 = "%cx";
			else if(p->op == IMUL3S)
				  p->op2 = "%si";
			else if(p->op == IMUL3DI)
				  p->op2 = "%di";
			else p->op2 = "%dx";
			nmcop++;
			return(true);
		}


	default:
		break ;

	}
	return(false);
}


/* ***************************************************************************
** ***************************************************************************
**
**
**
**			BLACKHOLE
**
*/

char *
lowestaddr ( ) ;

boolean
blackhole(pf,pl)
register NODE *pf, *pl; 
{
	register int cop1, cop2;
	int factor, mult;
	unsigned r1, r2 ;
	char *t,**tt;
	NODE *tmp;

	/*
	 * TRACING
	 */

	wchange() ;

	cop1 = pf->op ;
	cop2 = pl->op ;

	/*
	 * Look for test on a constant - change or remove jump 
	 */
	if(cop1 == TEST && iscon(pf->op1) && !pf->op2 ) {
		unsigned con;
		con = conval(pf->op1);
		if( (con && cop2 == JNE) || (!con && cop2 == JE) ) {
			pl->op = JMP;
			pl->opcode = "jmp";
			nmcontst++;
			return(true);
		}
		else if( (con && cop2 == JE ) || (!con && cop2 == JNE) ) {
			zapinst(pl, 1);
			nmcontst++;
			return(true);
		}
	}

	/*
	 * Multiply by 0 or 1 - put constant into %ax to save instruction 
	 */

	if (cop1 == MOV && cop2 == IMUL && iscon(pf->op1))
	{
		mult = conval(pf->op1);
		if (mult == 0 || mult == 1)
		{
			if (mult) pf->op1 = pl->op1;
			zapinst(pl, 1);
			nmcm++;
			return(true);
		}
	}


	/* 
	 *	x -> reg -> x
	 *    or  
	 *	reg -> x -> reg
	 */

	if (cop1 == MOV && cop2 == MOV && !strcmp(pf->op1,pl->op2)
	    && !strcmp(pf->op2,pl->op1)) 
	{
		if (getifreg(pf->op1))
		{
			zapinst(pl, 1);
			nmlas++;
		}
		else
		{
			zapinst(pf, 2);
			nmlsp += 2;
		}
		return(true);
	}

	/*
	 *	reg1 -> x -> reg2
	 */

	if (cop1 == MOV && cop2 == MOV && getifreg(pf->op1) && getifreg(pl->op2)
	    && !getifreg(pf->op2) && !strcmp(pf->op2,pl->op1)) 
	{
		pl->op1 = pf->op1 ;
		return(true);
	}

	/*
	 * mov[b] %r,mem; {test[b] mem | cmp[b] -,mem} 
	 */

	if (((cop1 == MOV && (cop2 == TEST || cop2 == CMP)) ||
	     (cop1 == MOVB && (cop2 == TESTB || cop2 == CMPB))) &&
	     getifreg(pf->op1) && !getifreg(pf->op2))
	{
		factor = (cop2 == TEST || cop2 == TESTB);
		if (!strcmp(pf->op2, factor ?  pl->op1 : pl->op2))
		{
			if (factor)
				pl->op1 = pf->op1;
			else
				pl->op2 = pf->op1;
			nmcop++;
			return(true);
		}
	}

	/*
	 *	 clrb %rh
	 *	 and $(1-255),%r 
	 *	->
	 *	 and $(1-255),%r
	 */

	if (cop1 == CLRB && getifreg(pf->op1) && pf->op1[2] == 'h' &&
	    cop2 == AND && iscon(pl->op1) && conval(pl->op1) <= 255l &&
	    getifreg(pl->op2) && pf->op1[1] == pl->op2[1])
	{
		zapinst(pf, 1);
		nmenop++;
		return(true);
	}

	/*
	 *	look for:-
	 *
	 *	mov	something , non-seg-register
	 *	mov	something+2 , segment-register
	 *
	 *	or vice versa
	 *
	 *	and replace with les or lds
	 */

	if ( (cop1==MOV) && (cop2==MOV) &&
	     ( ( dirref(pl->op2) & (ES|DS)) ||( dirref(pf->op2) & (ES|DS)))
	    && (t = lowestaddr(pl->op1,pf->op1,dirref(pl->op2) & (ES|DS))) )
	{
		if( ( r1 = (dirref(pf->op2)) & (ES|DS)) )
			pf->op2 = pl->op2 ;
		else
			r1 = dirref(pl->op2) ;

		if (r1 & ES) 
		{
			pf->op = LES ;
			pf->opcode = "les" ;
		}
		else
		{
			pf->op = LDS ;
			pf->opcode = "lds" ;
		}

		pf->op1 = t ;

		zapinst(pl,1);
		nmldes++;

		return(true);
	}

	/*
	 *	add	$constant , index-register
	 *	op	[2](index-register)[,something]     or vice versa
	 *
	 */

	if ( (cop1==ADD) && (iscon(pf->op1)) && ((r1=sets(pf)) & (SI|DI|BX))
		&& !( (pl->op==TEST) || (pl->op==CMP)))
	{
		if(  ( (tt= &pl->op1) && (addruse(*tt) & r1))
			||( (tt= &pl->op2) && (addruse(*tt) & r1)))
		{
			addtogether(tt,pf->op1) ;
			if(isdead(pf->op2,pl)||(sets(pl) & r1))
			{
				zapinst(pf,1);
				nminds++;
				return(true);
			}
			else
			{
				DELNODE(pf);
				APPNODE(pf,pl);
				nodeswap++;
				return(true);
			}
		}

		/*
		 * can't do that but can we move add down an instruction
		 */

		if( !( ((uses(pl) | sets(pl)) & (BX|SI|DI) ) || (pl->op==ADC)||
			(isbr(pl)) || (pl->op >= LOOP && pl->op <= REPZ) ))
		{
			/* yes */
			DELNODE( pf ) ;
			APPNODE( pf , pl ) ;
			nodeswap++;
			return( true ) ;
		}
	}

	/*
	 *	mov	something , register
	 *	push	register
	 */

	if( (cop2==PUSH) && (r1=dirref(pl->op1)) &&
		(cop1==MOV) && (sets(pf) & r1) && (isdead(pf->op2,pl) ) )
	{
		pl->op1 = pf->op1 ;
		zapinst(pf,1);
		nmloads++ ;
		return ( true ) ;
	}

	/*
	 *	mov	variable , non-seg reg
	 *	mov	same non-seg reg , seg reg
	 *
	 *	where 	non-seg reg is now dead
	 */

	if ( (cop1==MOV) && (r1=regnum(pf->op2)) && 
	     !(iscon(pf->op1) || regnum(pf->op1))
	    &&  (cop2==MOV) && (r1 == regnum(pl->op1))
		    && (dirref(pl->op2)&(ES|DS)) && (isdead(pl->op1,pl)))
	{
		pl->op1 = pf->op1 ;
		zapinst(pf,1);
		nmloads++;
		return(true);
	}

	/*
	 *	intervening instruction
	 *	mov	same non-seg reg , seg reg
	 *
	 *	where 	non-seg reg is now dead
	 *
	 *	just swap last two instructions peephole will
	 *	take care of the rest
	 */

	if ( (cop2==MOV) && (sets(pl) & (ES|DS)) && !( sets(pf) & uses(pl) )
	     &&(dirref(pl->op1)) && !( uses(pf) & sets(pl)) 
		&& !(uses(pf) & uses(pl) )
		&& !( sets(pf) & (ES|DS) && cop1 == MOV) ) /* don't loop! */
	{
		DELNODE( pf ) ;
		APPNODE( pf , pl ) ;
		nodeswap++;
		return ( true ) ;
	}




	return( false ) ;

}

/* ***************************************************************************
** ***************************************************************************
**
**
**
**			PEEPHOLE
**
**	c--; where c is char loads %ax uselessly 
**	Multiply by 0 or 1 - put constant into %ax to save instruction 
**	multiplication strength reduction 
**	if not power of 2 or %cl not free, then try to replace
**	x -> reg -> x    or    reg -> x -> reg 
**	Add of 1 turn into inc if not being done as part of long add
**	Sub of 1 turn into dec if not being done as part of long sub
**	Constant_or_reg -> scratch_reg -> reg_or_memory 
**	Can use lea instruction 
**	Needless use of %ax for computation 
**	Look for (after other changes) MOV mem,%ax; INC(DEC) mem; MOV %ax,reg 
**	INC(DEC), MOV to reg, DEC(INC)  
**	For a=b=c=5; compiler gens mov $5,a; mov $5,%ax; mov %ax,b; mov %ax,c.
**	mov[b] %r,mem; {test[b] mem | cmp[b] -,mem} 
**	clrb %rh; and $(1-255),%r -> and $(1-255),%r 
**	xor -,%r; [clrb %rh]; xor %r,- 
**	 3 identical single-bit shifts   
**	 movb $n,%cl;  s/r %cl,%r;  s/r %r ->  movb $n+1,%cl;  s/r %cl,%r;  
**	mov variable into non-seg reg followed by mov reg into seg reg
**	lds/les optimisations
*/

boolean
peephole(pf,pl) register NODE *pf, *pl; 
{
	register int cop1, cop2, cop3;
	int factor, mult;
	unsigned r1,r2,r3 ;
	char *t,**tt;
	NODE *pm, *tmp;

	pm = pf->forw;
	cop1 = pf->op;
	cop2 = pm->op;
	cop3 = pl->op;
	wchange();

	/*
	 * Constant_or_reg -> scratch_reg -> reg_or_memory 
	 */

	if (cop1 == MOV && cop2 == MOV && (factor = getifreg(pf->op2))
		&& factor == getifreg(pm->op1) && isdead(pm->op1,pm)
		&& ((getifreg(pf->op1) & GR) || (getifreg(pm->op2) & GR))) {

		pf->op2 = pm->op2;
		pf->nlive |= pm->nlive;
		zapinst(pm, 1);
		nmmpc1++;
		return(true);
	}

	/*
	 * multiplication strength reduction 
	 */

	if (cop1 == MOV && cop2 == IMUL && !strcmp(pf->op2, "%ax")) { 

		if ((mult = ispow2(pf->op1)) > 0 && isdead("%cx", pf)) {
			pf->op = MOVB;
			pf->opcode = "movb";
			pf->op1 = (char *)getspace(4);
			sprintf(pf->op1,"$%d",mult);
			pf->op2 = "%cl";
			pf->nlive |= CL;
			pm->op = SHL;
			pm->opcode = "shl";
			if (!strcmp(pm->op1, pl->op2)) {
				pm->op1 = "%cl";
				pm->op2 = pl->op2;
				zapinst(pl, 1);
				nmcm++;
				return(true);
			}
			else if (mflag) {
				generate(pf, "mov", pm->op1, pm->op2);
				pm->op1 = "%cl";
				ndisc--;
				nmcm++;
				return(true);
			}
		}

		/* if not power of 2 or %cl not free, then try to replace
		 * with adds and shifts if constant is small */

		if (mflag) {
			if ((mult=conval(pf->op1)) > -1l && mult <= mvalue) {
				asmult(pf, (int) mult);
				nmcm++;
				return(true);
			}
		}
	}


	/*
	 * Needless use of %ax for computation 
	 */

	factor = (cop2 == SHL || cop2 == INC || cop2 == DEC || cop2 == SAR) ?
		1 : ((cop2 == ADD || cop2 == SUB) ? 2 : 0);
	if (factor && cop1 == MOV && cop3 == MOV && !strcmp(pf->op2,pl->op1)
		&& isdead(pl->op1,pl) && !strcmp(pf->op2,factor==1?pm->op1:pm->op2)
		&& (factor == 1 || iscon(pm->op1) || 
			(getifreg(pm->op1) && !strcmp(pm->op1,pm->op2))) ) {

		factor--;
		if (!strcmp(pf->op1,pl->op2) ) {
			if (factor) pm->op2 = pl->op2;
			else pm->op1 = pl->op2;
			zapinst(pf, 1);
			zapinst(pl, 1);
			nmnuax += 2;
			return(true);
		}
		else if (getifreg(pf->op1) || getifreg(pl->op2) ) {

			pf->op2 = pl->op2;
			if (factor) pm->op2 = pl->op2;
			else pm->op1 = pl->op2;
			zapinst(pl, 1);
			nmnuax++;
			return(true);
		} ;
	}

	/*
	 * For a=b=c=5; compiler gens mov $5,a; mov $5,%ax; mov %ax,b; mov %ax,c
 	 * While it may look dumb, bytes and cycles are saved by:
 	 * mov $5,%ax; mov %ax,a; mov %ax,b; mov %ax,c;
 	 */

	if (cop1 == MOV && cop2 == MOV && iscon(pf->op1)
		&& !strcmp(pf->op1,pm->op1) && getifreg(pm->op2) == AX) {

		pm->op1 = pm->op2;
		pm->op2 = pf->op2;
		if (conval(pf->op1))
			pf->op2 = pm->op1;
		else {
			pf->op1 = pm->op1;
			pf->op2 = NULL;
			pf->op = CLR;
			pf->opcode = "clr";
		}
		pf->nlive |= AX;
		return(true);
	}

	/*
	 *	xor	-,%r
	 *	[clrb	%rh]
	 *	xor	%r,-
	 */

	if (cop1 == XOR && !getifreg(pf->op1) && getifreg(pf->op2) &&
	    (cop2 == CLRB || cop2 == XOR)) {

		tmp = (cop2 == XOR) ? pm : pl;
		if (!strcmp(pf->op1, tmp->op2) && !strcmp(pf->op2, tmp->op1)) {
			zapinst(pf, cop2 == CLRB ? 2 : 1);
			if (cop2 == CLRB) {
				pl->op = MOVB;
				pl->opcode = "movb";
				t = (char *) getspace(4);
				t[0] = '%';
				t[1] = pl->op1[1];
				t[2] = 'l';
				t[3] = '\0';
				pl->op1 = t;
				nmenop += 2;
			} else {
				pm->op = MOV;
				pm->opcode = "mov";
				nmenop++;
			}
			return(true);
		}
	}

	/*
	 *	look for:-
	 *
	 *	mov	something , non-seg-register
	 *	other instruction (esp add something,non-seg-register)
	 *	mov	something+2 , segment-register
	 *
	 *	or vice versa
	 *
	 *	and replace with les or lds
	 *	but 2nd instruction must not use seg-reg
	 */

	if ( (cop1==MOV) && (cop3==MOV) &&
	     !( uses(pm) & (ES|DS) & dirref(pl->op2) ) &&
	     ( ( dirref(pl->op2) & (ES|DS)) ||( dirref(pf->op2) & (ES|DS)))
	    && (t = lowestaddr(pl->op1,pf->op1,dirref(pl->op2) & (ES|DS))) )
	{
		if((r1=(dirref(pf->op2)) & (ES|DS)) )
			pf->op2 = pl->op2 ;
		else
			r1 = dirref(pl->op2) ;

		if (r1 & ES) 
		{
			pf->op = LES ;
			pf->opcode = "les" ;
		}
		else
		{
			pf->op = LDS ;
			pf->opcode = "lds" ;
		}

		pf->op1 = t ;

		zapinst(pl,1);
		nmldes++;

		return(true);
	}



	/*
	 *	mov	something , register
	 *	something else
	 *	push	register
	 */

	if( (cop3==PUSH) && (r1=dirref(pl->op1)) &&
		(cop1==MOV) && (sets(pf) & r1) && (isdead(pf->op2,pl) 
	    && !((uses(pm)|sets(pm)) & r1 )) )
	{
		pl->op1 = pf->op1 ;
		zapinst(pf,1);
		nmloads++ ;
		return ( true ) ;
	}


	return(false);
}


/* ***************************************************************************
** ***************************************************************************
**
**
**			POTHOLE
**
**	First go :-
**
**	Attempt to optimise redundant loads of longs - this happens a lot in
**	large model so is well worth looking for (I hope)
**
**	Second go :-
**
**	mov	variable , non-seg reg
**	[intervening instruction]
**	[intervening instruction]
**	mov	same non-seg reg , seg reg
**
**	where 	non-seg reg is now dead
**	
**	Third go :-
**
**		unary minus on long
**
**		not	a
**		not	b
**		add	$1,a
**		adc	$0,b
**
**	to --
**		neg	b
**		neg	a
**		sbb	$0,b
**
*/

boolean
pothole( pf , pl )
NODE *pf ,*pl;
{
	NODE	*p1 , *p2 , *p3 , *p4 ;
	int	i ;
	unsigned reg1 , reg2 , reg3 , reg4 ;

	wchange() ;

	p1 = pf ;
	p2 = p1->forw ;
	p3 = p2->forw ;
	p4 = p3->forw ;

	/*
	 *	sequence :-
	 *
	 *	mov	reg1 , somewhere1
	 *	mov	reg2 , somewhere2
	 *	mov	somewhere1 , reg1
	 *	mov	somewhere2 , reg2
	 *
	 *	or its ilk
	 */

	if ((p1->op==MOV) && (p2->op==MOV) && (p3->op==MOV) && (p4->op==MOV)
	    && (reg1 = regnum(p1->op1)) && (reg2 = regnum(p2->op1))
	    && ( reg3 = regnum(p3->op2)) && (reg4 = regnum(p4->op2)) )
	{
		/*
		 *	so are of form :-
		 *
		 *	mov	reg1 , somewhere1
		 *	mov	reg2 , somewhere2
		 *	mov	somewhere3 , reg3
		 *	mov	somewhere4 , reg4
		 *
		 */
	
		if (  ( (reg1==reg3) && (reg2==reg4) ) &&
		     (!strcmp(p1->op2,p3->op1) && !strcmp(p2->op2,p4->op1)) &&
			strcmp(p1->op2,p2->op2) )
		{
			zapinst(p3 , 2 ) ;
			nmlloads++ ;
			return(true);
		}
		else
		if (  ( (reg1==reg4) && (reg2==reg3) ) &&
		     (!strcmp(p1->op2,p4->op1) && !strcmp(p2->op2,p3->op1)) &&
			strcmp(p1->op2,p2->op2) )
		{
			zapinst(p3 , 2 ) ;
			nmlloads++ ;
			return(true);
		}
	
	}



	/*
	 *	mov	something , register
	 *	something else
	 *	something else
	 *	push	register
	 */

	if( (p4->op==PUSH) && (reg1=dirref(p4->op1)) &&
		(p1->op==MOV) && (sets(p1) & reg1) && (isdead(p1->op2,p4) 
	    && !((uses(p2)|sets(p2)|uses(p3)|sets(p3)) & reg1 )) )
	{
		if(p2->op != PUSH && p3->op != PUSH)
		{
			p1->op = PUSH;
			p1->opcode = "push";
			p1->op2 = p4->op2;
			zapinst(p4,1);
			nmloads++;
			return(true);
		}
		else if(!(uses(p1)|sets(p2)|sets(p3)) ) {
			p4->op1 = p1->op1 ;
			zapinst(p1,1);
			nmloads++ ;
			return ( true ) ;
		}
	}

	/* everything failed */
	return false ;
}
/* movpat
** match pattern of form:
**			mov thing,reg
**			 mov reg,segreg
** and return segreg
*/
unsigned 
movpat(p)
NODE *p;
{
	NODE *n;
	unsigned ret;
	n = p->forw;
	if( p->op == MOV && n->op == MOV &&
		!strcmp(p->op2,n->op1) && dirref(p->op2) &&
		(ret = sets(n) & (ES|DS)) ) return(ret);
	return(0);
}
/*
** globpat
** look for
** 	mov segaddr,reg
	mov reg,%ds
	lds/les %ds:offaddr,indxreg
** ie (global pointer to struct p) p->a
*/

unsigned short
globpt(p)
NODE *p;
{
	NODE *pff;
	if(  movpat(p)
		&& ( (pff = p->forw->forw)->op == LDS || pff->op == LES)
		&& strlen(pff->op1) >4
		&& !strcmp(p->op1+4,pff->op1+4)
		&& !strncmp(p->forw->op2,pff->op1,3) )
		return(pff->op);
	return(0);
}
/*
** are there another n instructions in this block?
*/
numinst(p,l,n)
NODE *p,*l;
int n;
{
	int i;
	for(i = 0;i<n;i++,p = p->forw) 
		if(p == l) return(0);
	return(1);
}
