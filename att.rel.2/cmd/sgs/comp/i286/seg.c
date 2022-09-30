/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)seg.c	1.4 - 85/08/09 */

# include "mfile2"

# ifdef SSPLIT

# ifndef FLEXNAMES
# define NCMP(x, y)	!strncmp(x->in.name, y->in.name, NCHNAM)

# else
# define NCMP(x, y)	!strcmp(x->in.name, y->in.name)
# endif

static struct segenv topenv;
struct segenv *segenv = &topenv;

splitops(p)

register NODE *p;
{
	register NODE *n;
	int doit;

	doit = 0;
	n = p;
	switch(p->in.op)
		{
		case UNARY AND:
			n = p->in.left;
			if(n->in.op == UNARY MUL)
				n = n->in.left;
			break;

		case UNARY MUL:
			/* no splitting op for structure umuls */
			if(p->in.type == STRTY)
				break;

			n = p->in.left;
			if( !shumul(n) ) {
				doit++;
				if( ( (n->in.op != PLUS) &&
				      ( (n->in.op != MINUS) ||
					(n->in.right->in.name[0] != 0) ) ) ||
				    (n->in.right->in.op != ICON) )
					n = p;
			}
			break;

		case FORCE:
			if( ISPTR(p->in.type) ) {
				p->in.type = ULONG;
				segtolong(p->in.left);
			}
			break;

		case PLUS:
			if( (p->in.right->in.op == ICON) &&
			    !ISPTR(p->in.right->in.type) )
				break;
		case MINUS:
			if( ISPTR(p->in.type) )
				doit++;
			break;

		case QUEST:
			if( ISPTR(p->in.left->in.type) )
				segtolong(p->in.left);

		case ASSIGN:
			if( !ISPTR(p->in.type) )
				break;

		case NAME:
		case STASG:
		case STARG:
			doit++;
			break;

		case CBRANCH:
			if( ISPTR(p->in.left->in.type) )
				segtolong(p->in.left);
			break;

		case NOT:
		case ANDAND:
		case OROR:
		case EQ:
		case NE:
		case LE:
		case LT:
		case GE:
		case GT:
		case UGT:
		case UGE:
		case ULT:
		case ULE:
			segtolong(p);
			break;
		}

	switch( optype(n->in.op) )
		{
		case BITYPE:
			splitops(n->in.right);
		case UTYPE:
			splitops(n->in.left);
			break;
		}

	if( !doit )
		return;

	n = talloc();
	*n = *p;
	p->in.op = UNARY SSPLIT;
	p->in.segment = DS;	/* default segment target */
	p->in.left = n;
	p->tn.rval = -1;
}

setsplit(p, cook)

