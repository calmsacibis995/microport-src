/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)local.c	1.8 - 85/09/20 */
/*      this file contains code which is dependent on the target machine */

# include "mfile1"
# include "storclass.h"
# include <math.h>

extern FILE *outfile;
extern int prflag, dsflag, dlflag, startln, oldln, lastloc;
extern int bb_flags[];
extern char *rnames[];
extern char startfn[];

# define LINENO  lineno-startln+1

arraychk(p)

struct symtab *p;
{
	int i;
	int d;
	TWORD t;
	OFFSZ s;
	OFFSZ mult;
	OFFSZ maxarray;

	mult = 1L;
	d = p->dimoff;
	t = p->stype;
	s = dimtab[p->sizoff];
	switch(p->sclass)
		{
		case EXTERN:
		case STATIC:
		case MOS:
		case MOU:
			maxarray = MAXSTATIC;
			break;

		default:
			maxarray = MAXAUTO;
		}

	for(i =0; i <= (SZINT-BTSHIFT-1); i+=TSHIFT)
		switch( (t>>i) & TMASK )
			{
			case PTR:
				if( (model >= MDL_LARGE) ||
				    ( (model == MDL_MIDDLE) && ISFTN(t >> i + 1) ) )
					mult <<= 1;
				if( SZPOINT * mult > maxarray )
					uerror("array too large");
				return;

			case ARY:
			case LARY:
				mult *= dimtab[ d++ ];
				if( s * mult > maxarray )
					uerror("array too large");
				continue;
			}
}

#define CHARMASK(x)	((x == CHAR) ? 0177 : 0377)

NODE *
clocal(p)

