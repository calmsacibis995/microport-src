/*      Copyright (c) 1985 AT&T */
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)code.c	1.8 - 85/09/20 */

# include <stdio.h>
# include <signal.h>

# include "mfile1"
# include "paths.h"
# include "sgs.h"
# include <storclass.h>

#define LINENO  lineno-startln+1	/* local line number, opening { = 1 */
#define CDATE	"(prelease 11/8/84)"
 
extern char
	*strcpy(),
	*strcat();
char startfn[100] = "";	/* the file name of the opening '{' */

extern char *mktemp();

int grflag = 0;	/* 0 => normal sub on save; 1 => call cgrow on save */
int tvflag = 0;	/* 0 => direct, 1 => transfer vectors */
int startln;	/* the editor line number of the opening '{' } */
int oldln;	/* remember the last line number printed */
TWORD strftn = 0;   /* does the current function return a structure value? */
int prflag = 0;	/* flag to do profiling - not implemented */
int dsflag = 0;	/* don't print symbolic debugging info when set */
int dlflag = 0;	/* don't print line number debugging ingo when set */
int Sflag  = 0; /* use re-entrant linkage for returning structures from fuctions */

int lastloc = -1;	/* remember the last location counter output */

int bb_flags[BCSZ];	/* remember whether or not bb/eb is needed */

#define RTNREGOFF -2	/* offset in the stack for the structure return address */

/* stmpfile is used to hold initializing strings that cannot be
 * output immediately.  Strings are read back in at the
 * conclusion of pass 1 and appended to the intermediate text 
 */
FILE *stmpfile;
FILE *outfile = stdout;

extern int asm_esc;  /* set if assembly language escape used */

branch( n ){
	/* output a branch to label n */
	fprintf( outfile,  "\tjmp\t.%d\n", n );
	}

defalign(n) {
	/* cause the alignment to become a multiple of n
	 * n will always be 1 or 2, odd addresses are
	 * rounded up to the next even address by .even
	 */
	n /= SZCHAR;
	if( lastloc != PROG && n > 1 )
		fprintf( outfile, "\t.even\n" );
}

locctr( l ) register int l; {
	/* l is PROG, ADATA, DATA, STRNG, ISTRNG, STAB, or BSS
	 * lastloc remembers the last location counter that was output
	 * the old location counter is returned by locctr
	 */
	register temp;

	/* don't output another if still in the same location space */
	if( l == lastloc )
		return( l );

	temp = lastloc;
	lastloc = l;

	switch( l ){

	case BSS:
		outfile = stdout;
		fprintf( outfile, "\t.bss\n" );
		break;

	case PROG:
		outfile = stdout;
		fprintf( outfile,  "\t.text\n" );
		break;

	case DATA:
	case ADATA:
		outfile = stdout;
		if( temp != DATA && temp != ADATA )
			fprintf( outfile, "\t.data\n" );
		break;

	case STRNG:
	case ISTRNG:
		outfile = stmpfile;
		break;

	default:
		/* should catch STAB and other errors */
		cerror( "illegal location counter" );
		}

	return( temp );
	}

deflab( n ){
	/* output something to define the current position as label n */
	fprintf( outfile,  ".%d:\n", n );
	}

getlab(){
	/* return a number usable for a label */

	static int crslab = 10;
	return( ++crslab );
	}

struct sw heapsw[SWITSZ];	/* heap for switches */

