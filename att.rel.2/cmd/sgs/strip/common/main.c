/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)main.c	1.3 - 85/08/09 */
/* UNIX HEADER */
#include	<stdio.h>
#include 	<ar.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"ldfcn.h"



/* SGS SPECIFIC HEADERS */
#include	"paths.h"
#include	"sgs.h"

/* EXTERNAL VARIABLES DEFINED */
LDFILE		*ldptr;
ARCHDR		arhead;
long		xsize = 0;
int 		aflag;
int 		retval;
long 		xmore;
int 		mag;
char		*tmpnam1,
		*tmpnam2,
		*tmpnam3,
		*tmpnam4;
#if FLEXNAMES
char		*tmpnam5;	/* string table temp file */
#endif

    /*  The SGS/UNIX Strip Command operates on one or more object files
     *
     *  main(argc, argv)
     *
     *  parses the command line (setflags( ))
     *  prepares to field interrupt (catchsig( ))
     *  opens, processes and closes each command line object file argument
     *  cleans up after itself 
     *
     *  defines:
     *      - ldptr = ldopen(*argv, NULL) for each command line file argument
     *      - tmpnam1 = temporary file to be used for building the 
     *			stripped file
     *
     *	    - tmpnam2 = temporary file to be used for building a
     *			temporary partially stripped symbol table
     *  calls:
     *      - setflags(--argc, ++argv) to set flags
     *      - catchsig( ) to set up interrupt handling
     *      - process(*argv) to direct the stripping of the object file *argv
     *
     *  prints:
     *      - a usage message if there are no command line object file arguments
     *      - an error message if it can't open a file or if it isn't MAGIC
     *
     *  exits successfully always
     */


main(argc, argv)

int	argc;
char	**argv;

{
    /* UNIX FUNCTIONS CALLED */
    extern		fprintf( ),
			sprintf( ),
			sputl( ),
			unlink( );
    extern int		getpid( );
#ifdef PORT5AR
    extern long		time( );
#endif

    /* OBJECT FILE ACCESS ROUTINES CALLED */
    extern LDFILE	*ldopen( );
    extern int		ldaclose( );

    /* STRIP FUNCTIONS CALLED */
    extern int		setflags( ),
			process( );
    extern		catchsig( ),
			copyarh( ),
			resetsig( );
    extern  FILE 	*stripout;
	    FILE	*tmp;
    extern  long	xmore;
    char		buf[BUFSIZ];
    int			nitems,
			retval = 0;		/* exit return value */
#ifdef PORT5AR
    struct  ar_hdr	arhbuf;
#endif



    if ((argc = setflags(--argc, ++argv)) == 0) {
#if UNIX
	fprintf(stderr,"usage:  %sstrip [-V] [-l] [-x] [-r] [-b] file ...\n", SGS);
#else
	fprintf( stderr, "usage: %sstrip [-V] [-l] [-x] [-r] [-b] [-s] [-f] file ...\n", SGS);
#endif
	exit(0);
    }

	tmpnam1= tempnam(TMPDIR, "strp1");
	tmpnam2= tempnam(TMPDIR, "strp2");
	tmpnam3= tempnam(TMPDIR, "strp3");
	tmpnam4= tempnam(TMPDIR, "strp4");
#if FLEXNAMES
	tmpnam5= tempnam(TMPDIR, "strp5");
#endif
	catchsig( );

	for (	; argc > 0; --argc, ++argv) {
		if ((ldptr = ldopen(*argv, NULL)) == NULL) {
			fprintf( stderr, "%sstrip:	%s:	cannot open\n", SGS, *argv );
			continue;
		}

		mag = TYPE(ldptr);
		if (!ISCOFF(TYPE(ldptr)) && (TYPE(ldptr) != ARTYPE)) {
			fprintf(stderr, "%sstrip:  %s:  bad magic\n", SGS, *argv);
		} else if (TYPE(ldptr) == ARTYPE) {
			aflag = 1;
			do {
			    if (ldahread(ldptr, &arhead) != SUCCESS) {
				fprintf(stderr,"error in archive hdr read %s    %s\n", *argv,arhead.ar_name);
				ldaclose(ldptr);
				exit(1);
				}

			    if (process(*argv) != SUCCESS) {
				/* abandon processing archive, bail out */
				ldaclose(ldptr);
				retval = 1;	/* return an error code */
				goto getout;
				}
			} while (xmore != SUCCESS);

			if ((stripout = fopen(tmpnam4, "r")) == NULL) {
				fprintf(stderr,"cannot open temporary file %s for reading  \n", tmpnam4);
				exit(1);
			}
			if ((tmp  = fopen(*argv, "w")) == NULL) {
				fprintf(stderr,"cannot open temporary file %s for writing\n", *argv);
				exit(1);
			}

#if defined(PORTAR) || defined(PORT5AR)
			/* rewrite the archive header without a symbol dir */
			fprintf(stderr,"%sstrip: Note - symbol directory deleted from archive %s\n",
				SGS, *argv);
			fprintf(stderr,"	execute  `ar ts %s` to restore symbol directory.\n",
				*argv);
#endif

#ifdef PORT5AR
			strncpy(arhbuf.ar_magic,ARMAG,SARMAG);
			strncpy(arhbuf.ar_name,*argv,sizeof(arhbuf.ar_name));
			sputl(time(NULL),arhbuf.ar_date);
			sputl(0L,arhbuf.ar_syms);
			if (fwrite(&arhbuf,sizeof(struct ar_hdr),1,tmp) != 1) {
#else
#ifdef PORTAR
			if (fwrite( ARMAG, SARMAG, 1, tmp ) != 1) {
#else
			if (fwrite( ARMAG, sizeof(ARMAG), 1, tmp ) != 1) {
#endif
#endif
				error( *argv, "can't recreate file", 5 );
				resetsig( );
				exit(1);
			}
			while((nitems = fread(buf,(int)sizeof(char), BUFSIZ, stripout)) != 0 ) {
				if (fwrite(buf,(int)sizeof(char), nitems,tmp) != nitems) {
					error(*argv,"can't recreate file ",5);
					resetsig( );
					exit(1);
				}
			}

		} else { /* not an archive */
			(void) process(*argv);
		}
		ldaclose(ldptr);
	} /* for */

getout:
	unlink(tmpnam1);
	unlink(tmpnam2);
	unlink(tmpnam3);
	unlink(tmpnam4);
#if FLEXNAMES
	unlink(tmpnam5);
#endif
	exit(retval);
}
/*
 */