NODE *p;
{	/* this is called to do local transformations on
	 * an expression tree preparitory to its being
	 * written out in intermediate code.
	 *
	 * the major essential job is rewriting the
	 * automatic variables (NAMEs) and arguments in terms of
	 * REG and OREG nodes
	 *
	 * conversion ops which are not necessary are also clobbered here
	 */

	register struct symtab *q;
	register NODE *r;
	register o;
	register m, ml;
	NODE *fixptr();

	fixchar(p);

	o = p->in.op;
	switch(o) {

	case NAME:
		if( p->tn.rval < 0 ) { /* already processed; ignore... */
			return(p);
			}
		q = &stab[p->tn.rval];
		switch( q->sclass ){

		case AUTO:
		case PARAM:
			/* fake up a structure reference for objects in the stack */
			r = block( REG, NIL, NIL, defptr()+STRTY, 0, 0 );
			r->tn.lval = 0;
			r->tn.rval = BP;
			p = stref( block( STREF, r, p, 0, 0, 0 ) );
			break;

		case ULABEL:
		case LABEL:
		case STATIC:
			/* remember the invented label number */
			if( q->slevel == 0 ) break;
			p->tn.lval = 0;
			p->tn.rval = -q->offset;
			break;

		case REGISTER:
			/* build a REGister node */
			p->in.op = REG;
			p->tn.lval = 0;
			p->tn.rval = q->offset;
			break;

			}
		break;

	case PCONV:
		if( ISPTR(p->in.left->in.type) ) {
			p->in.left->in.type = p->in.type;
			p->in.left->fn.cdim = p->fn.cdim;
			p->in.left->fn.csiz = p->fn.csiz;
			p->in.op = FREE;
			return(p->in.left);
		}
		p->in.op = SCONV;

	case SCONV:
		m = p->in.type;
		ml = p->in.left->in.type;

		if( ( (m == FLOAT) || (m == DOUBLE) ) &&
		    (p->in.left->in.op == FCON) ) {
			p->in.op = FREE;
			p = p->in.left;
			if(m == FLOAT)
				p->fpn.dval = (float) p->fpn.dval;
			return(p);
		}

		if( (p->in.left->in.op == ICON) &&
		    ( ISPTR(m) || !ISPTR(ml) ) ) {
			p->in.left->fn.cdim = p->fn.cdim;
			p->in.left->fn.csiz = p->fn.csiz;
			p->in.op = FREE;
			p = p->in.left;
			p->tn.type = m;
			switch(m)
				{
				case INT:
					p->tn.lval = (short) p->tn.lval;
					break;

				case UNSIGNED:
					p->tn.lval = (unsigned short) p->tn.lval;
					break;

				case ULONG:
					p->tn.type = LONG;

				case LONG:
					break;

				case CHAR:
					p->tn.lval = (char) p->tn.lval;
					p->tn.type = INT;
					break;

				case UCHAR:
					p->tn.lval = (unsigned char) p->tn.lval;
					p->tn.type = UNSIGNED;
					break;
				}
			return(p);
		}

		/* build compound conversions */
		if((m == CHAR) || (m == UCHAR)) {
			switch(ml)
				{
				case CHAR:
				case UCHAR:
					p->in.op = FREE;
					p = p->in.left;
					p->in.type = m;

				case INT:
				case UNSIGNED:
					return(p);
				}
			addsconv(p->in.left, (ml == ULONG) ? UNSIGNED : INT );
			return(p);
		}

		if((ml == CHAR) || (ml == UCHAR)) {
			if( (m != INT) && (m != UNSIGNED) )
				addsconv(p->in.left, (ml == UCHAR) ? UNSIGNED : INT );
			return(p);
		}

		break;

	case PVCONV:
		p->in.op = FREE;
		if(p->in.right->tn.lval != 1) {
			r = p->in.left;
			r->in.type = LONG;
			addsconv(r->in.left, UNSIGNED);
			addsconv(r->in.left, LONG);
			addsconv(r->in.right, UNSIGNED);
			addsconv(r->in.right, LONG);
			p = buildtree(DIV, p->in.left, p->in.right);
			addsconv(p, INT);
		}
		else {
			p->in.right->in.op = FREE;
			p = p->in.left;
			addsconv(p->in.left, INT);
			addsconv(p->in.right, INT);
		}
		return(p);

	case PMCONV:
		p->in.op = FREE;
		return( buildtree(MUL, p->in.left, p->in.right) );

	case FORCE:
		m = p->in.type;
		if(m == CHAR)
			m = INT;
		else if(m == UCHAR)
			m = UNSIGNED;
		p->in.type = m;

		if(m != p->in.left->in.type)
			addsconv(p->in.left, m);
		break;

	case ASG PLUS:
	case ASG MINUS:
	case ASG AND:
	case ASG OR:
	case ASG ER:
		if( ISPTR(p->in.type) &&
		    ( (p->in.right->in.type == LONG) ||
		      (p->in.right->in.type == ULONG) ) )
			addsconv(p->in.right, INT);
		break;

	case PLUS:
		m = p->in.type;
		ml = p->in.right->in.type;
		/* put pointer quantity on left */
		if( ( ISPTR(m) || ISARY(m) ) &&
		    ( ISPTR(ml) || ISARY(ml) ) ) {
			r = p->in.left;
			p->in.left = p->in.right;
			p->in.right = r;
		}

	case MINUS:
	case MUL:
		/* optimize address calculations with long indexes */
		m = p->in.type;
		if( ISPTR(m) || ISARY(m) ) {
			if( (p->in.left->in.type == LONG) ||
			    (p->in.left->in.type == ULONG) )
				addsconv(p->in.left, INT);

			if( (p->in.right->in.type == LONG) ||
			    (p->in.right->in.type == ULONG) )
				addsconv(p->in.right, INT);

			if(p->in.op != MUL)
				p = fixptr(p);
		}
		break;

	case ASG LS:
	case ASG RS:
	case LS:
	case RS:
		/* remove zero shifts */
		r = p->in.right;
		if( (r->in.op == ICON) &&
		    (r->tn.lval == 0) && (r->tn.rval == NONAME) ) {
			p->in.op = FREE;
			r->in.op = FREE;
			return(p->in.left);
		}

		/* remove type conversion of shift operand for long shifts */
		if( ( (p->in.type == LONG) || (p->in.type == ULONG) ) &&
		    (r->in.op == SCONV) && (r->in.type == INT) ) {
			r = r->in.left;
			if( (r->in.op == SCONV) &&
			    (r->in.type == p->in.type) ) {
				p->in.right->in.op = FREE;
				p->in.right = r->in.left;
				r->in.op = FREE;
			}
		}

		r = p->in.right;
		if(r->in.op == SCONV)
			switch(r->in.left->in.type)
				{
				case INT:
				case UNSIGNED:
				case CHAR:
				case UCHAR:
					r->in.op = FREE;
					p->in.right = r->in.left;
					break;
				}
		break;

	case COLON:
		m = p->in.type;
		if(m != p->in.left->in.type)
			addsconv(p->in.left, m);
		if(m != p->in.right->in.type)
			addsconv(p->in.right, m);
		break;
	}
	return(p);
}

