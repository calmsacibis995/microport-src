/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)allo.c	1.4 - 85/08/09 */

# include "mfile2"

NODE resc[3];

int busy[REGSZ];
long stcount = 0;

int maxa, mina, maxb, minb;

# ifndef ALLO0
allo0(){ /* free everything */

	register i;

	maxa = maxb = -1;
	mina = minb = 0;

	REGLOOP(i){
		busy[i] = 0;
		if( rstatus[i] & STAREG ){
			if( maxa<0 ) mina = i;
			maxa = i;
			}
		if( rstatus[i] & STBREG ){
			if( maxb<0 ) minb = i;
			maxb = i;
			}
		}
	}
# endif

# define TBUSY 01000

# ifndef ALLO
allo( p, q ) NODE *p; struct optab *q; {

	register n, i, j;
	int either;

	n = q->needs;
	either = ( EITHER & n );
	i = 0;

	while( n & NACOUNT ){
		resc[i].in.op = REG;
		resc[i].tn.rval = freereg( p, n&NAMASK );
		resc[i].tn.lval =
		    resc[i].tn.rval >= ST0 ? ++stcount : 0;
#ifdef FLEXNAMES
		resc[i].in.name = "";
#else
		resc[i].in.name[0] = '\0';
#endif
		n -= NAREG;
		++i;
		}

	if (either) { /* all or nothing at all */
		for( j = 0; j < i; j++ )
			if( resc[j].tn.rval < 0 ) { /* nothing */
				i = 0;
				break;
				}
		if( i != 0 ) goto ok; /* all */
		}

	while( n & NBCOUNT ){
		resc[i].in.op = REG;
		resc[i].tn.rval = freereg( p, n&NBMASK );
		resc[i].tn.lval = 0;
#ifdef FLEXNAMES
		resc[i].in.name = "";
#else
		resc[i].in.name[0] = '\0';
#endif
		n -= NBREG;
		++i;
		}
	if (either) { /* all or nothing at all */
		for( j = 0; j < i; j++ )
			if( resc[j].tn.rval < 0 ) { /* nothing */
				i = 0;
				break;
				}
		if( i != 0 ) goto ok; /* all */
		}

	if( n & NTMASK ){
		resc[i].in.op = OREG;
		resc[i].tn.rval = TMPREG;
		if( p->in.op == STCALL || p->in.op == STARG || p->in.op == UNARY STCALL || p->in.op == STASG ){
			resc[i].tn.lval = freetemp( (SZCHAR*p->stn.stsize + (SZINT-1))/SZINT );
			}
		else {
			resc[i].tn.lval = freetemp( (n&NTMASK)/NTEMP );
			}
#ifdef FLEXNAMES
		resc[i].in.name = "";
#else
#	ifdef FONEPASS
		resc[i].in.name[1] = '\1';
		resc[i].in.name[0] = '\0';
#	else
		resc[i].in.name[0] = '\0';
#	endif /* FONEPASS */
#endif /* FLEXNAMES */
		resc[i].tn.lval = BITOOR(resc[i].tn.lval);
		++i;
		}

	/* turn off "temporarily busy" bit */

	ok:
	REGLOOP(j){
		busy[j] &= ~TBUSY;
		}

	for( j=0; j<i; ++j ) if( resc[j].tn.rval < 0 ) return(0);
	return(1);

	}
# endif

freetemp( k ){ /* allocate k characters worth of temp space */
	/* we also make the convention that, if the number of words is more than 1,
	/* it must be aligned for storing doubles... */

# ifndef BACKTEMP
	int t;

	if( k>2 ){
		SETOFF( tmpoff, ALDOUBLE );
		}

	t = tmpoff;
	tmpoff += k*SZINT;
	if( tmpoff > maxoff ) maxoff = tmpoff;
	if( tmpoff > MAXAUTO )
		cerror( "stack overflow" );
	if( tmpoff-baseoff > maxtemp ) maxtemp = tmpoff-baseoff;
	return(t);

# else
	tmpoff += k*SZINT;
	if( k>2 ) {
		SETOFF( tmpoff, ALDOUBLE );
		}
	if( tmpoff > maxoff ) maxoff = tmpoff;
#ifndef FORT
	/* these 2 lines blow f77 on the VAX */
	if( tmpoff > MAXAUTO )
		cerror( "stack overflow" );
#endif
	if( tmpoff-baseoff > maxtemp ) maxtemp = tmpoff-baseoff;
	return( -tmpoff );
# endif
	}

