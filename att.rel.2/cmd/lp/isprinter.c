/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/* isprinter -- predicate which returns TRUE if the specified name is
	     a legal lp printer, FALSE if not.		*/

#include	"lp.h"


isprinter(name)
char *name;
{
	char printer[FILEMAX];

	if(*name == '\0' || strlen(name) > DESTMAX)
		return(FALSE);

	/* Check member directory */

	sprintf(printer, "%s/%s/%s", SPOOL, MEMBER, name);
	return(eaccess(printer, ACC_R | ACC_W | ACC_DIR) != -1);
}