addsconv(p, t)

register NODE *p;
int t;
{
	register NODE *n;

	n = talloc();
	*n = *p;
	n = block(SCONV, n, NIL, t, 0, t);
	n = clocal(n);
	*p = *n;
	n->in.op = FREE;
}

fixchar(p)

register NODE *p;
{
	register int lt, rt;
	NODE *n;
	int o;
	CONSZ v;

	o = p->in.op;
	if( (optype(o) == LTYPE) ||
	    ( (p->in.type != INT) && (p->in.type != UNSIGNED) &&
	      !callop(o) ) )
		return;

	if( optype(o) == BITYPE )
		rt =  p->in.right->in.type;
	lt = p->in.left->in.type;

	switch(o)
		{
		case ASSIGN:
		case ASG MUL:
		case ASG DIV:
		case ASG MOD:
			if((rt == CHAR) || (rt == UCHAR))
				addsconv(p->in.right, p->in.type);
			break;

		case ASG AND:
		case ASG OR:
		case ASG ER:
		case ASG PLUS:
		case ASG MINUS:
			if( (lt != CHAR) && (lt != UCHAR) &&
			    ( (rt == CHAR) || (rt == UCHAR) ) )
				addsconv(p->in.right, p->in.type);
			break;

		case AND:
		case OR:
		case ER:
			if(p->in.left->in.op == ICON) {
				n = p->in.right;
				p->in.right = p->in.left;
				p->in.left = n;
				lt = rt;
				rt = p->in.right->in.type;
			}

			if( (p->in.op == AND) &&
			    (p->in.right->in.op == ICON) &&
			    (p->in.right->tn.lval == 255) &&
			    (p->in.right->tn.rval == NONAME) ) {
				p->in.right->in.op = FREE;
				o = p->in.op = SCONV;
				p->in.type = UCHAR;
				if( UNSIGNABLE(lt) )
					p->in.left->in.type = ENUNSIGN(lt);
				break;
			}

			if( (p->in.right->in.op == ICON) &&
			    (p->in.right->tn.rval == NONAME) ) {
				v = p->in.right->tn.lval;
				if( ( (lt == CHAR) &&
				      (v >= -128) && (v < 128) ) ||
				    ( (lt == UCHAR) &&
				      (v >= 0) && (v < 256) ) ) {
					p->in.type = lt;
					break;
				}
			}

		case PLUS:
		case MINUS:
		case MUL:
		case DIV:
		case MOD:
			if((rt == CHAR) || (rt == UCHAR))
				addsconv(p->in.right, p->in.type);

		case LS:
		case RS:
		case UNARY MINUS:
		case COMPL:
			if((lt == CHAR) || (lt == UCHAR))
				addsconv(p->in.left, p->in.type);

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
			if( (p->in.left->in.op == ICON) ||
			    (p->in.right->in.op == ICON) )
				break;

			if( ISUNSIGNED(lt) )
				lt = DEUNSIGN(lt);

			if( ISUNSIGNED(rt) )
				rt = DEUNSIGN(rt);

			if(rt == lt)
				break;

			if(lt == CHAR)
				addsconv( p->in.left,
					  (p->in.left->in.type == CHAR) ?
						INT : UNSIGNED);
			if(rt == CHAR)
				addsconv( p->in.right,
					  (p->in.right->in.type == CHAR) ?
						INT : UNSIGNED);
			break;

		case CALL:
		case STCALL:
			n = p->in.right;
			for(n = p->in.right; n->in.op == CM; n = n->in.left) {
				lt = n->in.right->in.type;
				if( (lt == CHAR) || (lt == UCHAR) ||
				    ( ( (lt == ENUMTY) || (lt == MOETY) ) &&
				      (dimtab[n->in.right->fn.csiz] == SZCHAR) ) )
					addsconv(n->in.right, INT);
			}
			lt = n->in.type;
			if( (lt == CHAR) || (lt == UCHAR) ||
			    ( ( (lt == ENUMTY) || (lt == MOETY) ) &&
			      (dimtab[n->fn.csiz] == SZCHAR) ) )
				addsconv(n, INT);

			break;
		}
}

 NODE *