genswitch( p, n ) register struct sw *p; {
	/* p points to an array of structures, each consisting
	 * of a constant value and a label.
	 * The first is >=0 if there is a default label;
	 * its value is the label number
	 * The entries p[1] to p[n] are the nontrivial cases
	 *
	 * Costs	space		time
	 * if-else	5n 6n		4n+8
	 * jump		19+2r 21+2r	42
	 * heap		7n 9n		6l+4
	 *
	 * n = # of cases
	 * r = range of cases (max - min)
	 * l = ceiling of base 2 log of n
	 */
	register i;
	register CONSZ j, range;
	register dlab, swlab;

	range = p[n].sval-p[1].sval;

	/* choice of switch type is based on the following costs:
	 *	direct:	4*(range+1)+18 bytes
	 *	heap:	?
	 *	simple:	n*6+2 bytes
	 */

	if( range > 0 && range + 21 < 5*n ) { /* jump table switch */

		swlab = getlab();
		dlab = p->slab >= 0 ? p->slab : getlab();

		/* already in ax */
		fprintf( outfile, "\tsub\t$" );
		fprintf( outfile, CONFMT, (CONSZ)p[1].sval );
		fprintf( outfile, ",%%ax\n" );
		fprintf( outfile, "\tjl\t.%d\n", dlab );
		fprintf( outfile, "\tcmp\t$%ld,%%ax\n", range );
		fprintf( outfile, "\tjg\t.%d\n", dlab );
		fprintf( outfile, "\tsal\t%%ax\n" );
		fprintf( outfile, "\tmov\t%%ax,%%si\n" );
		fprintf( outfile, "\tjmp\t*%%cs:.%d(%%si)\n", swlab );
		fprintf( outfile, "\t.jmpbeg\n"); /* jmp tab start for dis */
		deflab( swlab );

		for( i=1,j=p[1].sval; i<=n; ++j )
			fprintf( outfile, "\t.value\t.%d\n",
				(j == p[i].sval) ? p[i++].slab : dlab );

		fprintf( outfile, "\t.jmpend\n"); /* jmp tab end for dis */
		if( p->slab < 0 )
			deflab( dlab );
		}

	else if( n > 8 ) {	/* heap switch: must be twice as fast as simple */

		heapsw[0].slab = dlab = p->slab >= 0 ? p->slab : getlab();
		makeheap( p, n, 1 );	/* build heap */

		walkheap( 1, n );	/* produce code */

		if( p->slab >= 0 )
			branch( dlab );
		else
			deflab( dlab );
		}

	else {	/* simple switch */

		for( i=1; i<=n; ++i ){
			/* the switch expression is guaranteed to be in ax */
	
			fprintf( outfile,  "\tcmp\t$" );
			fprintf( outfile,  CONFMT, (CONSZ)p[i].sval );
			fprintf( outfile,  ",%%ax\n\tje\t.%d\n", p[i].slab );
			}
	
		if( p->slab >= 0 )
			branch( p->slab );
		}
	}

makeheap( p, m, n ) register struct sw *p; {
	register int q;

	q = select( m );
	heapsw[n] = p[q];
	if( q > 1 ) makeheap( p, q-1, 2*n );
	if( q < m ) makeheap( p+q, m-q, 2*n+1 );
	}

select( m ) {
	register int l, i, k;

	for( i=1; ; i*=2 )
		if( (i-1) > m ) break;
	l = ((k = i/2 - 1) + 1)/2;
	return( l + (m-k < l ? m-k : l) );
	}

walkheap( start, limit ) {
	register int label;

	if( start > limit ) return;
	fprintf( outfile, "\tcmp\t$" );
	fprintf( outfile, CONFMT, (CONSZ)heapsw[start].sval );
	fprintf( outfile, ",%%ax\n\tje\t.%d\n", heapsw[start].slab );
	if( (2*start) > limit ) {
		fprintf( outfile, "\tjmp\t.%d\n", heapsw[0].slab );
		return;
		}
	if( (2*start+1) <= limit ) {
		label = getlab();
		fprintf( outfile, "\tjg\t.%d\n", label );
		}
	else
		fprintf( outfile, "\tjg\t.%d\n", heapsw[0].slab );
	walkheap( 2*start, limit );
	if( (2*start+1) <= limit ) {
		fprintf( outfile, ".%d:\n", label );
		walkheap( 2*start+1, limit );
		}
	}

TWORD ftntype;	/* Type of the current function, used by PCI */

