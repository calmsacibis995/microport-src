/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)ldaclose.c	1.3 - 85/08/09 */
#include    <stdio.h>
#include    "filehdr.h"
#include    "ldfcn.h"

int
ldaclose(ldptr)

LDFILE    *ldptr; 

{
    extern 		fclose( );

    extern int		vldldptr( );
    extern	    	freeldptr( );

    if (vldldptr(ldptr) == FAILURE) {
	return(FAILURE);
    }

    fclose(IOPTR(ldptr));
    freeldptr(ldptr);

    return(SUCCESS);
}