fixptr(p)

register NODE *p;
{
	register NODE *l;
	register NODE *r;
	int lo, ro;
	CONSZ lv, rv;

	l = p->in.left;
	if( (l->in.op != PLUS) && (l->in.op != MINUS) )
		return(p);

	r = p->in.right;
	if( (r->tn.op == ICON) &&
	    (r->tn.rval == NONAME) ) {
		if( (l->in.right->in.op == ICON) &&
		    conval( l->in.right,
			    (p->in.op == l->in.op) ? PLUS : MINUS,
			    p->in.right ) ) {
			p->in.op = FREE;
			p->in.right->in.op = FREE;
			return(l);
		}
		return(p);
	}

	if(l->in.right->in.op == ICON) {
		p->in.left = l->in.left;
		l->in.left = fixptr(p);
		return(l);
	}

	p->in.left = l->in.right;
	l->in.type = p->in.type;
	l->fn.cdim = p->fn.cdim;
	l->fn.csiz = p->fn.csiz;
	p->in.type = INT;
	p->fn.cdim = 0;
	p->fn.csiz = INT;
	l->in.right = p;

        if (l->in.op == MINUS)	/* toggle sign of expression brought under - */
                if (p->in.op == MINUS)
                        p->in.op = PLUS;
                else if (p->in.op == PLUS)
                        p->in.op = MINUS;

	lo = p->in.left->in.op;
	ro = p->in.right->in.op;

	r = p->in.left;
	if(lo == MUL)
		r = r->in.right;
	if( (r->tn.op != ICON) || (r->tn.rval != NONAME) )
		return(l);
	lv = r->tn.lval;

	r = p->in.right;
	if(ro == MUL)
		r = r->in.right;
	if( (r->tn.op != ICON) || (r->tn.rval != NONAME) )
		return(l);
	rv = r->tn.lval;

	if( (lo == MUL) && ( (rv % lv) == 0) ) {
		r = p->in.right;
		if(ro == MUL)
			r = r->in.right;
		r->tn.lval /= lv;

		r = p->in.left;
		p->in.left = r->in.left;
		r->in.left = p;
		l->in.right = r;
		return(l);
	}

	if( (ro == MUL) && ( (lv % rv) == 0) ) {
		r = p->in.left;
		if(lo == MUL)
			r = r->in.right;
		r->tn.lval /= rv;

		r = p->in.right;
		p->in.right = r->in.left;
		r->in.left = p;
		l->in.right = r;
		return(l);
	}

	return(l);
}

andable( p ) NODE *p; {
	return(1);  /* all names can have & taken on them */
	}

cendarg(){ /* at the end of the arguments of a ftn, set the automatic offset */
	autooff = AUTOINIT;
	}

cisreg( t ) TWORD t; { /* is an auto variable of type t OK for a register variable */

	/* these types are allowed in registers:
	 * di - pointer or int
	 * cx    - int
	 */

	if(  t==INT || t==UNSIGNED ) return(1);
	if (ISPTR(t) && regvar == 4 ) return(1);
	return(0);
	}

NODE *
offcon( off, t, d, s ) TWORD t; OFFSZ off; {

	/* return a node, for structure references, which is suitable for
	 * being added to a pointer of type t, in order to be off bits offset
	 * into a structure
	 * t, d, and s are the type, dimension offset, and sizeoffset
	 */

	register NODE *p;


	p = bcon(0);
	p->tn.lval = off/SZCHAR;
	return(p);

	}

