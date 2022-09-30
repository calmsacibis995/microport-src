/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)order.c	1.8 - 85/08/22 */

# include "mfile2"

/*ARGSUSED*/
stoasg(p, o)

NODE *p;
int o;
{	/* should the assignment op p be stored,
	 * given that it lies as the right operand of o
	 * (or the left, if o==UNARY MUL)
	 * 8086 does nothing
	 */

}

deltest(p)

NODE *p;
{	/* should we delay the INCR or DECR operation p */

	switch(p->in.left->in.op)
		{
		case UNARY MUL:
			p = p->in.left->in.left;
			if( !shumul(p) )
				return(0);
			break;
# ifdef SSPLIT
		case UNARY SSPLIT:
# endif
		case FLD:
			return(0);
		}

	return(1);
}

mkadrs(p)

register NODE *p;
{	/* select the subtree to be stored */
	register NODE *n;

	if( asgop(p->in.op) ) {
		if( p->in.left->in.su >= p->in.right->in.su ) {
			n = p->in.left;
			if(n->in.op == FLD)
				n = n->in.left;
# ifdef SSPLIT
			else if ((n->in.op == UNARY SSPLIT) &&
				 (n->in.left->in.op == UNARY MUL))
				n = n->in.left;
# endif
		}
		else {
			if( canaddr(p->in.left) )
				return;
			n = p->in.right;
		}
	}
	else
		n = (p->in.left->in.su > p->in.right->in.su) ?
			p->in.left : p->in.right;

# ifdef SSPLIT
	if( (n->in.op == UNARY SSPLIT) &&
	    (n->in.left->in.op == UNARY MUL) &&
	    ( fundtype(n->in.type) == FLOAT ) )
		n = n->in.left;
# endif

	if(n->in.op == UNARY MUL)
		n = n->in.left;
	SETSTO(n, INTEMP);
}

/*ARGSUSED*/
notoff(t, r, off, cp)

TWORD t;
int r;
CONSZ off;
char *cp;
{	/* is it legal to make an OREG or NAME entry which has an
	 * offset of off, (from a register of r), if the
	 * resulting thing had type t
	 * 1 = NO; 0 = YES
	 */

	if( R2TEST(r) ) {
		if( R2UPK2(r) != BP )
			return(1);
		switch( R2UPK1(r) )
			{
			case SI:
			case DI:
				return(0); /* only %si and %di may double with %bp */
			default:
				return(1);
			}
	}
	else
		switch(r)
			{
			case BX:
			case SI:    /* only the bregs may be used for offsets */
			case DI:
			case BP:
			case SP:
				return( 0 );
			default:
				return( 1 );
			}
}

# define max(x,y) ((x) < (y) ? (y) : (x))
# define min(x,y) ((x) < (y) ? (x) : (y))

canaddr(p)

register NODE *p;
{	/* can you address the node ? */

	switch(p->in.op)
		{
		case NAME:
		case OREG:
		case REG:
		case ICON:
			return(1);

		case UNARY MUL:
			return( shumul(p->in.left) );
		}
	return(0);
}

couldaddr(p)

register NODE *p;
{	/* could you address the node ? */
	int t, lt;

	switch(p->in.op)
		{
		case UNARY MUL:
			return(1);

		case FLD:
			if( couldaddr(p->in.left) )
				return(1);
			break;

		case SCONV:
			t = fundtype(p->in.type);
			lt = fundtype(p->in.left->in.type);
			if( ( lt == CHAR ) ||
			    ( t == FLOAT ) ||
			    ( szty(p->in.left->in.type) < szty(p->in.type) ) )
				break;
# ifdef SSPLIT
		case UNARY SSPLIT:
# endif
			if( canaddr(p->in.left) || couldaddr(p->in.left) )
				return(1);
			break;
		}
	return(0);
}

fundtype(t)

register TWORD t;
{
	/*
	 * fundamental type of the node
	 * from the viewpoint of the code generator
	 */

	if( ISPTR(t) )
		return(INT);

	switch(t)
		{
		case UCHAR:
			t = CHAR;
			break;

		case UNSIGNED:
			t = INT;
			break;

		case ULONG:
			t = LONG;
			break;

		case DOUBLE:
			t = FLOAT;
			break;
		}
	return((int)t);
}

