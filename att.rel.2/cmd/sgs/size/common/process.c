/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)process.c	1.3 - 85/08/09 */
/* UNIX HEADER */
#include	<stdio.h>
#include	<ar.h>

/* COMMON SGS HEADERS */
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"
#include	"sgs.h"

/* SIZE HEADERS */
#include	"defs.h"
#include	"process.h"


    /*  process(filename)
     *
     *  prints out the sum total of section sizes along with the size
     *  information (size, physical and virtual addresses) for each section
     *  in the object file
     *  uses static format strings to do the printing (see process.h).
     *
     *  calls:
     *      - error(file, string) if the object file is somehow messed up
     *
     *  simply returns
     */


process(filename)

char	*filename;

{
    /* UNIX FUNCTIONS CALLED */
    extern		printf( );

    /* COMMON OBJECT FILE ACCESS ROUTINE CALLED */
    extern int		ldshread( );

    /* SIZE FUNCTIONS CALLED */
    extern		error( );

    /* EXTERNAL VARIABLES USED */

    extern int		numbase;
    extern LDFILE	*ldptr;

    /* LOCAL VARIABLES */
    long		size;
    unsigned short	section;
    SCNHDR		secthead;
    ARCHDR		arhead;
    extern int		ldahread( );
    long		ssize[10];
    char		*sname[10];
    unsigned short	notload;  /* number of section which are not loaded 
				     during execution (currently only STYP_INFO) */

    if (ISARCHIVE(TYPE(ldptr))) {
    	if (ldahread(ldptr, &arhead) == SUCCESS) {
    		printf("%s:", arhead.ar_name);
    	} else {
    		printf("%s: ", filename);
	}
    }

    notload = 0;
    for (section = 1; section <= HEADER(ldptr).f_nscns; ++section) {
	if (ldshread(ldptr, section, &secthead) != SUCCESS) {
	    error(filename, "cannot read section header");
	    return;
	}
	if (secthead.s_flags & STYP_INFO) {
		notload = 1;
		break;
	}
    }

    size = 0L;
    for (section = 1; section <= HEADER(ldptr).f_nscns; ++section) {
	if (ldshread(ldptr, section, &secthead) != SUCCESS) {
	    error(filename, "cannot read section header");
	    return;
	}
	if (!(secthead.s_flags & STYP_INFO))  {
		printf(prusect[numbase], secthead.s_size);
		if (HEADER(ldptr).f_nscns - notload > 3)
	    	    	printf("\(%.8s\)", secthead.s_name);
		if (HEADER(ldptr).f_nscns - notload != section)
	    		printf(" + ");
		size += secthead.s_size;
	}
    }
    printf(prusum[numbase], size);
    return;
}

/*
 */
