/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)ldtbread.c	1.3 - 85/08/09 */
#include	<stdio.h>
#include	"filehdr.h"
#include	"syms.h"
#include	"ldfcn.h"

int
ldtbread(ldptr, symnum, symentry)

LDFILE	*ldptr;
long	symnum;
SYMENT	*symentry;

{
    extern int		fseek( );
    extern int		fread( );

    extern int		vldldptr( );

    if (vldldptr(ldptr) == SUCCESS) {
	if ((symnum >= 0) && (symnum <= (HEADER(ldptr)).f_nsyms)) {
	    if (FSEEK(ldptr,
		HEADER(ldptr).f_symptr + symnum * SYMESZ, BEGINNING)
		== OKFSEEK) {
		if (FREAD((char *)symentry,SYMESZ,1,ldptr) == 1){
		    return(SUCCESS);
		}
	    }
	}
    }

    return(FAILURE);
}

