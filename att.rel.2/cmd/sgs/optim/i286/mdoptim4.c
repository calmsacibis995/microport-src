/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   %W% - %E% */

/* **********************************************************************
** **********************************************************************
**
**	mdoptim4.c
**
**	This module contains all the code associated with a register
**	tracking routine.
**	It is freely adapted from the live/dead analysis routine used
**	by the portable code improver
**
*/

#include "optim.h"

#define NEWBLK(n, type)	((type *) xalloc((n) * sizeof(type)))
#define ALLB(b,s)	b = (s) ; b != NULL ; b = b->next

#define REGCANTRACK 0x63ff

/* block of text */

typedef struct block {
	struct block *next;	/* pointer to textually next block */
	struct block *nextl;	/* pointer to next executed block if no br */
	struct block *nextr;	/* pointer to next executed block if br */
	struct block *ltest;	/* for loop termination tests */
	NODE *firstn;		/* first text node of block */
	NODE *lastn;		/* last text node of block */
	short index;		/* block index for debugging purposes */
	short length;		/* number of instructions in block */
	short indeg;		/* number of text references */
	short marked;		/* marker for various things */
} BLOCK;

extern BLOCK b0;		/* header for block list */
extern int idx;			/* block index (for debugging) */

extern int nmsh ;
extern int nmenop ;
extern int nmsloads ;
extern int nmloads ;

typedef struct regtrace
	{
		unsigned	regok ;
		char 		*regis[14] ;
	} REGTRACE ;


/* rtanal -- perform register tracking analysis over flow graph
**
**
*/

	void
rtanal()
{ 
    register BLOCK * b;			/* pointer to current block */
    register NODE * p;			/* pointer to current inst. node */
    NODE *tmpf , *tmpl ;
    struct rtinfo			/* temporary block-level structure */
    {
	REGTRACE	rtin ;
	REGTRACE	rttemp ;
	REGTRACE	rtout ;
    };
    struct rtinfo * rtdata;		/* array of data for each block */
    register struct rtinfo * rtptr;	/* pointer to one of the above */
    unsigned i;
    boolean changed;

    bldgr(false); /* update block structure but don't call bboptim */

	/*
	 *	need to check don't attempt to allocate more than a segment
	 */

	if( ( (long)(idx+1) * (long)sizeof(struct rtinfo) ) >= 0x10000L )
		fatal( "Out of space\n",(char *)NULL);

    rtdata = NEWBLK(idx + 1, struct rtinfo);


/* Initialize:  set the recently allocated array to zero.  The idea, here,
** is that each entry in the array corresponds to one block in the flow
** graph.  We assume that blocks have sequential index numbers and that
** idx is the last index number.
*/

    clear(rtdata, (idx + 1) * sizeof(struct rtinfo));


/*
** Now go through all the basic blocks performing register analysis
** First time through there will be no information available as to entry
** conditions - but subsequent entries should have this.
** Continue process until no changed
**
*/

	do
	{
		changed = 0 ;

		/*
		 * perform register tracking for all basic blocks
		 */

		for( ALLB(b,b0.next) )
		{
			rtdata[b->index].rttemp = rtdata[b->index].rtin ;
			rtdata[b->index].rtout = rtdata[b->index].rtin ;
			clear(&rtdata[b->index].rtin, sizeof(REGTRACE));
			rtdata[b->index].rtin.regok = REGCANTRACK ;
			if( b->length)
			{
				int omit ;
				tmpf = b->firstn->back ;
				tmpl = b->lastn->forw ;
				omit = rtbboptim( b->firstn , b->lastn ,
						&rtdata[b->index].rtout) ;
				if(omit>b->length)
					omit = b->length ;
				b->length -= omit ;
				ndisc += omit ;
				b->firstn = tmpf->forw ;
				b->lastn = tmpl->back ;
				if( tmpf->forw == tmpl )
					b->length = 0 ;
			}
		}

		/*
		 * propogate into all basic block start information
		 */

		for( ALLB(b,b0.next) )
		{
			if( b->nextl != NULL )
				rtchange ( &rtdata[b->index].rtout ,
					&rtdata[b->nextl->index].rtin ) ;
			if( b->nextr != NULL )
				rtchange ( &rtdata[b->index].rtout ,
					&rtdata[b->nextr->index].rtin ) ;
		}

		/*
		 * look for any changes
		 */

		for( ALLB(b,b0.next) )
			if( rtdata[b->index].rtin.regok ^ rtdata[b->index].rttemp.regok )
			{
				changed++ ;
				break ;
			}

	} while (changed) ;

/*
** tidy up the space used by this optimisation stage
**
*/

    xfree((char *) rtdata);		/* free up temp. storage */
}


/*
** rtchange
**
** propogate changed into another node
**
*/

rtchange( out , in )
REGTRACE *in , *out ;
{
	int i,change = 0 ;
	unsigned thisreg , thisregnum ;
	static char *regs[] = { "%ax" , "%bx" , "%cx" , "%dx" ,
				"%si" , "%di" , "%ds" , "%es" } ;

	for(i=0;i<8;i++)
	{
		thisreg = getreg ( &regs[i][1] ) ;
		thisregnum = regnum ( regs[i] ) ;
		if (   ( thisreg & in->regok & out->regok)
		    && ( ( (in->regis[thisregnum] == NULL) ||
		     		(out->regis[thisregnum] == NULL) )
			|| ( !strcmp( in->regis[thisregnum] , 
				      out->regis[thisregnum] ) ) ) )
		{
			in->regis[thisregnum] = out->regis[thisregnum] ;
			change++ ;
		}
		else
			in->regok &= ~thisreg ;
	}
	return change ;
}