# ifndef FREEREG
freereg( p, n ) NODE *p; {
	/* allocate a register of type n */
	/* p gives the type, if floating */

	register j;

	/* not general; means that only one register (the result) OK for call */
	if( callop(p->in.op) ){
		j = callreg(p);
		if( usable( p, n, j ) ) return( j );
		/* have allocated callreg first */
		}
	j = p->in.rall & ~(MUSTDO|NEEDREGS);
	if( j!=NOPREF && usable(p,n,j) ){ /* needed and not allocated */
		return( j );
		}
	if( n&NAMASK ){

/* woa 2 -> */
		if(p->in.rall & NEEDREGS) {
			for(j = DX + 1; j <= maxa; ++j)
				if(rstatus[j] & STAREG) {
					if( usable(p, n, j) ) {
						return(j);
					}
				}
		}
/* woa 2 <- */

		for( j=mina; j<=maxa; ++j ) if( rstatus[j]&STAREG ){
			if( usable(p,n,j) ){
				return( j );
				}
			}
		}
	else if( n &NBMASK ){
		for( j=minb; j<=maxb; ++j ) if( rstatus[j]&STBREG ){
			if( usable(p,n,j) ){
				return(j);
				}
			}
		}

	return( -1 );
	}
# endif

# ifndef USABLE
usable( p, n, r ) NODE *p; {
	/* decide if register r is usable in tree p to satisfy need n */

	/* checks, for the moment */
	if( !istreg(r) ) cerror( "usable asked about nontemp register" );

	if( busy[r] > 1 ) return(0);
	if( isbreg(r) ){
		if( n&NAMASK ) return(0);
		}
	else {
		if( n & NBMASK ) return(0);
		}
	if( (n&NAMASK) && (szty(p->in.type) == 2) ){ /* only do the pairing for real regs */
		if( r&01 ) return(0);
		if( !istreg(r+1) ) return( 0 );
		if( busy[r+1] > 1 ) return( 0 );
		if( busy[r] == 0 && busy[r+1] == 0  ||
		    busy[r+1] == 0 && shareit( p, r, n ) ||
		    busy[r] == 0 && shareit( p, r+1, n ) ){
			busy[r] |= TBUSY;
			busy[r+1] |= TBUSY;
			return(1);
			}
		else return(0);
		}
	if( (n&NBMASK) && (szty(p->in.type) == 2) ){
		if( r&01 ) return(0);
		if( !istreg(r+1) ) return( 0 );
		if( busy[r+1] > 1 ) return( 0 );
		if( busy[r] == 0 && busy[r+1] == 0  ||
		    busy[r+1] == 0 && shareit( p, r, n ) ||
		    busy[r] == 0 && shareit( p, r+1, n ) ){
			busy[r] |= TBUSY;
			busy[r+1] |= TBUSY;
			return(1);
			}
		else return(0);
	}
	if( busy[r] == 0 ) {
		busy[r] |= TBUSY;
		return(1);
		}

	/* busy[r] is 1: is there chance for sharing */
	return( shareit( p, r, n ) );

	}
# endif

shareit( p, r, n ) NODE *p; {
	/* can we make register r available by sharing from p
	   given that the need is n */
	if( (n&(NASL|NBSL)) && ushare( p, 'L', r ) ) return(1);
	if( (n&(NASR|NBSR)) && ushare( p, 'R', r ) ) return(1);
	return(0);
	}

# ifndef USHARE
ushare( p, f, r ) NODE *p; {
	/* can we find a register r to share on the left or right
		(as f=='L' or 'R', respectively) of p */
	p = getlr( p, f );
	if( p->in.op == UNARY MUL ) p = p->in.left;
	if( p->in.op == OREG ){
		if( R2TEST(p->tn.rval) ){
			return( r==R2UPK1(p->tn.rval) || r==R2UPK2(p->tn.rval) );
			}
		else return( r == p->tn.rval );
		}
	if( p->in.op == REG ){
		return( r == p->tn.rval || ( szty(p->in.type) == 2 && r==p->tn.rval+1 ) );
		}
	return(0);
	}
# endif

recl2( p ) register NODE *p; {
	register r = p->tn.rval;
	if( p->in.op == REG ) rfree( r, p->in.type );
	else if( p->in.op == OREG ) {
		if( R2TEST( r ) ) {
			if( R2UPK1( r ) != R2NOBASE )
				rfree( R2UPK1( r ), PTR+INT );
			rfree( R2UPK2( r ), INT );
			}
		else {
			rfree( r, PTR+INT );
			}
		}
# ifdef SSPLIT
	if( (model >= MDL_LARGE) &&
	    ( (p->in.op == NAME) || (p->in.op == OREG) ) &&
	    (p->in.segment >= SS) )
		rfree( p->in.segment, INT );
# endif
	}

int rdebug = 0;