sucomp(p)

register NODE *p;
{	/* set the su field in the node to the
	 * sethi-ullman number, or local equivalent */

	register int lregs, rregs;
	int tregs;
	register int lt, rt;

	tregs = 0;
	switch( optype(p->in.op) )
		{
		case LTYPE:
			lregs = szty(p->in.type);
			break;

		case BITYPE:
			sucomop(p);
			if( ( fundtype(p->in.left->in.type) == FLOAT ) &&
			    shtemp(p->in.right) )
				rregs = 1;
			else if( canaddr(p->in.right) )
				rregs = 0;
			else if( couldaddr(p->in.right) )
				rregs = 1;
			else
				rregs = szty(p->in.right->in.type);

		case UTYPE:
			lregs = szty(p->in.left->in.type);
			break;
		}

	/*
	 * sethi-ullman rule pathologies induced by
	 * irregularities in the 8086 architecture
	 *
	 *	quite a few, eh?
	 */

	p->in.su = 0;
	switch(p->in.op)
		{
# ifdef SSPLIT
		case UNARY SSPLIT:
			p->in.su = (p->in.left->in.op == NAME) ?
					p->in.left->in.su + 1 :
					p->in.left->in.su;
			return;
# endif

		case UNARY MUL:
			lregs = shumul(p->in.left) ? 0 : 1;
			goto utype;

		case SCONV:
			lregs = szty(p->in.type);
			if(p->in.left->in.op == UNARY MUL)
				lregs++;
			goto utype;

		case REG:
			if( istreg(p->tn.rval) )
				p->in.su = szty(p->in.type);
			return;

		case OREG:
			if( istreg(p->tn.rval) )
				p->tn.su = 1;
			return;

		case STASG:
			lregs = rregs = tregs = 1;
			goto bitype;

		case UNARY CALL:
		case UNARY FORTCALL:
		case UNARY STCALL:
		case CALL:
		case FORTCALL:
		case STCALL:
		case HARDOP:
			p->in.su = fregs;	/* all regs needed */
			return;

		case DIV:
		case MOD:
			if( fundtype(p->in.type != FLOAT) ) {
				lregs = 2;
				if(p->in.right->in.op == ICON)
					rregs++;
			}
			goto bitype;

		case MUL:
			if( ISUNSIGNED(p->in.type) &&
			    (p->in.right->in.op == ICON) ) {
				rregs++;
				goto bitype;
			}
			tregs++;

		case LS:
		case RS:
			if( fundtype(p->in.type) == LONG ) {
				if(p->in.right->in.op == ICON) {
					if((lt = p->in.right->tn.lval) < 0)
						lt = -lt;
					if((lt < LTHRESH) || (lt >= SZINT))
						goto bitype;
				}
				tregs++;
				goto bitype;
			}

			if(p->in.right->in.op != ICON)
				tregs++;

			goto bitype;

		case ANDAND:
		case OROR:
		case QUEST:
		case COLON:
		case COMOP:
			tregs = lregs;
			lregs = 0;
			goto bitype;
		}

	/*
	 * the default, non-pathological
	 * sethi-ullman rules
	 */

	switch( optype(p->in.op) )
		{ 
		case UTYPE:
utype:
			p->in.su = max(p->in.left->in.su, lregs);

		case LTYPE:
			return;

		case BITYPE:
bitype:
			p->in.su = rfirst(p) ?

			    /* r then l */
			    max(p->in.right->in.su, p->in.left->in.su + rregs) :

			    /* l then r */
			    max(p->in.left->in.su, p->in.right->in.su + lregs);

			lregs += rregs + tregs;
			if(lregs > p->in.su)
				p->in.su = lregs;

			return;
		}
}

sucomop(p)

register NODE *p;
{
	register NODE *l;

	switch(p->in.op)
		{
		case PLUS:
			if( !ISPTR(p->in.type) )
				break;
		default:
			return;

		case AND:
		case OR:
		case ER:
		case MUL:
		case EQ:
		case NE:
		case LE:
		case LT:
		case GE:
		case GT:
		case ULE:
		case ULT:
		case UGE:
		case UGT:
			break;
		}

	l = p->in.left;
	if( (p->in.right->in.su > l->in.su) &&
	    ( canaddr(l) || couldaddr(l) ) &&
	    ( fundtype(l->in.type) != FLOAT ) ) {
		p->in.left = p->in.right;
		p->in.right = l;
		if( logop(p->in.op) )
			p->in.op = revrel[p->in.op - EQ];
	}
}

