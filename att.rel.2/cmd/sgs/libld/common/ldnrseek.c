/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)ldnrseek.c	1.3 - 85/08/09 */
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"

int
ldnrseek(ldptr, sectname)

LDFILE	*ldptr;
char 	*sectname; 

{
	extern	int	ldnshread( );
	extern	int	fseek( );

	SCNHDR	shdr;

	if (ldnshread(ldptr, sectname, &shdr) == SUCCESS) {
		if (shdr.s_nreloc != 0) {
		    if (FSEEK(ldptr, shdr.s_relptr, BEGINNING) == OKFSEEK) {
			    return(SUCCESS);
		    }
		}
	}

	return(FAILURE);
}