# ifndef RFREE
rfree( r, t ) TWORD t; {
	/* mark register r free, if it is legal to do so */
	/* t is the type */

# ifndef BUG3
	if( rdebug ){
		printf( "rfree( %s ), size %d\n", rnames[r], szty(t) );
		}
# endif

	if( istreg(r) ){
		if( --busy[r] < 0 ) cerror( "register overfreed");
		if( szty(t) == 2 && r!=STKREG){
/* following code does not test correctly because of
 dx and ci being a register pair for long pointers
		*/
#ifdef FONEPASS
			/*if(  (istreg(r)^istreg(r+1)) ) cerror( "illegal free" );*/
#else
			/*if( (r&01) || (istreg(r)^istreg(r+1)) ) cerror( "illegal free" );*/
#endif /* FONEPASS */
			if( --busy[r+1] < 0 ) cerror( "register overfreed" );
			}
		}
	}
# endif

# ifndef RBUSY
rbusy(r,t) TWORD t; {
	/* mark register r busy */
	/* t is the type */

# ifndef BUG3
	if( rdebug ){
		printf( "rbusy( %s ), size %d\n", rnames[r], szty(t) );
		}
# endif /* BUG3 */

	if( istreg(r) ) ++busy[r];
	if( szty(t) == 2 && r != STKREG) {
		if( istreg(r+1) ) ++busy[r+1];
#ifdef FONEPASS
		else if( (istreg(r)^istreg(r+1)) ) cerror( "illegal register pair freed" );
#else
		/* The commented test became invalid when register
		   doubles were added to the 3b.  Not clear why. */
		else if( (r&01) || (istreg(r)^istreg(r+1)) )
			cerror( "illegal register pair freed" );
# endif /* FONEPASS */
		}
	}
#endif  /* RBUSY */

# ifndef BUG3
rwprint( rw ){ /* print rewriting rule */
	register i, flag;
	static char * rwnames[] = {

		"RLEFT",
		"RRIGHT",
		"RESC1",
		"RESC2",
		"RESC3",
		0,
		};

	if( rw == RNULL ){
		printf( "RNULL" );
		return;
		}

	if( rw == RNOP ){
		printf( "RNOP" );
		return;
		}

	flag = 0;
	for( i=0; rwnames[i]; ++i ){
		if( rw & (1<<i) ){
			if( flag ) printf( "|" );
			++flag;
			printf( rwnames[i] );
			}
		}
	}
# endif