static inwd;	/* current bit offsed in word */
static word;	/* word being built from fields */

incode( p, sz ) register NODE *p; {

	/* generate initialization code for assigning a constant c
	 * to a field of width sz
	 * we assume that the proper alignment has been obtained
	 * inoff is updated to have the proper final value
	 * we also assume sz  < SZINT
	 */

	if((sz+inwd) > SZINT) cerror("incode: field > int");
	word |= (p->tn.lval & ((1 << sz) - 1)) << inwd;
	inwd += sz;
	inoff += sz;
	if(inoff%SZINT == 0) {
		fprintf( outfile, "\t.value\t0%o\n", word);
		word = inwd = 0;
		}
	}

fincode( d, sz ) double d; {
	/* initialize floating point */

	/* output code to initialize space of size sz to the value d
	 * inoff is updated to have the proper final value
	 */

	fprintf(outfile, "\t.%s\t%.20g\n",
		(sz == SZDOUBLE) ? "double" : "float", d);
	inoff += sz;
	}

cinit( p, sz ) NODE *p; {
	/* arrange for the initialization of p into a space of
	 * size sz
	 * the proper alignment has been obtained
	 * inoff is updated to have the proper final value
	 */

	ecode( p );
	inoff += sz;
	}

vfdzero( n ){ /* define n bits of zeros in a vfd */

	if( n <= 0 ) return;

	inwd += n;
	inoff += n;
	if( inoff%SZINT ==0 ) {
		fprintf( outfile,  "\t.value\t0%o\n", word );
		word = inwd = 0;
		}
	}


inend() {

	if(inwd == SZCHAR) {
		fprintf( outfile,  "\t.byte 0%o\n", word );
		word = inwd = 0;
	}
}


char *
exname(p ) register char *p; {
	/* make a name look like an external name in the local machine
	 * external names should look the same as locals
	 */
# ifndef FLEXNAMES
	static char ename[NCHNAM + 1];
	register int i;
	register char *t = ename;

	for (i = 0; i < NCHNAM && ( *t++ = *p++ ); i++);

	return( ename );
# else
	return ( p );
# endif

	}

/* TWORD	I wish pcc new this was the return value */
ctype( type ) register TWORD type;
{
	/* map types which are not defined on the local machine */

	switch( BTYPE(type) ){
	case SHORT:
		MODTYPE(type,INT);
		break;
	case USHORT:
		MODTYPE(type,UNSIGNED);
		break;
		}
	return( type );
	}

noinit()
{
	/* curid is a variable which is defined but
	 * is not initialized (and not a function );
	 * this routine returns the storage class
	 * for an uninitialized declaration
	 */

	return(EXTERN);

	}

commdec( id )
{
	/* make a common declaration for id, if reasonable */
	register struct symtab *q;
	OFFSZ tsize();
	register OFFSZ off;
	register int saveloc;

	q = &stab[id];
	if (q->sclass == EXTDEF) {
# ifdef FLEXNAMES
		werror("redeclaration of %s", q->sname);
# else
		werror("redeclaration of %.8s", q->sname);
# endif
		return;
		}

	/* print debugging code for the symbol */
	/* make sure the .def appears in .data section */
	saveloc = locctr( DATA );
	extdef( q );
	off = tsize( q->stype, q->dimoff, q->sizoff ) / SZCHAR;
	if(q->sclass != STATIC)
# ifdef FLEXNAMES
		fprintf( outfile, "\t.comm\t%s,%ld\n", exname(q->sname), off );
# else
		fprintf( outfile, "\t.comm\t%.8s,%ld\n", exname(q->sname), off );
# endif
	else {
		/* now change to .bss and put out the symbol */
		locctr( BSS );
		defalign( talign( q->stype, q->sizoff ) );
		if( q->slevel>1 )
			deflab( (int)q->offset );
		else
# ifdef FLEXNAMES
			fprintf( outfile, "%s:", exname(q->sname));
# else
			fprintf( outfile, "%.8s:", exname(q->sname) );
# endif
		fprintf( outfile, "\t.set\t.,.+%ld\n", off );
	}
	/* and return to the original location counter */
	locctr( saveloc );
	if(q->sclass != STATIC) q->sclass = EXTDEF;
}

