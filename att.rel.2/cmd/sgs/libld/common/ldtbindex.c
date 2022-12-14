/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)ldtbindex.c	1.3 - 85/08/09 */
#include	<stdio.h>
#include	"filehdr.h"
#include	"syms.h"
#include	"ldfcn.h"

long
ldtbindex(ldptr)

LDFILE	*ldptr;

{
    extern long		ftell( );

    extern int		vldldptr( );

    long		position;


    if (vldldptr(ldptr) == SUCCESS) {
	if ((position = FTELL(ldptr) - OFFSET(ldptr) - HEADER(ldptr).f_symptr) 
	    >= 0) {

	    if ((position % SYMESZ) == 0) {
		return(position / SYMESZ);
	    }
	}
    }

    return(BADINDEX);
}