/* 
**
**
**
**			RTBBOPTIM
**
**	First cut at some register tracking .
**	Main idea here is to save segment register reloads anything else
**	is a bonus.
**
**	rtbboptim gets called before live/dead register analysis has been
**	performed and with pf pointing to the first instruction and pl
**	pointing to the last of a 'basic block' of code  - that is one
**	which is processed serially i.e. there are no branches out or
**	into it.
**
**	Strategy is to trace lloads of constants (including $<s>xxx) into
**	registers then to track the contents of register moves. Any time
**	a register gets reloaded with a value already in there - zap the
**	instruction . Later live/dead analysis wil get rid of superfluous
**	register loads that are left dangling. ( see slithole)
**
**	E.g.
**		mov	$<s>a , %ax
**		mov	%ax , %ds
**		mov	$5 , %ax
**		mov	$<s>a , %bx
**		mov	%bx , %ds
**
**	will become :-
**
**		mov	$<s>a , %ax
**		mov	%ax , %ds
**		mov	$5 , %ax
**		mov	$<s>a , %bx
**
**	the final mov into bx will not be optimised until live/dead
**	analysis determines that the value in bx is not required.
**		
**	PROBLEM
**
**	The graph is not constructed across switches so must assume the worst
**	at any hard labels
**
**	Also
**		turn shifts by cl
**	where cl is a constant into constant shifts
**
**	and
**		jcxz when its known not to be
**	
**	and
**		mov	 reg , somewhere
**		where reg is constant and somewhere else has value
**		in it
**
*/

unsigned xsets() ;

int
rtbboptim(pf, pl , rt )
NODE *pf, *pl;	/* ptrs to first and last nodes of block */
REGTRACE *rt ;  /* state of regisetrs at start of basic block */
{
	/*
	 *	killed - counts number of instructions zapped
	 *		 this is returned as the result of rtbboptim
	 *	rt->regok  - bit map of segments for which the information
	 *		 is correct
	 *	srcreg
	 *	dstreg - source and destination registers of an instruction
	 *	regsset- registers set by the instruction
	 *	value  - pointer to char string containing argument
	 *	rt->regis  - array of pointers to char strings giving current
	 *		 contents of register.
	 *	p      - pointer to current node
	 */

	int		killed = 0 ;
	unsigned	srcreg ,
			dstreg ,
			regsset ;
	char *		value ;
	NODE *		p ;

	p = pf ;

	if ( islabel(p) && ishlp(p) )
	{
		clear( rt , sizeof(REGTRACE) ) ;
	}

	do {
		regsset = xsets ( p ) ;


		/*
		 *	constant in cx/cl
		 *	shift cx/cl , something
		 */

		if (rt->regok & CX) 
		{
			if  (( (p->op>=RCL) && (p->op<=SHRB) ) 
				&& (dirref(p->op1) & CX ) )
			{
				p->op1 = rt->regis[regnum("%cx")] ;
				nmsh++ ;
			}

			if ( (p->op==JCXZ) && (conval(rt->regis[regnum("%cx")])!=0))
			{
				DELNODE(p);
				killed++ ;
				nmenop++;
				continue ;
			}
		}

		/*
		 * If mov reg , memory and regok and
		 * another lo
		 */

		if( (p->op==MOV) && !strcmp(p->op1,"%dx") && (rt->regok & DX)
		   && (rt->regok & AX) && !strcmp(rt->regis[regnum("%ax")],
						rt->regis[regnum("%dx")]) )
		{
			p->op1 = "%ax" ;
		}

		/* check if clr instruction on rt->register */

		if( (p->op == CLR) && (dstreg = regnum(p->op1)) )
		{
			value = "$0" ;
		}
		else
		if ( (p->op == MOV) && (dstreg = regnum (p->op2))
		     && ( ( (srcreg = regnum(p->op1)) && (uses(p) & rt->regok) )
			  ||
			  ( iscon(p->op1) )
			)
		   )
		{
			if(srcreg)
				value = rt->regis[srcreg] ;
			else
				value = p->op1 ;
		}
		else
		if ( (p->op == XCHG) && (srcreg=regnum(p->op1)) &&
		     (dstreg=regnum(p->op2)) )
		{
			value = rt->regis[srcreg] ;

			regsset = rt->regok ;   /* convenient temporary */

			if( rt->regok & dirref(p->op2))
			{
				rt->regis[srcreg] = rt->regis[dstreg] ;
				rt->regok |= dirref(p->op1) ;
			}
			else
				rt->regok &= ~dirref(p->op1) ;

			if( regsset & dirref(p->op1))
			{
				rt->regis[dstreg] = value ;
				rt->regok |= dirref(p->op2) ;
			}
			else
				rt->regok &= ~dirref(p->op2) ;

			continue ;
		}
		else
		{
			/* can't track anything */

			rt->regok &= ~regsset ;
			value = NULL ;
		}

		/* if we did track something value will be non NIL */

		if( value )
		{
			if( (regsset & rt->regok) && !(rt->regis[dstreg]==NULL)
				&& !strcmp(rt->regis[dstreg] , value) )
			{
				DELNODE(p) ;
				if( regsset & (ES | DS | SS | CS) )
					nmsloads++ ;
				else
					nmloads++ ;
				killed ++ ;
			}

			rt->regis[dstreg] = value ;
			rt->regok |= regsset ;
		}

		/* go on to next instruction */

	} while ( p = p->forw , p != pl->forw ) ;

	return(killed) ;
}