register NODE *p;
int cook;
{
	register NODE *q;
	struct segenv lres, rres;
	NODE *n;

	q = p->in.left;

	switch(q->in.op)
		{
/*
		case SCONV:
			order(q->in.left, cook);
			q = q->in.left;

			n = talloc();
			*n = *q;
			n->tn.op = ICON;
			n->tn.lval = 3;
			n->tn.type = INT;
			p->in.left->in.right = n;

			n = p->in.left;
			n->in.op = ISPTR(n->in.type) ? ASG LS : ASG RS;
			n->in.left = tcopy(&segenv->lastseg);
			n->in.type = INT;
			order(n, INTAREG|INTBREG);
			rfree(n->tn.rval, INT);

			segset(segenv, FREE, n);
			p->in.left = q;
			break;
*/

		case UNARY MUL:
			segsetenv(&lres, p->in.segment & ~MUSTDO);
			segpushenv(&lres);

			q = q->in.left;
			if( (q->in.op == PLUS) ||
			    (q->in.op == MINUS) ) {
				if( ISPTR(q->in.right->tn.type) ) {
					lres.prefseg = AX;
					order(q->in.left, INTBREG);
					segset(&lres, ICON, q->in.right);
				}
				else
					order(q->in.left, INTBREG);
			}
			else
				order(q, INTBREG);

			segpopenv(&lres);
			q = p->in.left;
			q->in.segment = segbind(&lres, p->in.segment);
			segbusy(&lres);
			break;

		case PLUS:
		case MINUS:
			segadd(q, cook);
			break;

		case ASSIGN:
			segasg(q, cook);
			break;

		case QUEST:
			segquest(q, cook);
			break;

		case NAME:
			segsetenv(&lres, AX);
			segset(&lres, ICON, q);
			q->in.segment = segbind(&lres, p->in.segment);
			segbusy(&lres);
			break;

		case STASG:
			segsetenv(&lres, ES);
			segsetenv(&rres, DS);
			if( !rfirst(q) ) {
				if( usplitchk(q->in.right) )
					lres.prefseg = CX;
			}
			else
				usplitset(q->in.left, ES);

			n = talloc();
			if(q->in.left->in.op == REG) {
				n->in.op = REG;
				n->tn.rval = ES;
				segset(&lres, FREE, n);
				segfree(&lres);
			}
			if(q->in.right->in.op == REG) {
				n->in.op = REG;
				n->tn.rval = DS;
				segset(&rres, FREE, n);
				segfree(&rres);
			}
			n->in.op = FREE;
			segact(q, (q->in.left->in.op  != REG) ? INTBREG : 0,
				  (q->in.right->in.op != REG) ? INTBREG : 0,
				  &lres, &rres);

			segbusy(&rres);
			q->in.left->in.segment = segbind(&lres, ES|MUSTDO);
			segfree(&rres);
			q->in.right->in.segment = segbind(&rres, DS);

			if(cook & (INTAREG|INTBREG))
				segset(segenv, FREE, &rres.lastseg);
			break;

		case STARG:
			segsetenv(&lres, DS);
			segpushenv(&lres);
			order(q->in.left, INTBREG);
			segpopenv(&lres);
			q->in.left->in.segment = segbind(&lres, DS);
			break;

		default:
			cerror("illegal node in USPLIT rewrite - get help");
		}

	*p = *q;
	q->in.op = FREE;

	return(1);
}

usplitchk(p)

register NODE *p;
{
	if(p->in.op == UNARY SSPLIT)
		return(1);

	switch( optype(p->in.op) )
		{
		case UTYPE:
			return( usplitchk(p->in.left) );

		case LTYPE:
			return(0);

		case BITYPE:
			return( usplitchk(p->in.right) |
				usplitchk(p->in.left) );
		}
}

usplitset(p, t)

register NODE *p;
int t;
{
	if( (p->in.op == UNARY SSPLIT) &&
	    ( (p->in.left->in.op == UNARY MUL) ||
	      (p->in.left->in.op == NAME) ) )
		p->in.segment = t;

	switch( optype(p->in.op) )
		{
		case BITYPE:
			usplitset(p->in.right, t);
		case UTYPE:
			usplitset(p->in.left, t);
			break;
		}
}

segadd(p, cook)

register NODE *p;
int cook;
{
	int lact;
	int ract;
	int rseg;
	struct segenv lres;
	struct segenv rres;

	lact = ract = 0;
	rseg = ISPTR(p->in.right->in.type);
	if( !canaddr(p->in.right) )
		ract =
		    ( rseg && couldaddr(p->in.right) ) ?
			INBREG : (INTAREG | INTBREG);

	if( !istnode(p->in.left) ) {
		lact = cook & (INTAREG | INTBREG);
		if( !lact )
			lact = INTAREG | INTBREG;
	}

	if( !lact && !ract )
		return;

	segsetenv(&lres, segenv->prefseg);
	segsetenv(&rres, segenv->prefseg);
	*( rseg ? &lres.prefseg : &rres.prefseg ) = AX;
	lres.lastseg.tn.rall = rres.lastseg.tn.rall =
		NOPREF | (p->in.rall | segenv->lastseg.in.rall) & NEEDREGS;
	segact(p, lact, ract, &lres, &rres);

	if(model == MDL_LARGE) {
		p = rseg ? &(rres.lastseg) : &(lres.lastseg);
		if(p->in.op != FREE)
			segset(segenv, FREE, p);
	}
}

segasg(p, cook)