int radebug = 0;

needregs(p)

register NODE *p;
{ 
	switch(p->in.op)
		{ 
		case SCONV:
			if( (p->in.left->in.type == CHAR) ||
			    ( ( fundtype(p->in.type) == LONG ) &&
			      (p->in.left->in.type == INT) ) )
				return(NEEDREGS);
			break;

		case MUL:
			if( (p->in.type == INT) &&
			    (p->in.right->in.op == ICON) )
				break;

		case DIV:
		case MOD:
			return(NEEDREGS);

		case EQ:
		case NE:
		case LE:
		case LT:
		case GE:
		case GT:
		case ULE:
		case ULT:
		case UGE:
		case UGT:
			if( ( fundtype(p->in.left->in.type) == FLOAT) ||
			    ( fundtype(p->in.right->in.type) == FLOAT) )
				return(NEEDREGS);
			break;

		case CBRANCH:
			if( fundtype(p->in.left->in.type) == FLOAT)
				return(NEEDREGS);
			break;
		}

	return(0);
}

# ifndef BUG1
# define RAPRNT(x, y)\
	if(radebug) {\
		printf("rallo( %lo ) - %s\n", (long) y, x);\
		fwalk(y, eprint, 0);\
		}
# else
# define RAPRNT(x, y)
# endif

rallo(p, down)

register NODE *p;
int down;
{ 
	RAPRNT("before", p);
	rallo1(p);
	RAPRNT("after rallo1", p);
	rallo2(p, down);
	RAPRNT("after rallo2", p);
}

rallo1(p)

register NODE *p;
{ 
	register int rall;
	register NODE *n;

	switch( optype(p->in.op) )
		{ 
		case UTYPE:
			rallo1(p->in.left);
			rall = p->in.left->in.rall;
			break;

		case LTYPE:
			rall = NOPREF;
			break;

		case BITYPE:
			rallo1(p->in.right);
			rallo1(p->in.left);
			rall =  p->in.left->in.rall | p->in.right->in.rall;
			break;
		}
	p->in.rall = rall | needregs(p);
}

rallo2(p, down)