efcode(){
	/* code for the end of a function */

	deflab( retlab );

	/* if this function returns a structure value, the address
	 * of the structure to be returned is in %ax,
	 * the address where the structure is to be returned
	 * to is in -2(%bp)
	 */

	if( strftn && (retstat & RETVAL) ) {
		/* copy output (in ax) to caller */
		register NODE *l, *r;
		register struct symtab *p;
		register TWORD t;
		int i;
	
		p = &stab[curftn];
		t = p->stype;
		t = DECREF(t);

		/* generate a structure assignment expression
		 * reached is set to ecomp() won't complain
		 */

		reached = 1;

		if (Sflag) { /* RE-ENTRANT STRUCTURE RETURN LINKAGE */
			printf("\tmov\t%%ax,%%si\n\tpop\t%%ax\n\tmov\t%%ax,%%di\n", i);
			if(model >= MDL_LARGE)
				printf("\tmov\t%%dx,%%ds\n\tpop\t%%dx\n\tmov\t%%dx,%%es\n", i);
		} else {
			i = getlab();
			printf("\t.bss\n.%d:\t.set\t.,.+%d\n\t.text\n",
				i, tsize(t, p->dimoff, p->sizoff) / SZCHAR );
			printf("\tmov\t%%ax,%%si\n\tmov\t$.%d,%%ax\n\tmov\t%%ax,%%di\n", i);
			if(model >= MDL_LARGE)
				printf("\tmov\t%%dx,%%ds\n\tmov\t$<s>.%d,%%dx\n\tmov\t%%dx,%%es\n", i);
		}
		/* left operand is (%di) */
		l = block( REG, NIL, NIL, INCREF(t), p->dimoff, p->sizoff );
		l->tn.rval = DI;

		/* right operand is (%si) */
		r = block( REG, NIL, NIL, INCREF(t), p->dimoff, p->sizoff );
		r->tn.rval = SI;
		r->tn.lval = 0;

		l = buildtree( UNARY MUL, l, NIL );
		r = buildtree( UNARY MUL, r, NIL );
		l = buildtree( ASSIGN, l, r );
		l->in.op = FREE;

		ecomp( l->in.left );
	}

	/* print end-of-function pseudo and its line number */
	if( !dsflag ) {
		fprintf( outfile, "\t.def\t.ef; .val .; .scl %d; .type 0x00; .line %d; .endef\n", C_FCN, LINENO );
		if( LINENO > 1 )
			fprintf( outfile, "\t.ln\t%d\n", LINENO );
	}

	/* this is the only place where return from a function can appear */
/*
	if(model < MDL_LARGE) {
		fprintf(outfile,"\tpop\t%%di\n");
		fprintf(outfile,"\tpop\t%%cx\n");
	}
*/
	fprintf(outfile,"\tleave\n");
	if(model < MDL_MIDDLE)
		fprintf(outfile,"\tret\n");
	else
		fprintf(outfile,"\tlret\n");
	ftntype = stab[curftn].stype;
	ftntype = DECREF(ftntype);
# ifdef FLEXNAMES
	fprintf(outfile, "\t.def\t%s; .val .; .scl -1; .endef\n", stab[curftn].sname);
# else
	tfprintf(outfile, "\t.def\t%.8s; .val .; .scl -1; .endef\n", stab[curftn].sname);
# endif
	/* calculate stack for automatics in pass two */
#ifdef ONEPASS
	p2bend();
#else
	printf("%c%o\t\n", BEND, ftntype);
#endif
	}

char *rnames[] =
	{  /* keyed to register number tokens */

	"%ax", "%dx",     /* scratch registers */
	"%si", "%bx",     /* scratch registers */
	"%cx", "%di",   /* register variables */
	"%sp", "%bp",    /* special purpose registers */
	"%ss", "%ds", /* segment registers  */
	"%es", "%cs",
	"%st", "%st(1)"
	};
