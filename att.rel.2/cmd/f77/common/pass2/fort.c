/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/*	iAPX286 @(#)fort.c	1.4 85/09/06 */

/* this forces larger trees, etc. */

# include "mfile2"
# include "fort.h"

/*	masks for unpacking longs */

# ifndef FOP
# define FOP(x) (int)((x)&0377)
# endif

# ifndef VAL
# define VAL(x) (int)(((x)>>8)&0377)
# endif

# ifndef REST
# define REST(x) (((x)>>16)&0177777)
# endif

extern int model;
short debugflag = 0;

FILE * lrd;  /* for default reading routines */
# ifndef NOLREAD
long lread(){
	static long x;
	if( fread( (char *) &x, 4, 1, lrd ) <= 0 ) cerror( "intermediate file read error" );
	return( x );
	}
# endif
#if TARGET == IAPX286
extern	tranfrm();
#endif

# ifndef NOLOPEN
lopen( s ) char *s; {
	/* if null, opens the standard input */
	if( *s ){
		lrd = fopen( s, "r" );
		if( lrd == NULL ) cerror( "cannot open intermediate file %s", s );
		}
	else  lrd = stdin;
	}
# endif

# ifndef NOLCREAD
lcread( cp, n ) char *cp; {
	if( n > 0 ){
		if( fread( cp, 4, n, lrd ) != n ) cerror( "intermediate file read error" );
		}
	}
# endif

# ifndef NOLCCOPY
lccopy( n ) register n; {
	register i;
	static char fbuf[128];
	if( n > 0 ){
		if( n > 32 ) cerror( "lccopy asked to copy too much" );
		if( fread( fbuf, 4, n, lrd ) != n ) cerror( "intermediate file read error" );
		for( i=4*n; fbuf[i-1] == '\0' && i>0; --i ) { /* VOID */ }
		if( i ) {
			if( fwrite( fbuf, 1, i, stdout ) != i ) cerror( "output file error" );
			}
		}
	}
# endif

#if TARGET == IAPX286
long
rdin( base ){
	register sign, c;
	long val;

	sign = 1;
	val = 0;

	while( (c=getchar()) > 0 ) {
		if( c == '-' ){
			if( val != 0 ) cerror( "illegal -");
			sign = -sign;
			continue;
			}
		if( c == '\t' ) break;
		if( c>='0' && c<='9' ) {
			val *= base;
			if( sign > 0 )
				val += c-'0';
			else
				val -= c-'0';
			continue;
			}
		cerror( "illegal character `%c' on intermediate file", c );
		break;
		}

	if( c <= 0 ) {
		cerror( "unexpected EOF");
		}
	return( val );
	}

extern int ftnno;
extern long maxoff;
int maxargs;
feoblk()
{
	long spoff;	
	spoff = maxoff/8;
	if( (spoff%4) != 0 ) spoff =  (spoff/4 + 1) * 4;
	printf("\t.set\t.S%d,%ld+.SSP%d\n", ftnno, spoff, ftnno);
	maxargs = -1;
}
#endif

/*	new opcode definitions */

# define FORTOPS 200
# define FTEXT 200
# define FEXPR 201
# define FSWITCH 202
# define FLBRAC 203
# define FRBRAC 204
# define FEOF 205
# define FARIF 206
# define LABEL 207

/*	stack for reading nodes in postfix form */

# define NSTACKSZ 250