register NODE *p;
int down;
{	/* do register allocation */
	register int down1, down2;
	register int ty;
	int t, tl;

	down2 = NOPREF;
	down1 = down & ~(MUSTDO | NEEDREGS);
	ty = optype(p->in.op);

	if(ty == BITYPE) {
		if( rfirst(p) )
			down2 |= p->in.left->in.rall & NEEDREGS;
		else
			down1 |= p->in.right->in.rall & NEEDREGS;
	}

	switch(p->in.op )
		{ 
		case UNARY MUL:
			down1 = NOPREF;
			break;

		case STARG:
			down1 = SI | MUSTDO;
			break;

		case SCONV:
			t  = fundtype(p->in.type);
			tl = fundtype(p->in.left->in.type);

			if( ( (t == FLOAT) && (tl != FLOAT) ) ||
			    ( (t != FLOAT) && (tl == FLOAT) ) )
				down1 = NOPREF;

			else if(t == LONG) {
				if(p->in.left->in.type == INT)
					down1 = AX | MUSTDO;
				else
					down1 |= down;
			}

			else if(p->in.left->in.type == CHAR)
				down1 = AX | MUSTDO;

			break;

		case FORCE:
			if( fundtype(p->in.type) == FLOAT)
				down1 = ST0 | MUSTDO;
			else
				down1 = AX | MUSTDO;
			break;

# ifdef SSPLIT
		case UNARY SSPLIT:
			down1 |= down;
			break;
# endif

		case CM:
			if(p->in.type == LONG) {
				down1 = AX | MUSTDO | (down1 & NEEDREGS);
				down2 = BX | MUSTDO | (down2 & NEEDREGS);
				break;
			}
			else if(p->in.type == ULONG) {
				down1 = NOPREF | down1 & NEEDREGS;
				break;
			}

		case CALL:
		case FORTCALL:
		case STCALL:
			down1 = down2 = NOPREF;
			break;

		case ASSIGN:
			if( fundtype(p->in.right->in.type) != FLOAT )
				down2 = (down & ~MUSTDO) | (down2 & NEEDREGS);
			down1 = NOPREF | (down1 & NEEDREGS);
			break;

		case STASG:
			down2 = SI | MUSTDO;
			down1 = DI | MUSTDO;
			break;

		case ASG MUL:
		case MUL:
			if( (p->in.type == INT) &&
			    (p->in.right->in.op == ICON) )
				break;

			if( fundtype(p->in.type) != FLOAT ) {
				down1 = AX | MUSTDO;
				down2 = (down2 & NEEDREGS) ?
					NEEDREGS : DX | MUSTDO;
			}
			break;

		case ASG DIV:
		case ASG MOD:
		case DIV:
		case MOD:
			/* force left op into ax+dx pair */
			if( fundtype(p->in.type) != FLOAT ) {
				down1 = AX | MUSTDO;
				down2 = NEEDREGS;
			}
			break;

		case ASG LS:
		case ASG RS:
		case LS:
		case RS:
			down1 = NOPREF;
			down2 = CX | MUSTDO;
			break;

		case INCR:
		case DECR:
			down1 = down;
			break;

		case NOT:
		case EQ:
		case NE:
		case LE:
		case LT:
		case GE:
		case GT:
		case ULE:
		case ULT:
		case UGE:
		case UGT:
			down1 = NOPREF | (down1 & NEEDREGS);
			break;

		case ANDAND:
		case OROR:
		case QUEST:
		case COMOP:
			down1 = NOPREF;
			break;
		}

	p->in.rall = down;

	if(ty != LTYPE)
		rallo2(p->in.left, down1);

	if(ty == BITYPE)
		rallo2(p->in.right, down2 );
}

asgchk(p)

register NODE *p;
{ 
	/*
	 * Check for an assignment operator
	 * anywhere in the tree topped by p
	 * Assignment ops have to be caught
	 * because they usually need both bregs
	 */

	switch( optype(p->in.op) )
		{ 
		case UTYPE:
			return( asgchk(p->in.left) );

		case LTYPE:
			return(0);

		case BITYPE:
			return( (p->in.op == ASSIGN) ?
				    1 :
				    asgchk(p->in.left) || asgchk(p->in.right) );
		}
}

needrchk(p)

NODE *p;
{ 
	register int n;

	if( n = needregs(p) )
		return(n);

	switch( optype(p->in.op ) )
		{ 
		case UTYPE:
			return( needrchk(p->in.left) );

		case LTYPE:
			return(0);

		case BITYPE:
			return( needrchk(p->in.right) | needrchk(p->in.left) );
		}
}

rfirst(p)

register NODE *p;
{ 
	register int LleR;

	LleR = (p->in.left->in.su <= p->in.right->in.su);

	if( ( ( fundtype(p->in.type) == FLOAT) &&
	      ( (dope[p->in.op] & (FLOFLG|ASGOPFLG)) == FLOFLG) ) ||
	    ( (EQ <= p->in.op) && (p->in.op <= UGT) &&
	      (p->in.left->in.type == DOUBLE) ) )
		return( !couldaddr(p->in.right) );

	switch(p->in.op )
		{ 
		case ASSIGN:
			if( asgchk(p->in.right) )
				return(1);
			if( asgchk(p->in.left) )
				return(0);

			return(LleR);

# ifdef SSPLIT
		case PLUS:
		case MINUS:
			if( (model >= MDL_LARGE) && ISPTR(p->in.type) )
				return( ISPTR(p->in.left->in.type) ? 1 : 0 );
			break;
# endif

		case ASG DIV:
		case ASG MOD:
		case ASG MUL:
		case DIV:
		case MOD:
		case MUL:
			return( needrchk(p->in.right) ||
				(p->in.right->in.su > 1) );

		case CM:
			if( (p->in.type == ULONG) ||
			    ( (p->in.type == LONG) &&
			      !needrchk(p->in.right) ) )
				break;
			return(1);

		case ANDAND:
		case OROR:
		case QUEST:
		case COLON:
		case COMOP:
			return(0);
		}

	if ((dope[p->in.op] & ASGOPFLG) && LleR)
		return(1);

	return(p->in.left->in.su < p->in.right->in.su);
}