bfcode( a, n ) int a[]; {
	/* code for the beginning of a function; a is an array of
		indices in stab for the arguments; n is the number */
	register i, temp;
	register struct symtab *p;
	OFFSZ off;
	/* TWORD ftype; */

	/* get into the .text section */
	locctr( PROG );

	/* print the entry point for the function and the .def */
	p = &stab[curftn];
	{
	int temp;
	temp = dsflag; /* save dsflag */
	if( dsflag )	/* don't remove .def for function name */
		dsflag = -1;
	defnam( p );
	dsflag = temp; /* restore dsflag */
	}
	/*  procedure declarative not used at this time
	printf("%s:\t.proc\t%s\n", p->sname,
				    (model < MDL_MIDDLE) ? "near" : "far");
	*/

	/* determine if this function returns a structure value */
	temp = p->stype;
	temp = DECREF( temp );
	strftn = (temp==STRTY) || (temp==UNIONTY);
	retlab = getlab();

	/* routine prolog */
	fprintf(outfile,"\tenter\t$.F%d,$0\n", ftnno);

	if(Sflag && strftn) { /* RE-ENTRANT STRUCTURE RETURN LINKAGE */
		if(model >= MDL_LARGE) fprintf(outfile, "\tpush\t%%dx\n");
		fprintf(outfile, "\tpush\t%%ax\n");
	}

	if (prflag) {
		fprintf(outfile, "\t.data\n");
		temp = getlab();
		deflab(temp);
		if(model > 3)
			fprintf(outfile, "\t.value\t0\n");
		fprintf(outfile, "\t.value\t0\n\t.text\n");

		if (model > 1) 
		    fprintf(outfile,"\tmov\t$<s>.%d,%%bx\n\tmov\t$.%d,%%si\n\tlcall\t_mcount\n",temp, temp);
		else 
		    fprintf(outfile,"\tmov\t$.%d,%%si\n\tcall\t_mcount\n",temp);
	}

	/* initialize line number counters */
	oldln = startln = lineno;
	strcpy(startfn,ftitle);

	off = ARGINIT;

	/* load parameters declared to be of class register */
	for( i=0; i<n; ++i ){
		p = &stab[a[i]];

		if( p->sclass == REGISTER ){
			temp = p->offset;  /* save register number */
			p->sclass = PARAM;  /* forget that it is a register */
			p->offset = NOOFFSET;
			oalloc( p, &off );
			fprintf( outfile,  "\tmov\t%ld(%%bp),%s\n", p->offset/SZCHAR, rnames[temp] );
			p->offset = temp;  /* remember register number */
			p->sclass = REGISTER;   /* remember that it is a register */
			}
		else {
			if( oalloc( p, &off ) ) cerror( "bad argument" );
			}
		}

	if( !dsflag ) {
		/* do .bf symbol and .defs for parameters
		 * paramters are delayed to here to two reasons:
		 *    1: they appear inside the .bf - .ef
		 *    2: the true declarations have been seen
		 */
		fprintf( outfile, "\t.ln\t1\n" );
		fprintf( outfile, "\t.def\t.bf; .val .; .scl %d; .type 0x00; .line %d; .endef\n", C_FCN, lineno );
		for( i=0; i<n; ++i ) {
			p = &stab[a[i]];
			prdef( p );
		}

	}

}

/*	beg_file - mach. dep. stuff for the start of a file	*/
beg_file(){}

bccode() {
	/* called just before the first executable statement
	 * by now, the automatics and register variables are allocated

	SETOFF( autooff, ALSTACK );

	/* this is the block header:
	 * autooff is the max offset for auto and temps
	 * regvar is the least numbered register variable
	 */
#ifdef ONEPASS
	p2bbeg(autooff, regvar);
#else
	printf("%c%d\t%ld\t%d\t\n", BBEG, ftnno, autooff, regvar);
#endif
	}

ejobcode( flag ){
/*	prtable();	*/
	/* called just before final exit */
	/* flag is 1 if errors, 0 if none */
	}

aoend(){
	/* called after removing all automatics from stab */
	/* print .eb here if .bb printed for this block  */
	if ( bb_flags[blevel+1] ) {
		fprintf( outfile, "\t.def\t.eb; .val .; .scl %d; .type 0x00; .line %d; .endef\n", C_BLOCK, LINENO );
		bb_flags[blevel+1] = 0;
		}
	}

defnam( p ) register struct symtab *p; {
	/* define the current location as the name p->sname */
	/* first give debugging code for external definitions */

	extdef( p );

	if( p->sclass == EXTDEF )
# ifdef FLEXNAMES
		fprintf( outfile,  "\t.globl\t%s\n", exname( p->sname ) );
# else
		fprintf( outfile,  "\t.globl\t%.8s\n", exname( p->sname ) );
# endif
	if (tvflag && ISFTN(p->stype))
# ifdef FLEXNAMES
		fprintf(outfile, "\t.tv     %s\n", exname(p->sname));
# else
		fprintf(outfile, "\t.tv\t%.8s\n", exname(p->sname));
# endif
	if( p->sclass == STATIC && p->slevel>1 )
		deflab( (int)p->offset );
	else
# ifdef FLEXNAMES
		fprintf( outfile,  "%s:\n", exname( p->sname ) );
# else
		fprintf( outfile,  "%.8s:\n", exname( p->sname ) );
# endif
}

