/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)ldtbseek.c	1.3 - 85/08/09 */
#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"

int
ldtbseek(ldptr)

LDFILE		*ldptr;

{
    extern int		fseek( );

    extern int		vldldptr( );

    if (vldldptr(ldptr) == SUCCESS) {
	if (HEADER(ldptr).f_symptr != 0L) {
	    if (FSEEK(ldptr, HEADER(ldptr).f_symptr, BEGINNING) == OKFSEEK) {
		return(SUCCESS);
	    }
	}
    }

    return(FAILURE);
}