/* is lastcon to be long or short */
/* cb is the first character of the representation, ce the last */
/* This is apparently not used
 * isitlong( cb, ce )
 * {
 * 
 * 	if( ce == 'l' || ce == 'L' ||
 * 		lastcon >= (1L << (SZINT-1) ) )
 * 			return( 1 );
 * 	return(0);
 * 	}
 */

isitfloat( s ) char *s; {
	double atof();
	extern int fpe_flag, errno;

	fpe_flag = errno = 0;
	dcon = atof(s);
	if( fpe_flag || errno ) {
	    if( errno )
		werror( "floating point constant too large" );
	    dcon = HUGE;
	    fpe_flag = 0;
	}
	return( FCON );
	}

ecode( p ) NODE *p; {

	/* walk the tree and write out the nodes.. */

	if( nerrors ) return;

	/* generate a new line number breakpoint if
	 * the line number has not changed.
	 */
	if( !dlflag && lineno != oldln ) {
		oldln = lineno;
		if( lastloc == PROG && !strcmp(startfn,ftitle) )
			fprintf( outfile, "\t.ln\t%d\n", LINENO );
	}

	/* output the tree to intermediate text */
#ifdef ONEPASS
	p2tree(p);
	p2compile(p);
#else
	printf("%c%d\t%s\n", EXPR, lineno, ftitle);
	prtree(p);
#endif
	}

fixdef( p ) register struct symtab *p;
{
	if( ISARY(p->stype) )
		arraychk(p);

	/* general .def printing routine
	 * don't do params, externs, structure names or members
	 * at this time.  they are delayed until more informataion
	 * is known about them
	 */
	switch(p->sclass) {
	case EXTERN:
	case EXTDEF:
	case STNAME:
	case UNAME:
	case ENAME:
	case MOS:
	case MOE:
	case MOU:
	case FIELD:
	case USTATIC:

		return;
	}

	/* parameters */
	if( p->slevel == 1 ) return;

	/* static externs */
	if( p->sclass == STATIC && p->slevel == 0 ) return;

	/* fields */
	if (p->sclass & FIELD) return;

	prdef( p );
}



prdims( p )
struct symtab *p;
{
	/* print debugging info for dimensions */

	register int temp, dtemp, dflag;
	OFFSZ tsize();
	dflag = 0; /* need to print .dim */
	dtemp = p->dimoff;

	for( temp=p->stype; temp&TMASK; temp = DECREF(temp) ) {
		/* put out a dimension for each instance of ARY in type word */

		if( ISARY(temp) ) {
			if( !dflag )
				fprintf( outfile, " .dim %ld", dimtab[dtemp++] );
			else
				fprintf( outfile, ", %ld", dimtab[dtemp++] );
			if (++dflag == 4) break;
		}
	}

	if( dflag ) {
		fprintf( outfile, "; .size " );
		fprintf( outfile, CONFMT, tsize(p->stype, p->dimoff, p->sizoff ) / SZCHAR  );
		fprintf( outfile, ";" );
	}
}

extdef( p )
struct symtab *p;
{
	/* print debugging code for external definitions */
	if( p->slevel == 0 )/* make sure it is external */
		prdef( p );
}



char tagname[10] = "";
#define FAKENM 99	/* maximum number of fakenames */
/* local table of fakes for un-names structures
 * sizoff for .ifake is stored in mystrtab[i]
 */
static int mystrtab[FAKENM], myfake = 0;
struct symtab mytable;

prdef( p )
struct symtab *p;
{
	/* this routine does all the printing of the actual .def pseudos */
	TWORD pty, bpty, npty, tpty, rpty;
	char *strname(), savech, *tagnm;
	register char stclass;
	register int saveloc;
	register int outcls;

	/* make some local modifications to storage classes */
	stclass = outcls = p->sclass;
	if (stclass == EXTDEF) outcls = C_EXT;
	if (stclass & FIELD)
		stclass = FIELD;

