/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)ldnshread.c	1.3 - 85/08/09 */
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

int
ldnshread(ldptr, sectname, secthdr)

LDFILE	*ldptr;
char	*sectname;
SCNHDR	*secthdr; 

{
    extern int		fseek( );
    extern int		fread( );

    extern int		vldldptr( );

    unsigned short	i;
    int			j;
    unsigned short	numsects;

    if (vldldptr(ldptr) == SUCCESS) {
	if (FSEEK(ldptr, (long)( FILHSZ + HEADER(ldptr).f_opthdr),
            BEGINNING) == OKFSEEK){
	    numsects = (HEADER(ldptr)).f_nscns;
	    for (i=0;
		(i < numsects) && (FREAD((char *)secthdr,SCNHSZ,1,ldptr) == 1);
		 ++i) {
		    for (j = 0;
		        (j < 8) && (secthdr->s_name[j] == sectname[j]);
		     	++j) {
			    if (secthdr->s_name[j] == '\0') {
				return(SUCCESS);
			    }
		    }
		    if (j == 8) {
		    	return(SUCCESS);
		    }
	    }
	}
    }

    return(FAILURE);
}