reclaim( p, rw, cookie ) NODE *p; {
	register NODE **qq;
	register NODE *q;
	register i;
	NODE *recres[5];
	struct respref *r;

	/* get back stuff */

# ifndef BUG3
	if( rdebug ){
		printf( "reclaim( %o, ", p );
		rwprint( rw );
		printf( ", " );
		prcook( cookie );
		printf( " )\n" );
		}
# endif

	if( rw == RNOP || ( p->in.op==FREE && rw==RNULL ) ) return;  /* do nothing */

	walkf( p, recl2 );

	if( callop(p->in.op) ){
		/* check that all scratch regs are free */
		callchk(p);  /* ordinarily, this is the same as allchk() */
		}

	if( rw == RNULL || (cookie&FOREFF) ){ /* totally clobber, leaving nothing */
		tfree(p);
		return;
		}

	/* handle condition codes specially */

	if( (cookie & FORCC) && (rw&RESCC)) {
		/* result is CC register */
		tfree(p);
		p->in.op = CCODES;
		p->tn.lval = 0;
		p->tn.rval = 0;
		return;
		}

	/* locate results */

	qq = recres;

	if( rw&RLEFT) *qq++ = getlr( p, 'L' );
	if( rw&RRIGHT ) *qq++ = getlr( p, 'R' );
	if( rw&RESC1 ) *qq++ = &resc[0];
	if( rw&RESC2 ) *qq++ = &resc[1];
	if( rw&RESC3 ) *qq++ = &resc[2];

	if( qq == recres ){
		cerror( "illegal reclaim");
		}

	*qq = NIL;

	/* now, select the best result, based on the cookie */

	for( r=respref; r->cform; ++r ){
		if( cookie & r->cform ){
			for( qq=recres; (q= *qq) != NIL; ++qq ){
				if( tshape( q, r->mform ) ) goto gotit;
				}
			}
		}

	/* we can't do it; die */
	cerror( "cannot reclaim");

	gotit:

	if( p->in.op == STARG ) p = p->in.left;  /* STARGs are still STARGS */

	q->in.type = p->in.type;  /* to make multi-register allocations work */
		/* maybe there is a better way! */
	q = tcopy(q);

	if( q->tn.op == REG && q->tn.rval >= ST0 ) {
	    switch( optype( p->in.op ) ) {
	    case BITYPE:
		if( p->in.right->in.type == FLOAT ||
		    p->in.right->in.type == DOUBLE  ) {
			q->tn.lval =
			    p->in.right->tn.lval ?
				p->in.right->tn.lval : ++stcount;
			break;
		}
	    case UTYPE:
		if( p->in.left->in.type == FLOAT ||
		    p->in.left->in.type == DOUBLE  ) {
			q->tn.lval =
			    p->in.left->tn.lval ?
				p->in.left->tn.lval : ++stcount;
		} else {
			q->tn.lval = ++stcount;
		}
		break;
	    case LTYPE:
		q->tn.lval = p->tn.lval ? p->tn.lval : ++stcount;
		break;
	    }
	}

	tfree(p);

	p->in.op = q->in.op;
	p->tn.lval = q->tn.lval;
	p->tn.rval = q->tn.rval;
	p->in.segment = q->in.segment;
#ifdef FLEXNAMES
	p->in.name = q->in.name;
#ifdef ONEPASS				/* is this REALLY needed?? */
	p->in.stalign = q->in.stalign;
#endif
#else
	for( i=0; i<NCHNAM; ++i )
		p->in.name[i] = q->in.name[i];
#endif

	q->in.op = FREE;

	/* if the thing is in a register, adjust the type */

	switch( p->in.op ){

	case REG:
		if( !rtyflg ){
			/* the C language requires intermediate results to change type */
			/* this is inefficient or impossible on some machines */
			/* the "T" command in match supresses this type changing */
			if( p->in.type == CHAR || p->in.type == SHORT ) p->in.type = INT;
			else if( p->in.type == UCHAR || p->in.type == USHORT ) p->in.type = UNSIGNED;
			else if( p->in.type == FLOAT ) p->in.type = DOUBLE;
			}

/* woa 3 -> */
		if(p->in.rall & NEEDREGS) {
			if((p->tn.rval == AX) || (p->tn.rval == DX)) {
				p->in.rall = NOPREF;
				rbusy(AX, INT);
				rbusy(DX, INT);
				if( (i = freereg(p, NAREG)) < 0 )
					i = freereg(p, NBREG);
				busy[i] &= ~TBUSY;
				rfree(AX, INT);
				rfree(DX, INT);
				rbusy(i, p->in.type);
				rfree(p->tn.rval, p->in.type);
				rmove(i, p->tn.rval, p->tn.type);
				p->tn.rval = i;
			}
			return;
		}
/* woa 3 <- */

		if( ! (p->in.rall & MUSTDO ) ) return;  /* unless necessary, ignore it */
		i = p->in.rall & ~MUSTDO;
		if( i & NOPREF ) return;
		if( i != p->tn.rval ){
			if( busy[i] || ( szty(p->in.type)==2 && busy[i+1] ) ){
				cerror( "faulty register move" );
				}
			rbusy( i, p->in.type );
			rfree( p->tn.rval, p->in.type );
			rmove( i, p->tn.rval, p->in.type );
			p->tn.rval = i;
			}

	case OREG:
		if( R2TEST(p->tn.rval) ){
			int r1, r2;
			r1 = R2UPK1(p->tn.rval);
			r2 = R2UPK2(p->tn.rval);
			if( r1 != R2NOBASE )
				if( (busy[r1]>1 && istreg(r1)) || (busy[r2]>1 && istreg(r2)) ){
					cerror( "potential register overwrite" );
					}
			}
		else if( (busy[p->tn.rval]>1) && istreg(p->tn.rval) ) cerror( "potential register overwrite");

		}

	}

NODE *
tcopy( p ) register NODE *p; {
	/* make a fresh copy of p */

	register NODE *q;
	register r;

	q = talloc();
	*q = *p;

	r = p->tn.rval;
	if( p->in.op == REG ) rbusy( r, p->in.type );
	else if( p->in.op == OREG ) {
		if( R2TEST(r) ){
			if( R2UPK1(r) != R2NOBASE ) rbusy( R2UPK1(r), PTR+INT );
			rbusy( R2UPK2(r), INT );
			}
		else {
			rbusy( r, PTR+INT );
			}
		}
# ifdef SSPLIT
	if( (model >= MDL_LARGE) &&
	    ( (p->in.op == NAME) || (p->in.op == OREG) ) &&
	    (p->in.segment >= SS) )
		rbusy( p->in.segment, INT );
# endif

	switch( optype(q->in.op) ){

	case BITYPE:
		q->in.right = tcopy(p->in.right);
	case UTYPE:
		q->in.left = tcopy(p->in.left);
		}

	return(q);
	}

allchk(){
	/* check to ensure that all register are free */

	register i;

	REGLOOP(i){
		if( istreg(i) && busy[i] ){
			cerror( "register allocation error");
			}
		}

	if(fdepth)
		cerror( "float stack not freed - get help" );
	}