	if (stclass == USTATIC) outcls = C_USTATIC;
	if (stclass == FIELD)   outcls = C_FIELD;
	if (stclass == LABEL || stclass == ULABEL) outcls = C_LABEL;
	if (stclass == REGISTER && blevel == 1) outcls = C_REGPARM;

	if(dsflag <= 0) {
		/* print a bb symbol if this is the first symbol in the block */

		if( blevel > 2 && !bb_flags[blevel] && outcls != C_LABEL ) {
			fprintf( outfile, "\t.def\t.bb; .val .; .scl %d; .type 0x00; .line %d; .endef\n", C_BLOCK, LINENO );
			bb_flags[blevel] = 1;   /* keep another bb from printing */
		}

		/* make sure that .defs in functions are in .text */
		if( blevel > 1 )
			saveloc = locctr( PROG );

		if(p->sname)
# ifdef FLEXNAMES
			fprintf( outfile, "\t.def\t%s;", p->sname );
# else
			fprintf( outfile, "\t.def\t%.8s;", p->sname );
# endif
		else
			fprintf( outfile, "\t.def\t;");


		switch( stclass ) {
		/* print .val based on storage class */

		case AUTO:
		case MOS:
		case MOU:
		case PARAM:
			/* offset in bytes */
			fprintf( outfile, " .val %ld;", p->offset/SZCHAR );
			break;
		case FIELD:
		case MOE:
			/* offset in bits, or internal value of enum symbol */
			fprintf( outfile, " .val %ld;", p->offset );
			break;
		case REGISTER:
			/* offset in bytes in savearea for reg vars */
			/* DI = 2, CX = 4 */
			fprintf( outfile, " .val %d;", p->offset == 5 ? 2 : 4);
			break;
		case STATIC:
		case LABEL:
		case ULABEL:
		case EXTDEF:
		case EXTERN:
			/* actual or hidden name, depending on scope */
			if( p->slevel >0 )
				fprintf( outfile, " .val .%ld;", p->offset );
			else
# ifdef FLEXNAMES
				fprintf( outfile, " .val %s;", p->sname );
# else
				fprintf( outfile, " .val %.8s;", p->sname );
# endif
			break;
		default:
			break;
		}

		pty = p->stype;
		bpty = BTYPE(pty);
		fprintf( outfile, " .scl %d;", outcls );
		if (stclass == LABEL)
			fprintf(outfile," .type 0x00;");
		else {
			npty = 1; rpty = 0; 
			for (tpty=pty&~BTMASK; tpty&TMASK; tpty>>=TSHIFT) {
				if(ISPTR(tpty)) rpty+=npty*01;
				else if(ISARY(tpty)) rpty+=npty*03;
				else if(ISFTN(tpty)) rpty+=npty*02;
				npty <<= 2;
			}
			rpty = (rpty<<4) + bpty;
			fprintf(outfile," .type 0%o;", rpty);
		}

		/* print tag and size info, size only for tags themselves */
		/* don't print if size not known */
		if( dimtab[p->sizoff] > 0 )
			switch( stclass ) {
			case STNAME:
			case UNAME:
			case ENAME:
				fprintf( outfile, " .size %ld;", dimtab[p->sizoff] / SZCHAR );
				break;
			default:	/* bpty is base type */
				if( !dsflag &&
				    ( (bpty == STRTY) ||
				      (bpty == UNIONTY) ||
				      (bpty == ENUMTY) ) ) {
					tagnm = strname(p->sizoff);
					savech = *tagnm;
					if( savech == '$' )
						*tagnm = '_';
# ifdef FLEXNAMES
					fprintf( outfile, " .tag %s;", tagnm );
# else
					fprintf( outfile, " .tag %.8s;", tagnm );
# endif
					*tagnm = savech;
					fprintf( outfile, " .size %ld;", dimtab[p->sizoff] / SZCHAR );
				}
				break;
			}

		/* print line for block symbols */
		if( p->slevel > 2 )
			fprintf( outfile, " .line %d;", LINENO );

		/* look for dimensions to print */
		if( stclass != LABEL && stclass != ULABEL )
			prdims( p );

		/* size of field is its length in bits */
		if(stclass == FIELD )
			fprintf( outfile, " .size %d;", p->sclass&FLDSIZ );
		fprintf( outfile, " .endef\n" );
		if( blevel > 1 )
			locctr( saveloc );
	}
}


