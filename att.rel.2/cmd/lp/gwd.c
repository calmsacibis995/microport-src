/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
/*
 *	gwd(wkdir) -- place name of working directory in wkdir
 *	returns 0 for success, -1 for failure
 */

#include	"lp.h"


gwd(wkdir)
char *wkdir;
{
	FILE *fp, *popen();
	char *c;

	if ((fp = popen("pwd 2>/dev/null", "r")) == NULL) {
		*wkdir = '\0';
		return(-1);
	}
	if (fgets(wkdir, FILEMAX, fp) == NULL) {
		pclose(fp);
		*wkdir = '\0';
		return(-1);
	}
	if (*(c = wkdir + strlen(wkdir) - 1) == '\n')
		*c = '\0';
	pclose(fp);
	return(0);
}