NODE * fstack[NSTACKSZ];
NODE ** fsp;  /* points to next free position on the stack */
#if TARGET != IAPX286
unsigned int caloff();
extern unsigned int offsz;
#endif
mainp2( argc, argv ) char *argv[]; {
	int files;
	register long x;
	register NODE *p;
#if TARGET != IAPX286
	offsz = caloff();
#endif
	files = p2init( argc, argv );
	tinit();

		
	if( files ){
		while( files < argc && argv[files][0] == '-' ) {
			++files;
			}
		if( files > argc ) return( nerrors );
		lopen( argv[files] );
		}
	else lopen( "" );

	fsp = fstack;

	for(;;){
		/* read nodes, and go to work... */
		x = lread();

	if( xdebug ) fprintf( stderr, "op=%d, val = %d, rest = 0%o\n", FOP(x), VAL(x), (int)REST(x) );
		switch( (int)FOP(x) ){  /* switch on opcode */

		case 0:
			fprintf( stderr, "null opcode ignored\n" );
			continue;
		case FTEXT:
			lccopy( VAL(x) );
			printf( "\n" );
			continue;

		case FLBRAC:
			tmpoff = baseoff = lread();
			maxtreg = VAL(x);
			if( ftnno != REST(x) ){
				/* beginning of function */
				maxoff = baseoff;
				ftnno = REST(x);
				maxtemp = 0;
				}
			else {
				if( baseoff > maxoff ) maxoff = baseoff;
				/* maxoff at end of ftn is max of autos and temps 
				   over all blocks in the function */
				}
			setregs();
			continue;

		case FRBRAC:
			SETOFF( maxoff, ALSTACK );
			feoblk();
			continue;

		case FEOF:
			return( nerrors );

		case FSWITCH:
			uerror( "switch not yet done" );
			for( x=VAL(x); x>0; --x ) lread();
			continue;

		case ICON:
			p = talloc();
			p->in.op = ICON;
			p->in.type = REST(x);
			p->tn.rval = 0;
			p->tn.lval = lread();
			if( VAL(x) ){
				lcread( p->in.name, 2 );
				}
			else p->in.name[0] = '\0';

		bump:
			if (p->in.type&017 == SHORT) p->in.type=(p->in.type&(~017)|INT);
			p->in.su = 0;
			p->in.rall = NOPREF;
			*fsp++ = p;
			if( fsp >= &fstack[NSTACKSZ] ) uerror( "expression depth exceeded" );
			continue;

		case NAME:
			p = talloc();
			p->in.op = NAME;
			p->in.type = REST(x);
			p->tn.rval = 0;
			if( VAL(x) ) p->tn.lval = lread();
			else p->tn.lval = 0;
			lcread( p->in.name, 2 );
			goto bump;

		case OREG:
			p = talloc();
			p->in.op = OREG;
			p->in.type = REST(x);
			p->tn.rval = VAL(x);
			p->tn.lval = lread();
			lcread( p->in.name, 2 );
			goto bump;

		case REG:
			p = talloc();
			p->in.op = REG;
			p->in.type = REST(x);
			p->tn.rval = VAL(x);
			rbusy( p->tn.rval, p->in.type );
			p->tn.lval = 0;
			p->in.name[0] = '\0';
			goto bump;

		case FEXPR:
			lineno = REST(x);
			if( VAL(x) ) lcread( filename, VAL(x) );
# if TARGET == IAPX286
			else {
				tmpoff = baseoff = (lread()<<3);
				if(tmpoff > maxoff)
					maxoff = tmpoff;
			}
# endif
			if( fsp == fstack ) continue;  /* filename only */
			if( --fsp != fstack ) uerror( "expression poorly formed" );
			if( lflag ) lineid( lineno, filename );
			tmpoff = baseoff;
			p = fstack[0];
			if( edebug ) fwalk( p, eprint, 0 );
#if TARGET == IAPX286
			walkf( p, tranfrm );
			if( edebug ) fwalk( p, eprint, 0 );
#endif
# ifdef MYREADER
			MYREADER(p);
# endif

			nrecur = 0;
			delay( p );
			reclaim( p, RNULL, 0 );

			allchk();
			tcheck();
			continue;

		case LABEL:
			if( VAL(x) ){
				tlabel();
				}
			else {
				label( (int) REST(x) );
				}
			continue;

		case GOTO:
			if( VAL(x) ) {
				cbgen( 0, (int) REST(x), 'I' );  /* unconditional branch */
				continue;
				}
			/* otherwise, treat as unary */
			goto def;

		default:
		def:
			p = talloc();
			p->in.op = FOP(x);
			p->in.type = REST(x);
			if (p->in.type&017 == SHORT) p->in.type=(p->in.type&(~017)|INT);

			switch( optype( p->in.op ) ){

			case BITYPE:
				p->in.right = *--fsp;
				p->in.left = *--fsp;
				goto bump;

			case UTYPE:
				p->in.left = *--fsp;
				p->tn.rval = 0;
				goto bump;

			case LTYPE:
				uerror( "illegal leaf node: %d", p->in.op );
				exit( 1 );
				}
			}
		}
	}



/*
 * tranfrm : transforms fortran tree into one digestable
 * 	     by the C/286 back end
 */
tranfrm( p, down ) NODE *p; {

	if (optype(p->in.op) == BITYPE) {
		NODE	*lp, *rp;
		rp = p->in.right;
		lp = p->in.left;
		switch( p->in.op ) { /* special cases */
		case	MINUS:
		case	PLUS:
			if( (rp->in.op == ICON) && (rp->in.type == INT) &&
			    (lp->in.type == LONG) )
				rp->in.type = LONG;
			if ((ISPTR(lp->in.type)) && (rp->in.type == LONG)) {
				sconvof(p->in.right, INT);
				break;
			} else if ((ISPTR(rp->in.type)) && (lp->in.type == LONG)) {
				sconvof(p->in.left, INT);
				break;
			}
			if(  ISPTR(p->in.type) &&
			    !ISPTR(lp->in.type) && !ISPTR(lp->in.type) ) {
				p->in.type = lp->in.type;
				break;
			}
			break;
		case	ASSIGN:
			if ((lp->in.type == FLOAT) && (rp->in.type == DOUBLE)) {
				sconvof(p->in.right, FLOAT);
				break;
			} else if ((rp->in.type == FLOAT) && (lp->in.type == DOUBLE)) {
				sconvof(p->in.left, FLOAT);
				break;
			}
			if ((lp->in.type == LONG) && (rp->in.type == INT)) {
				sconvof(p->in.right, LONG);
				break;
			}
			if ((ISPTR(lp->in.type)) && (rp->in.op == ICON)
				 && (rp->in.type == INT)) {
				rp->in.type = lp->in.type;
				break;
			}
			if ((model == MDL_SMALL) && (ISPTR(rp->in.type))
				&& (lp->in.type == LONG)) {
				sconvof(p->in.right, LONG);
			}
			if ((lp->in.type == FLOAT) && (rp->in.type == INT)
				&& (rp->in.op == ICON)) {
				break;
			}
			break;
		case	CM:
			p->in.type = INT;
			break;
		}
	}
}