bycode( t, i ){
	/* put byte i+1 in a string */
	register char *s;
	static int lastzero;

	if(t < 0) {
		if(i != 0)
			fprintf(outfile, "\"\n");
		return;
	}
	if(i == 0) {
		lastzero = 0;
		fprintf(outfile, "\t.string\t\"");
	}
	if(lastzero) {
		fprintf(outfile, "\\000");
		lastzero = 0;
	}
	if(t == 0) {
		lastzero++;
		return;
	}
	if((t < 040) || (t >= 0177)) {
		switch(t)
			{
			default:
				fprintf(outfile, "\\%.3o", t);
				return;

			case '\n':
				s = "\\n";
				break;

			case '\r':
				s = "\\r";
				break;

			case '\b':
				s = "\\b";
				break;

			case '\t':
				s = "\\t";
				break;

			case '\f':
				s = "\\f";
				break;

			case '\v':
				s = "\\v";
				break;
			}
		fprintf(outfile, s);
		return;
	}
	if((t == '"') || (t == '\\'))
		putc('\\', outfile);
	putc(t, outfile);
	}

zecode( n ){	/* fill out partly initialized arrays */
	/* n integer words of zeros */
	OFFSZ temp;
	register i;

	if( n <= 0 ) return;
	fprintf( outfile,  ".=.+%d\n", n*2 );
	temp = n;
	inoff += temp * SZINT;
	}

fldal( t ) unsigned t; { /* return the alignment of field of type t */
	uerror( "illegal field type" );
	return( ALINT );
	}

fldty( p ) struct symtab *p; { /* fix up type of field p */
	}

where(c){ /* print location of error  */
	/* c is either 'u', 'c', or 'w' */
	fprintf( stderr, "%s, line %d: ", ftitle, lineno );
	}

extern int fpe_flag;
fpe_catch() {
	extern double dcon, dcon1;
	/*
	 * Floating exception generated by constant folding.
	 */
	werror( "floating point constant expression too large" );
	fpe_flag++;
	dcon = dcon1 = 1.0;
	(void) signal( SIGFPE, fpe_catch );
}

char tempnm[I286TMPLN+12] = I286TMP;
char *tmpname = tempnm;
int  wloop_level = LL_TOP;
int  floop_level = LL_TOP;

main( argc, argv ) char *argv[]; {
	register int c;
	int dexit(), retrn, i;
	register char *findnm, *gotnm;
	extern int fpe_catch();
	/* Code to collect performance stats */
	long ttime;
	struct tbuffer {
		long proc_user_time;
		long proc_system_time;
		long child_user_time;
		long child_system_time;
	} ptimes;
	extern long times();

	ttime = times(&ptimes);
	gotnm = "";

	for( i = 1; i < argc && argv[i][0] == '-'; i++ ) {
		switch( argv[i][1] ) {
		case 'p' :
			prflag = 1;
			break;	/* profile not implemented */

		case 'd' :
			/* set dl & ds flags to prevent symbolic debug stuff */
			if( argv[i][2] == 'l' || argv[i][3] == 'l' )
				dlflag = 1;
			if( argv[i][2] == 's' || argv[i][3] == 's' )
				dsflag = 1;
			break;

		case 'f' :
			/* pick up file name */
			for( gotnm = findnm = argv[++i]; *findnm; ++findnm )
				if( *findnm == '/' )
					gotnm = findnm + 1;
			break;

		case 'i': {	/* input file name */
			if( freopen(argv[++i],"r",stdin) == NULL ) {
				fprintf( stderr, "front: can't open %s\n", argv[i] );
				exit( 2 );
			}
			break;
		}

		case 'o': {	/* output file name */
			if( freopen(argv[++i],"w",stdout) == NULL ){
				fprintf( stderr, "front: can't open %s\n", argv[i] );
				exit( 2 );
			}
			break;
		}

		case 't':	/* transfer vector flag */
			tvflag++;
			break;

		case 'X':
			break;	/* debugging flag */

#ifdef ONEPASS
		case 'Y':
			argv[i]++;	/* overwrite the Y with - */
			argv[i][0] = '-';	/* as in old main in local2 */
			break;	/* debugging flag */
#endif

		case 'G':
			grflag++;	/* call seq change for mini-UN*X */
			break;

		case 'S':
			Sflag++;	/* re-entrant structure return linkage */
			break;

		case 'V':
			fprintf( stderr, "%s: compiler - %s %s\n",
				SGSNAME, RELEASE, CDATE );
			break;

		case 'M':
			switch(argv[i][2])
				{
				case 's':
					model = MDL_SMALL;
					break;

				case 'm':
					model = MDL_MIDDLE;
					break;

				case 'l':
					model = MDL_LARGE;
					break;

				case 'h':
					model = MDL_HUGE;
					break;

				case 'c':
					fprintf( stderr, "compact model not supported yet\n");
					exit(2);

				default:
					fprintf( stderr, "bad model selected\n");
					exit(2);
				}
			break;

		case 'n':
			fartab = &argv[++i];
			fartabsize = argc - i;
			break;

		default:
#ifndef ONEPASS
			fprintf( stderr, "front: bad argument %s\n", argv[i] );
			exit( 2 );
#else
			continue;
#endif

		}
	}

	/* print the .file pseudo */
	if( *gotnm != '\0' )
		fprintf( stdout, "\t.file\t\"%.14s\"\n", gotnm );

	strcat(tmpname,"/c286XXXXXX");
	stmpfile = fopen(mktemp(tmpname), "w");

	if ( signal( SIGHUP, SIG_IGN ) == SIG_DFL )
		signal( SIGHUP, dexit );
	if ( signal( SIGINT, SIG_IGN ) == SIG_DFL )
		signal( SIGINT, dexit );
	if ( signal( SIGTERM, SIG_IGN ) == SIG_DFL )
		signal( SIGTERM, dexit );
	/*
	 * Catch floating exceptions generated by the constant
	 * folding code.
	 */
	(void) signal( SIGFPE, fpe_catch );

	/* main for pass 1 is in scan (the scanner) */
	retrn = mainp1( argc, argv );

	if (tvflag)
		exttv();	/* .tv for referenced functions */

	outfile = stdout;
	stmpfile = freopen(tmpname, "r", stmpfile);
	if (stmpfile != NULL)
		while((c=getc(stmpfile)) != EOF)
			putchar(c);
	else
		cerror("Lost temp file");

	unlink(tmpname);

	if (retrn == 1) retrn = 2;
	/* Write performance stats to file */
	{
	FILE *perfile;

		ttime = times(&ptimes) - ttime;
		if ((perfile = fopen("comp.perf.info", "r")) != NULL) {
			fclose(perfile);
			if ((perfile = fopen("comp.perf.info", "a")) != NULL) {
				fprintf(perfile,"front\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\n",ttime,ptimes);
				fclose(perfile);
			}
		}
	}
	if (ferror(stdout)) cerror("Trouble writing output file");
	return( retrn ? retrn : (asm_esc ? 1 : 0) );
	}

