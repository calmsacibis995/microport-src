/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)errdead.c	1.3 - 85/08/09 */
#include "stdio.h"
#include "a.out.h"
#include "sys/param.h"
#include "signal.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/utsname.h"
#include "sys/wn.h"
#include "sys/elog.h"
#include "sys/erec.h"
#include "sys/map.h"
#include "sys/err.h"
#include "sys/iobuf.h"
#include "sys/seg.h"
#include "sys/mmu.h"

long vtop();

#undef n_name
struct nlist nl[] = {
	{"err"},
	{"time"},
	{"i215tab"},
	{"\0"}
};

time_t	time;
char	*dump;
char	*nlfile	= "/unix";
char	*tmpfil;
int	fd;
int	tfd;
int	child;
int	wflg;
struct iobuf iobuf;

struct err err;

char	*sbrk();
char	*mktemp();
int	catch();

main(argc,argv)
char **argv;
{
	register long n, slot;
	register struct err *ep = &err;
	int status;
	struct iobuf *dp;

	if(argc < 2)
		errexit("Arg count\n");
	dump = *++argv;
	if(argc > 2)
		nlfile = *++argv;
	nlist(nlfile,nl);
	if(nl[0].n_value == 0 || nl[1].n_value == 0)
		errexit("Bad nlist\n");
	if((fd = open(dump,0)) < 0)
		errexit("Can't open dump file\n");
	fetch(vtop(nl[0].n_value,fd),(char *)&err,sizeof(struct err));
	fetch(vtop(nl[1].n_value,fd),(char *)&time,sizeof(time_t));
	if(NESLOT != ep->e_nslot)
		errexit("Number of slots in dump don't agree with sys/err.h\n");
	for(n = 1; n < 32; n++)
		signal(n,catch);
	tmpfil = mktemp("/usr/tmp/errXXXXXX");
	if((tfd = creat(tmpfil,0666)) < 0) {
		tmpfil = mktemp("/tmp/errXXXXXX");
		if((tfd = creat(tmpfil,0666)) < 0)
			errexit("Can't create tmp file \n");
	}
	slot = (struct errhdr **) ((char *)ep->e_org
		- (char *)nl[0].n_value
		+ (char *)&err) - ep->e_ptrs;
	for(n = slot; n < ep->e_nslot; n++)
		puterec(ep->e_ptrs[n],0);
	for(n = 0; n < slot; n++)
		puterec(ep->e_ptrs[n],0);
	if(nl[2].n_value)
		{
		n = (unsigned)(vtop(nl[2].n_value,fd) +  sizeof(iobuf)*8);
		for(dp = (struct iobuf *)vtop(nl[2].n_value,fd); dp < (struct iobuf *) n; dp++)
			getiobuf((unsigned)dp);
	}
	for (n = 3; nl[n].n_name[0]; n++)
		getiobuf(vtop(nl[n].n_value,fd));
	close(tfd);
	if(wflg == 0)
		printf("No errors logged\n");
	else {
		if((child = fork()) == 0) {

			execl("errpt","errpt","-a",tmpfil,0L);
			execl("/bin/errpt","errpt","-a",tmpfil,0L);
			execl("/usr/bin/errpt","errpt","-a",tmpfil,0L);
			exit(16);
		}
		if(child < 0) {
			unlink(tmpfil);
			errexit("Can't fork\n");
		}
		wait(&status);
	}
	unlink(tmpfil);
}
errexit(s1)
char *s1;
{
	fprintf(stderr,s1);
	exit(16);
}
fetch(off,buf,size)
char *buf;
long off;
{
	lseek(fd,off,0);
	if(read(fd,buf,size) != size)
		errexit("Read error\n");
}
getiobuf(addr)
unsigned long register addr;
{
	if(addr)
		{
		fetch(vtop(addr,fd), (char *) &iobuf, sizeof(iobuf));
		if(iobuf.io_erec)
			puterec(iobuf.io_erec,1);
	}
}
puterec(erec,unlog)
register struct errhdr *erec;
register unlog;
{
	if(erec)
		{
		erec = (struct errhdr *) ((char *)erec
		- (char *)nl[0].n_value
		+ (char *)&err);
		if(unlog)
			{
			erec->e_type = E_BLK;
			erec->e_time = time;
			((struct eblock *)((char *)erec + sizeof(struct errhdr)))->e_bflags |= E_ERROR;
		}
		write(tfd, (char *)erec, erec->e_len);
		wflg++;
	}
}
catch()
{
	if(tmpfil)
		unlink(tmpfil);
	exit(1);
}

/* Convert virtual address to physical offset in core file. */
long
vtop(vad, fd)
unsigned long	vad;	/* virtual address */
int		fd;	/* core file file descriptor */
{
	struct seg_desc	seg;	/* virtual address GDT segment descriptor */

	/* Seek to GDT selector for given virtual address. */
	if(lseek(fd, (long)(PHYS_KDATA + WUBSIZ) + ((vad >> 16) & SEL_INDEX), 0) == -1)
		return(0L);

	/* Read the segment descriptor. */
	if(read(fd, &seg, sizeof seg) != sizeof seg)
		return(0L);

	/* Verify that this is a kernel data segment. */
	if((seg.sd_access & (ACC_KDATA)) != (ACC_KDATA))
		return(0L);

	/* Verify that offset of given address in within segment limit. */
	if((unsigned)vad > seg.sd_limit)
		return(0L);

	/* Calculate physical offset of virtual address. */
	return(((unsigned long)seg.sd_hibase << 16) + 
		(unsigned long)seg.sd_lowbase + (vad & 0xffffL));
}
