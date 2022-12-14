/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)freeldptr.c	1.3 - 85/08/09 */
#include	<stdio.h>
#include	"filehdr.h"
#include	"ldfcn.h"
#include	"lddef.h"

int
freeldptr(ldptr)

LDFILE	*ldptr;

{
    extern		free( );

    extern LDLIST	*_ldhead;

    LDLIST		*ldindx;

    if (ldptr != NULL) {
	if (_ldhead == (LDLIST *) ldptr) {
	    _ldhead = _ldhead->ld_next;
	    free((char *)ldptr);
	    return(SUCCESS);
	} else {
	    for (ldindx = _ldhead; ldindx != NULL; ldindx = ldindx->ld_next) {
		if (ldindx->ld_next == (LDLIST *) ldptr) {
		    ldindx->ld_next = ((LDLIST *) ldptr)->ld_next;
		    free((char *)ldptr);
		    return(SUCCESS);
		}
	    }
	}
    }

    return(FAILURE);
}