doact(p, lact, ract)

register NODE * p;
int lact;
int ract;
{ 
	int xact;

# ifndef BUG4
	if(odebug > 1)
		pract("doact", p, lact, ract);
# endif

	if( !lact && !ract )
		return(0);

	if( rfirst(p) ) {
		if( ract )
			doact1(p->in.right, ract);
		xact = 0;
	}
	else
		xact = ract;

	if(lact)
		doact1(p->in.left, lact);

	if(xact)
		doact1(p->in.right, ract);

	return(1);
}

doact1(p, a)

register NODE *p;
int a;
{ 
	register NODE *n;

	if(a & INBREG) {
		a &= ~INBREG;
		switch(p->in.op)
			{
			case UNARY MUL:
				offstar(p->in.left, a);
				break;
# ifdef SSPLIT
			case UNARY SSPLIT:
				order(p, INTBREG|SNAME|SOREG);
				break;
# endif
			case FLD:
				doact1(p->in.left, INBREG);
				break;

			case SCONV:
				if( fundtype(p->in.left->in.type) == FLOAT ) {
					order(p->in.left, INTAREG);
					if( match(p, INTEMP) != MDONE )
						goto stuck;
					break;
				}
				else if( !canaddr(p->in.left) ) {
					doact1(p->in.left, INBREG);
					canon(p->in.left);
				}
				if( couldaddr(p) ) {
					n = p->in.left;
					n->in.type = p->in.type;
					*p = *n;
					n->in.op = FREE;
				}
				else
stuck:					cerror("stuck sconv - get help");
				break;

			default:
				order(p, a);
			}
	}
	else
		order(p, a);
}

# ifndef BUG4

pract(s, p, lact, ract)

char *s;
register NODE *p;
int lact;
int ract;
{
	printf("%s( %o, ", s, p);
	if(lact)
		prcook(lact);
	else
		putchar('0');
	printf(", ");
	if(ract)
		prcook(ract);
	else
		putchar('0');
	printf(" )\n");
	fwalk(p, eprint, 0);
}
# endif

setbin(p, cook)

register NODE *p;
int cook;
{ 
	register NODE *r, *l;
	int lact, ract;

	r = p->in.right;
	l = p->in.left;
	lact = ract = 0;

	switch(p->in.op)
		{ 
		case DIV:
		case MOD:
		case MUL:
			if( !canaddr(r) || (r->in.op == ICON) )
				ract = couldaddr(r) && !rfirst(p) ?
					INBREG : (INTAREG | INTBREG);

			if( !istnode(l) )
				lact = INTAREG;

			if( (p->in.op == MUL) &&
			    (r->in.op == ICON) ) {
				if(p->in.type == INT) {
					ract = 0;
					if( couldaddr(l) )
						lact = INBREG;
					return( doact(p, lact, ract) );
				}
				r->in.type = INT;
			}

			if( (p->in.type == DOUBLE) && shtemp(r) )
				ract = INTAREG;

			break;

		case PLUS:
		case MINUS:
		case AND:
		case OR:
		case ER:
			if( !canaddr(r) )
				ract =
				    couldaddr(r) && !rfirst(p) ?
					INBREG : (INTAREG | INTBREG);

			if( (p->in.type == DOUBLE) && shtemp(r) )
				ract = INTAREG;

			if( !istnode(l) ) {
				lact = cook & (INTAREG | INTBREG);
				if( !lact )
					lact = INTAREG | INTBREG;
			}

			if( (cook == FORCC) &&
			    ((p->in.op != PLUS) && (p->in.op != MINUS)) ) {
				if(r->in.op == ICON) {
					if( couldaddr(l) )
						lact = INBREG;
				}
				else if(r->in.op != REG)
					ract = INTAREG;
				return( doact(p, lact, ract) );
			}

			break;

		case LS:
		case RS:
			if( !istnode(r) &&
			    ( (r->in.op != ICON) || ( r->tn.name[0] != 0) ) )
				ract = INTAREG | INTBREG;

			if( !istnode(l) ) {
				lact = cook & (INTAREG | INTBREG);
				if( !lact )
					lact = INTAREG | INTBREG;
			}

			break;

		case EQ:
		case NE:
		case LE:
		case LT:
		case GE:
		case GT:
		case ULE:
		case ULT:
		case UGE:
		case UGT:
			if( !canaddr(r) )
				ract =
				    couldaddr(r) && !rfirst(p) ?
					INBREG : (INTAREG | INTBREG);

			if( (p->in.left->in.type == DOUBLE) && shtemp(r) )
				ract = INTAREG;

			if(r->in.op == ICON) {
				if( !canaddr(l) || (l->in.op == ICON) )
					lact = couldaddr(l) ?
						INBREG : INTAREG | INTBREG;
			}
			else if( !istnode(l) )
				lact = INTAREG | INTBREG;

			return( doact(p, lact, ract) );
		}
	/*
	 * implement setbin actions with
	 * order specified by rfirst
	 * make addressable if INBREG is set
	 * otherwise compile using action as cookie
	 */

	doact(p, lact, ract);
	return(0);
}

