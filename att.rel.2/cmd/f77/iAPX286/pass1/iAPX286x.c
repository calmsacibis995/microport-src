/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)iAPX286x.c	1.3 85/09/06 */
#include <stdio.h>
#include "defines"
#include "machdefs"


prjunk(fp, infname)
FILEP fp;
char *infname;
{
register char *t;
register int i;
	t = infname + (i = strlen(infname));
	while (*--t != '/' && i-- > 0)
		;
	fprintf(fp, "\t.file\t\"%s\"\n", ++t);
}

prchars(fp, s)
FILEP fp;
int *s;
{

fprintf(fp, ".byte 0%o,0%o\n", s[0], s[1]);
}



pruse(fp, s)
FILEP fp;
char *s;
{
fprintf(fp, "\t%s\n", s);
}



prskip(fp, k)
FILEP fp;
ftnint k;
{
fprintf(fp, "	.=.+%ld\n", k);
}





prcomblock(fp, name)
FILEP fp;
char *name;
{
fprintf(fp, LABELFMT, name);
}