register NODE *p;
int cook;
{
	NODE *n;
	int lact;
	int ract;
	struct segenv lres;
	struct segenv rres;

	lact = ract = 0;
	if( couldaddr(p->in.left) ) {
		lact = INBREG;
		if( !rfirst(p) && !canaddr(p->in.right) &&
		     (p->in.left->in.op == UNARY SSPLIT) )
			p->in.left->in.segment = ES;
	}

	if( (p->in.right->in.op != REG) &&
	    (p->in.right->in.op != ICON) ) {
		ract = cook & (INTAREG|INTBREG);
		if( !ract )
			ract = INTAREG;
	}

	if( !lact && !ract )
		return;

	segsetenv(&lres, AX);
	segsetenv(&rres, CX);
	rres.lastseg.in.rall = NOPREF | p->in.right->in.rall & NEEDREGS;
	segact(p, lact, ract, &lres, &rres);

	if( (cook != FOREFF) &&
	    (rres.lastseg.in.op != ICON) ) {
		rres.lastseg.in.rall = NOPREF | p->in.rall & NEEDREGS;
		segbind(&rres, AX);
	}
	segset(segenv, FREE, &rres.lastseg);
}

segquest(p, cook)

register NODE *p;
int cook;
{
	register NODE *r;
	struct segenv lres;
	int lab, lab1;
	int segreg;

	cook &= INTAREG|INTBREG;
	if( !cook )
		cook = INTAREG|INTBREG;
	r = p->in.right;
	segsetenv(&lres, AX);
	segpushenv(&lres);
	cbranch(p->in.left, -1, lab = getlab() );

	segsetenv(&lres, AX);
	r->in.left->in.rall = p->in.rall;
	order(r->in.left, cook);
	r->in.right->in.rall = r->in.left->tn.rval | MUSTDO;
	segfree(&lres);
	segreg = segbind(&lres, AX);
	reclaim(r->in.left, RNULL, 0);
	cbgen(0, lab1 = getlab(), 'I');

	deflab(lab);
	segsetenv(&lres, AX);
	order(r->in.right, cook);
	*p = *r->in.right;
	r->in.right->in.op = FREE;
	r->in.op = FREE;

	segpopenv(&lres);
	segbind(&lres, segreg | MUSTDO);
	deflab(lab1);
	segset(segenv, FREE, &lres.lastseg);
}

segact(p, lact, ract, lseg, rseg)

register NODE *p;
int lact;
int ract;
struct segenv *lseg;
struct segenv *rseg;
{
	int xact;

# ifndef BUG4
	if(odebug > 1)
		pract("segact", p, lact, ract);
# endif

	if( rfirst(p) ) {
		if( ract ) {
			segpushenv(rseg);
			doact1(p->in.right, ract);
			segpopenv(rseg);
			segbusy(rseg);
		}
		xact = 0;
	}
	else
		xact = ract;

	if( lact ) {
		segpushenv(lseg);
		doact1(p->in.left, lact);
		segpopenv(lseg);
		segbusy(lseg);
	}

	if( xact ) {
		segpushenv(rseg);
		doact1(p->in.right, ract);
		segpopenv(rseg);
		xact = 0;
	}
	else
		xact = ract;

	if( lact )
		segfree(lseg);

	if( xact )
		segfree(rseg);

	return(lact || ract);
}

segset(env, o, p)

struct segenv *env;
int o;
register NODE *p;
{
	int rall;

	rall = env->lastseg.in.rall;
	env->lastseg = *p;
	p = &env->lastseg;
	p->in.type = INT;
	p->in.rall = rall;
	p->in.su = 0;
	if(o != FREE)
		p->in.op = o;

	switch(o)
		{
		case REG:
			p->tn.rval++;
			break;

		case OREG:
		case NAME:
			p->tn.lval += 2;
			break;

		case ICON:
			if(p->tn.name[0] != 0) {
				p->tn.segment = BX;
				if(model == MDL_LARGE)
					p->tn.lval = 0;
			}
			break;
		}
	segbusy(env);

# ifndef BUG4
	if(odebug > 1) {
		printf("segset - lastseg\n");
		fwalk(p, eprint, 1);
	}
# endif
}

segsetenv(env, pref)

register struct segenv *env;
int pref;
{
	env->prefseg = pref;
	env->lastseg.in.op = FREE;
	env->lastseg.in.type = INT;
	env->lastseg.in.rall = NOPREF;
}

segpushenv(env)

register struct segenv *env;
{
	env->lastenv = segenv;
	segenv = env;
}

segpopenv(env)

register struct segenv *env;
{
	static NODE nullnode;

	segfree(env);
	if(env->lastseg.tn.op == FREE) {

# ifdef FLEXNAMES
		nullnode.tn.name = "";
# endif
		segset(env, ICON, &nullnode);
	}
	else if( (env->prefseg != AX) &&
		 (env->lastseg.tn.op != ICON) &&
		 ( (env->lastseg.tn.op != OREG) ||
		   (env->lastseg.tn.rval != BP) ) )
		segbind(env, env->prefseg);

	segenv = env->lastenv;
}