setasg(p, cook)

register NODE *p;
int cook;
{	/* setup for assignment operator */
	register int lact, ract;
	NODE *r;

# ifdef FORT
	if(p->in.op == GOTO) {
		if( couldaddr(p->in.left) ) {
			doact1(p->in.left, INBREG);
			return(1);
		}
		return(0);
	}
# endif

	lact = ract = 0;
	r = p->in.right;

	if( !istnode(r) ) {
		if( fundtype(r->in.type) == FLOAT )
			ract = INTAREG;
		else if(r->in.op != ICON)
			ract = INTAREG | INTBREG;
	}

	if( couldaddr(p->in.left) ) {
		lact = INBREG;
		if( !rfirst(p) && !canaddr(p->in.right) &&
		     (p->in.left->in.op == UNARY SSPLIT) )
			p->in.left->in.segment = ES;
	}

	return( doact(p, lact, ract) );
}

setasop(p, cook)

register NODE *p;
int cook;
{	/* setup for =ops */
	register NODE *l, *r;
	int lact, ract;

	if( p->in.type == DOUBLE) {
		p->in.op -= ASG 0;
		return(0);
	}

	l = p->in.left;
	r = p->in.right;
	lact = ract = 0;

	if(l->in.op != REG) {
		if( (r->in.op != REG) &&
		    ( (r->in.op != ICON) ||
		      (p->in.op == ASG DIV) || (p->in.op == ASG MOD) ) )
			ract = INTAREG | INTBREG;
	}

	else if( !canaddr(r) )
		ract = couldaddr(r) ? INBREG : INTAREG | INTBREG;

	if( couldaddr(l) )
		lact = INBREG;

	return( doact(p, lact, ract) );
}

offstar(p, cook)

register NODE *p;
int cook;
{
	if( ((p->in.op == PLUS) || (p->in.op == MINUS) ) &&
	    (p->in.right->in.op == ICON) )
		p = p->in.left;
	order(p, INTBREG);
}

setstr(p, cook)

register NODE *p;
int cook;
{	/* set up for structure assignment */
	if(p->in.op == STARG) {
		order(p->in.left, INTBREG);
		return(1);
	}
	return( doact(p, (p->in.left->in.op  != REG) ? INTBREG : 0,
			 (p->in.right->in.op != REG) ? INTBREG : 0 ) );
}

setincr(p, cookie)

