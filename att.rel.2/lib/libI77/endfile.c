/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286.c @(#)endfile.c	1.4 85/09/16 */
/*	@(#)endfile.c	1.3	*/

#include "fio.h"
static alist *ax;
extern char *mktemp(), *strcpy();
f_end(a) alist *a;
{
	unit *b;
	if(a->aunit>=MXUNIT || a->aunit<0) err(a->aerr,101,"endfile");
	b = &units[a->aunit];
	if(b->ufd==NULL) return(0);
	b->uend=1;
	if( b->useek==0) return(0);
	ax=a;
	if(b->uwrt) (void) nowreading(b);
	return(t_runc(b));
}
t_runc(b) unit *b;
{
	char buf[128],nm[16];
	FILE *tmp;
	int n,m;
	long loc,len;
	if(b->url) return(0);	/*don't truncate direct files*/
	loc=ftell(b->ufd);
	(void) fseek(b->ufd,0L,2);
	len=ftell(b->ufd);
	if(loc==len || b->useek==0 || b->ufnm==NULL) return(0);
	(void) strcpy(nm,"tmp.FXXXXXX");
	if(b->uwrt) (void) nowreading(b);
	(void) mktemp(nm);
	tmp=fopen(nm,"w");
	(void) fseek(b->ufd,0L,0);
	for(;loc>0;)
	{
		n=fread(buf,1,loc>128?128:(int)loc,b->ufd);
		if(n>loc) n=loc; else if (n<=0) break;
		loc -= n;
		(void) fwrite(buf,1,n,tmp);
	}
	(void) fflush(tmp);
	for(n=0;n<10;n++)
	{
		if((m=fork())==-1) continue;
		else if(m==0)
		{
#if iAPX286
			(void) execl("/bin/cp","cp",nm,b->ufnm,(char *) 0);
			(void) execl("/usr/bin/cp","cp",nm,b->ufnm,(char *) 0);
#else
			(void) execl("/bin/cp","cp",nm,b->ufnm,0);
			(void) execl("/usr/bin/cp","cp",nm,b->ufnm,0);
#endif
			fprintf(stdout,"no cp\n");
			exit(1);
		}
		(void) wait(&m);
		if(m!=0) err(ax->aerr,111,"endfile");
		(void) fclose(tmp);
		(void) unlink(nm);
		return(0);
	}
	err(ax->aerr,111,"endfile");
}