segbusy(env)

struct segenv *env;
{
	register NODE *p;

	p = &env->lastseg;
	if(p->in.op == REG)
		rbusy(p->tn.rval, INT);
}

segfree(env)

struct segenv *env;
{
	register NODE *p;

	p = &env->lastseg;
/*	if( (p->in.op == REG) ||
	    (p->in.op == OREG) ) */
	if(p->in.op == REG)
		rfree(p->tn.rval, INT);
}

segput(t)

int t;
{
	register char *s;

	switch(t)
		{
		case BX:
			printf("$<s>");
			return;

		case SS:
		case DS:
		case ES:
		case CS:
			s = rnames[t];
			printf("%s:", s);
			return;
		}
}

segbind(env, target)

struct segenv *env;
register int target;
{
	register NODE *n;
	int mustdo;
	int rall;

	mustdo = target & MUSTDO;
	target &= ~MUSTDO;
	n = &env->lastseg;

	switch(n->in.op)
		{
		case FREE:
			return(AX);

		case REG:
			if(n->tn.rval == target)
				return(target);
			if(n->tn.rval >= SS) {
				if(target >= SS) {
					if( !mustdo )
						return(n->tn.rval);
					rbusy(n->tn.rval, INT);
					if( match(n, INTAREG) != MDONE )
						return(AX);
					mustdo++;
				}
			}
			else {
				if( !mustdo && (target < SS) )
					return(n->tn.rval);
				rbusy(n->tn.rval, INT);
			}
			break;

		case OREG:
			rbusy(n->tn.rval, INT);
		case NAME:
			if(n->tn.segment >= SS)
				rbusy(n->tn.segment, INT);
			break;

		case ICON:
			if(n->tn.name[0] != 0) {
				n->tn.op = NAME;
			}
			else
				n->tn.lval >>= SZINT;

			if(target < SS)
				break;
			if( match(n, INTAREG|INTBREG) != MDONE)
				return(AX);
			break;

		default:
			cerror("illegal node in segment binding - get help");
		}

	n->tn.type = INT;
	rall = target;
	if( mustdo )
		rall |= MUSTDO;
	else if(target < SS)
		rall = NOPREF;
	n->tn.rall = rall | n->tn.rall & NEEDREGS;;
	if( match(n, INTAREG|INTBREG) != MDONE )
		cerror("unbindable segment - get help");
	if((target >= SS) && (n->tn.rval != target))
		cerror("segment overwrite - get help");
	rfree(n->tn.rval, INT);
	return(n->tn.rval);
}

segtolong(p)

register NODE *p;
{
	register NODE *n;

	if( logop(p->in.op) ) {
		if(p->in.op != NOT) {
			if( ISPTR(p->in.right->in.type) )
				segtolong(p->in.right);
		}
		if( ISPTR(p->in.left->in.type) )
			segtolong(p->in.left);
		return;
	}

	switch(p->in.op)
		{
		case ICON:
			if(p->in.name[0] != 0)
				p->in.segment = BX;
		case NAME:
		case OREG:
		case UNARY CALL:
		case UNARY STCALL:
		case CALL:
		case STCALL:
			p->in.type = ULONG;
			return;

		case SCONV:
			p->in.type = ULONG;
			n = p->in.left;
			if( ISPTR(n->in.type) ||
			    ( fundtype(n->in.type) == LONG ) ) {
				segtolong(n);
				*p = *n;
				n->in.op = FREE;
			}
			return;

		case ASSIGN:
			if( fundtype(p->in.right->in.type) != LONG )
				segtolong(p->in.right);
			if( fundtype(p->in.left->in.type) != LONG )
				p->in.left->in.type = ULONG;
			p->in.type = ULONG;
			return;

		case UNARY MUL:
			if( ISPTR(p->in.type) ||
			    ( fundtype(p->in.type) == LONG ) ) {
				p->in.type = ULONG;
				return;
			}

		default:
			n = talloc();
			*n = *p;
			p->in.op = SCONV;
			p->in.left = n;
			p->in.type = ULONG;
			return;
		}
}
# endif