register NODE *p;
int cookie;
{	/* setup for incr/decr operation (p++ or p--) */
	NODE *q;
	NODE *n;

	if( couldaddr(p->in.left) )
		doact1(p->in.left, INBREG);

	if(p->in.left->in.op != FLD)
		canon(p->in.left);
	else {
		n = tcopy(p);
		n->in.op = (p->in.op == INCR) ? PLUS : MINUS;
		order(n->in.left, INTAREG|INTBREG);

		p->in.op = ASSIGN;
		p->in.right->in.op = FREE;
		p->in.right = n;

		if(cookie != FOREFF) {
			q = n->in.left;
			n->in.left = tcopy(n->in.left);
			if( match(n->in.left, INTAREG|INTBREG) != MDONE )
				cerror("no temp reg for ++ - get help");
		}
		goto out;
	}

	if(cookie != FOREFF) {
		q = tcopy(p->in.left);
		cookie &= (FORARG|INTAREG|INTBREG);
		if( !cookie )
			cookie = INTAREG|INTBREG;

# ifdef SSPLIT
		if( (model >= MDL_LARGE) &&
		    (segenv->prefseg == q->in.segment) )
			segenv->prefseg = AX;
# endif
		match(q, cookie);
	}
	p->in.op = (p->in.op == INCR) ? ASG PLUS : ASG MINUS;
out:
	order(p, FOREFF);
	if(cookie != FOREFF) {
		*p = *q;
		q->in.op = FREE;
	}
	return(1);
}

getlab()
{	/* generate labels unique to pass 2 */
	static int crslab = 10000;
	return(crslab++);
}

deflab(l)
{	/* print a pass 2 label */
	printf(".%d:\n", l);
}

genargs(p)

register NODE *p;
{	/* generate code for the arguments */

	/*  first, do the arguments on the right (last->first) */
	while(p->in.op == CM) {
		genargs(p->in.right);
		p->in.op = FREE;
		p = p->in.left;
	}
# ifdef SSPLIT
	if(model >= MDL_LARGE)
		segsetenv(segenv, ISPTR(p->in.type) ? CX : AX);
# endif
	order(p, FORARG);
}

argsize(p)

register NODE *p;
{
	/* determine the size of the argument area */
	register t = 0;

	if(p->in.op == CM) {
		t = argsize(p->in.left);
		p = p->in.right;
	}

# ifdef SSPLIT
	if(p->in.op == UNARY SSPLIT)
		p = p->in.left;
# endif

	switch(p->in.type)
		{
		case LONG:
		case ULONG:
			SETOFF(t, ALLONG/SZCHAR);
			return(t + 4);

		case FLOAT:
		case DOUBLE:
			SETOFF(t, ALDOUBLE/SZCHAR);
			return(t + 8);
		}

	if( ISPTR(p->in.type) && (model >= MDL_LARGE)) {
		SETOFF(t, ALPOINT/SZCHAR);
		return(t + 4);
	}

	if(p->in.op == STARG) {
		SETOFF(t, p->stn.stalign);  /* alignment */
		return(t + p->stn.stsize);  /* size */
	}

	/* all else takes two bytes */
	SETOFF(t, ALINT/SZCHAR);
	return(t + 2);
}

	/*
	 * usable routine - for now it replaces
	 * the machine independent copy in allo.c
	 */

usable(p, n, r)

NODE *p;
register int n;
register int r;
{	/* decide if register r is usable in tree p to satisfy need n */

	if( !istreg(r) )
		cerror("usable asked about nontemp register");

	if( fundtype(p->in.type) == FLOAT ) {
		if(r == ST0) {
			fdepth++;
			return(1);
		}
	}

	if(busy[r] > 1)
		return(0);

	if( szty(p->in.type) == 2 ) {
		if( !istreg(r + 1) || (busy[r + 1] > 1) )
			return(0);

		if( ( (busy[r] == 0) ||
		      shareit(p, r, n) ) &&
		    ( (busy[r + 1] == 0) ||
# ifdef SSPLIT
		      ( (model >= MDL_LARGE) &&
			(p->in.op == SCONV) &&
			ISPTR(p->in.left->in.type) &&
			(segenv->lastseg.tn.op == REG) &&
			(segenv->lastseg.tn.rval == r + 1) ) ||
# endif
		      shareit(p, r + 1, n) ) ) {
			busy[r] |= TBUSY;
			busy[r + 1] |= TBUSY;
			return(1);
		}
		return(0);
	}

	if(busy[r] == 0) {
		busy[r] |= TBUSY;
		return(1);
	}

# ifdef SSPLIT
	if( (model >= MDL_LARGE) &&
	    ( (p->in.op == NAME) || (p->in.op == OREG) ) &&
	    (r >= SS) && (r == p->in.segment) )
		return(1);
# endif

	return( shareit(p, r, n) );
}