/*VARARGS*/
dexit() {
	/* remove the temp file when signals are caught */
	unlink(tmpname);
	exit(2);
}

/* exttv prints .tv pseudo-ops for each function referenced
   but not defined.  This includes _csv and _cret but not longops */
exttv() {
	int i;
	register struct symtab *p;
	fprintf(outfile, "\t.tv\t_csv;	.tv	_cret\n");
	for (i=0; i < SYMTSZ+1; i++) {
		p = &stab[i];
		if (ISFTN(p->stype) && p->sclass == EXTERN)
# ifdef FLEXNAMES
			fprintf(outfile, "\t.tv     %s\n", p->sname);
# else
			fprintf(outfile, "\t.tv\t%.8s\n", p->sname);
# endif
	}
}

/* prtable displays the contents of the symtab and dimtab
   so that i can see how they work */
/* This is apparently not used
 * prtable() {
 * int i;
 * struct symtab *p;
 * int *q;
 * 
 * printf("SYMTAB\n");
 * printf("addr(o) index name type(o) sclass level flags(o) offset dimoff(o) sizoff(o) suse\n\n\n");
 * for(i=0;i<SYMTSZ+1;i++){
 * 	p= &stab[i];
 * 	printf("%6o %6d %8.8s %6o %6d %6d %6o %6ld %6o %6o %6d\n",p,i,p->sname,p->stype,p->sclass,p->slevel,p->sflags,p->offset,p->dimoff,p->sizoff,p->suse);
 * 	}
 * printf("\n\n\n\n\n)DIMTAB\n)addr(o) index val val(o)\n\n");
 * for(i=0;i<DIMTABSZ;++i){
 * 	q= &dimtab[i];
 * 	printf("%6o %6d %6ld %6lo\n",q,i,dimtab[i],dimtab[i]);
 * 	}
 * }
 */

arginit() {
	if ( model < 3 ) 
		return(tvflag ? 48 : 32);
	else
		return(tvflag ? 64 : 48);
}