strend( dimst )
int dimst;
{
	/* called at the end of a structure declaration
	 * this routine puts out the structure tag, its members
	 * and an eos.  dimst is the index in dimtab of the
	 * structure description
	 */
	register int member, saveloc;
	OFFSZ size;
	char savech;
	struct symtab *memptr, *tagnm, *strfind();
# ifdef FLEXNAMES
	char *buff;
# endif

	if( dsflag ) return;

	/* set locctr to text */
	saveloc = locctr( PROG );

	/* set up tagname */
	member = dimtab[dimst + 1];
	tagnm = strfind(dimst);

	if( tagnm == 0 ) {
		/* create a fake if there is no tagname */
		/* use the local symbol table */
		tagnm = &mytable;
		if( myfake == FAKENM )
			cerror( "fakename table overflow" );

		/* generate the fake name and enter into the fake table */
# ifdef FLEXNAMES
		sprintf(mytable.sname = (char *)malloc(24), ".%dfake", myfake );
# else
		sprintf( mytable.sname, ".%dfake", myfake );
# endif
		mystrtab[myfake++] = dimst;
		memptr = &stab[dimtab[member]];

		/* fix up the fake's class, type, and sizoff based on class of its members */
		switch( memptr->sclass ) {
		case MOS:
		case FIELD:
			tagnm->sclass = STNAME;
			tagnm->stype = STRTY;
			break;
		case MOE:
			tagnm->sclass = ENAME;
			tagnm->stype = ENUMTY;
			break;
		case MOU:
			tagnm->sclass = UNAME;
			tagnm->stype = UNIONTY;
			break;
		default:
			if (memptr->sclass & FIELD) {
				tagnm->sclass = STNAME;
				tagnm->stype = STRTY;
			} else
				cerror( "can't identify type of fake tagname" );
		}
		tagnm->slevel = 0;;
		tagnm->sizoff = dimst;
	}

	/* print out the structure header */
	savech = tagnm->sname[0];
	if( savech == '$' )
		tagnm->sname[0] = '_';
	prdef( tagnm );
	tagnm->sname[0] = savech;

	/* print out members */
	while( dimtab[member] >= 0 ) {
		memptr = &stab[dimtab[member++]];
		savech = memptr->sname[0];
		if( savech == '$' )
			memptr->sname[0] = '_';
		prdef( memptr );
		memptr->sname[0] = savech;
	}

	/* print eos */
	size = dimtab[dimst] / SZCHAR;
	fprintf( outfile, "\t.def\t.eos; .val %ld; .scl %d;", size, C_EOS );
# ifdef FLEXNAMES
	fprintf( outfile, " .tag %s; .size %ld; .endef\n", tagnm->sname, size );
# else
	fprintf( outfile, " .tag %.8s; .size %ld; .endef\n", tagnm->sname, size );
# endif

	/* return to old locctr */
	locctr( saveloc );
}

struct symtab *
strfind( key )
int key;
{
	/* find the structure tag in the symbol table, 0 == not found */
	struct symtab *sptr;
	char spc;
	for( sptr = stab; sptr < stab + SYMTSZ; ++sptr ) {
		spc = sptr->sclass;
		if( (spc == STNAME || spc == ENAME || spc == UNAME ) && sptr->sizoff == key ) {
			return( sptr );
		}
		}
	/* not found */
	return( 0 );
}

char *
strname( key )
int key;
{
	/* return a pointer to the tagname,
	 * the fake table is used if not found by strfind
	 */
	register int i;
	struct symtab *tagnm, *strfind();
	tagnm = strfind( key );
	if( tagnm != 0 )
		return( tagnm->sname );

	for( i = 0; i < FAKENM; ++i )
		if( mystrtab[i] == key ) {
			sprintf( tagname, ".%dfake", i );
			return( tagname );
		}

	cerror( "structure tagname not found" );
	return(NULL);
}

