/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)iAPXmisc.c	1.4 - 85/08/12 */
#include	"crash.h"
#include	"sys/inode.h"
#include	"sys/proc.h"
#include	"sys/file.h"
#include	"sys/mount.h"
#include	"sys/text.h"
#if iAPX286
long lseek();
#else
#define	SEGBYTES	0x40000000L	/* power of 2 to the 30th */
#define	CORECLKS	512		/* Core Clicks */
#define	BITMASK		0777		/* Remove low order bits */

extern	struct	dstk	dmpstack;
extern	struct	glop	dumpglop;
#endif
extern	struct	uarea	x;
int	nmfd;

fatal(str)
	char	*str;
{
	printf("error: %s\n", str);
	exit(1);
}

error(str)
	char	*str;
{
	printf("error: %s\n", str);
}

atoi(s)
	register  char  *s;
{
	register  base, numb;

	numb = 0;
	if(*s == '0')
		base = 8;
	else
		base = 10;
	while(*s) {
		if(*s < '0' || *s > '9') {
			error("number expected");
			return(-1);
		}
		numb = numb * base + *s++ - '0';
	}
	return(numb);
}

init()
{
	extern	char	*dumpfile;
	extern	struct	dstk	dmpstack;
#if iAPX286
	extern	ADDR	Kfp;
#else
	extern	int	Kfp;
#endif
	extern	char	*namelist;
	extern	struct	syment	*stbl;
	extern  unsigned  symcnt;
	extern	struct	var	v;

	long	symloc;
	char	*sbrk();
	int	sigint();
	struct	syment	*symsrch();
	int	bufaddr;

	if((mem = open(dumpfile, 0)) < 0)
		fatal("cannot open dump file");

	nmfd = open(namelist, 0);

	rdsymtab();

	Dmpstk = symsrch("dumpstack");
#if iAPX286
	Curproc = symsrch("curproc");
	U = symsrch("u");
	File = symsrch("file");
	Inode = symsrch("inode");
	Mount = symsrch("mount");
	Proc = symsrch ("proc");
	Text = symsrch("text");
	Swap = symsrch("swapmap");
	Sbuf = symsrch("sbuf");
	Core = symsrch("coremap");
	V = symsrch("v");
	Sbrpte = symsrch("sbrpte");
	Sys = symsrch("utsname");
	Time = symsrch("time");
	Lbolt = symsrch("lbolt");
	Panic = symsrch("panicstr");
	Etext = symsrch("etext");
	End = symsrch("end");
	Callout = symsrch("callout");
	Buf = symsrch("buf");
#else
	Curproc = symsrch("_curproc");
	U = symsrch("_u");
	File = symsrch("_file");
	Inode = symsrch("_inode");
	Mount = symsrch("_mount");
	Proc = symsrch ("_proc");
	Text = symsrch("_text");
	Swap = symsrch("_swapmap");
	Sbuf = symsrch("_sbuf");
	Core = symsrch("_coremap");
	V = symsrch("_v");
	Sbrpte = symsrch("_sbrpte");
	Sys = symsrch("_utsname");
	Time = symsrch("_time");
	Lbolt = symsrch("_lbolt");
	Panic = symsrch("_panicstr");
	Etext = symsrch("_etext");
	End = symsrch("_end");
	Callout = symsrch("_callout");
	vaddrinit();
#endif

#if !iAPX286
	if(readmem(&bufaddr, SYM_VALUE(Sbuf),
		sizeof bufaddr) != sizeof bufaddr) {
		Buf = symsrch("_buf");
		error("read error on _buf");
	} else {
		Sbuf->n_value = bufaddr;
#ifdef DEBUG
		printf("bufaddr = %x\n", bufaddr);
#endif
		Buf = Sbuf;
	}
#endif

	if(readmem(&v,(long)SYM_VALUE(V), sizeof v) != sizeof v)
		error("read error on v structure");
#if iAPX286
	v.ve_inode = (char *)((struct inode *)v.ve_inode - 
					(struct inode *)Inode->n_value) ;
	v.ve_file = (char *)((struct file *)v.ve_file -
					(struct file *)File->n_value) ;
	v.ve_mount = (char *)((struct mount *)v.ve_mount -
					(struct mount *)Mount->n_value) ;
	v.ve_proc = (char *)((struct proc *)v.ve_proc -
					(struct proc *)Proc->n_value) ;
	v.ve_text = (char *)((struct text *)v.ve_text -
					(struct text *)Text->n_value) ;
#else
	v.ve_inode = (char *)(((unsigned)v.ve_inode - Inode->n_value) /
		sizeof (struct inode));
	v.ve_file = (char *)(((unsigned)v.ve_file - File->n_value) /
		sizeof (struct file));
	v.ve_mount = (char *)(((unsigned)v.ve_mount - Mount->n_value) /
		sizeof (struct mount));
	v.ve_proc = (char *)(((unsigned)v.ve_proc - Proc->n_value) /
		sizeof (struct proc));
	v.ve_text = (char *)(((unsigned)v.ve_text - Text->n_value) /
		sizeof (struct text));
#endif

#if iAPX286
	if(!symsrch("con_tty")) {
#else
	if(!symsrch("_con_tty")) {
#endif
		if (!settty("dz")) 
			if (!settty("dzb"))
				printf("invalid tty structure\n");
	}
	signal(SIGINT, sigint);
}

/*
 *	Gather the uarea together and put it into the structure x.u
 *
 *	The particular uarea is selectable by the process table slot.
*/
getuarea(slot)
{
	struct	proc pbuf;
#if iAPX286
	ADDR	curproc;
#else
	int	i;
	int	sbrpte, ww, spte;
	int	curproc;
	int	upte[128];
	long	addr;
#endif

	if(slot == -1) {
		if(readmem((char *)&curproc, (long)SYM_VALUE(Curproc),
			sizeof curproc) == -1) {
				error("read error on curproc");
				return(-1);
		}
#if iAPX286
		slot = ((struct proc *)curproc - (struct proc *)Proc->n_value) ;
#else
		slot = (curproc - Proc->n_value) / sizeof(struct proc);
#endif
	}
	if(readmem((char *)&pbuf, SYM_VALUE(Proc)+slot*sizeof(struct proc),
		sizeof(pbuf)) != sizeof(pbuf)) {
			error("read error on proc table");
			return(-1);
	}

#if !iAPX286
	if(readmem((char *)&sbrpte, (long)(SYM_VALUE(Sbrpte)),
		sizeof sbrpte) != sizeof sbrpte) {
			error("read error on sbrpte");
			return(-1);
	}
	sbrpte &= VIRT_MEM;
#endif

	if ((pbuf.p_flag& (SLOAD | SSPART)) == 0) {
		return(SWAPPED);
	} else {
#if iAPX286
		if (lseek(mem,ctob((long)pbuf.p_addr),0) == -1L)
				return(-1);
		if (read(mem,(char *)&x.u,ctob((int)USIZE)) != ctob((int)USIZE))
				return(-1);
#else
		ww = (int)(pbuf.p_spt + (pbuf.p_nspt-1)*128);
		ww = sbrpte + ((ww&VIRT_MEM)>>9)*4;
		lseek(mem,(long)ww&VIRT_MEM,0);
		read(mem,&spte, sizeof(spte));
		lseek(mem,(spte<<9),0);
		read(mem, upte, sizeof upte);
		for(i=0; i<USIZE; i++) {
			lseek(mem, upte[128-USIZE+i]<<9, 0);
			/* get u page */
			if(read(mem,(char *)(((int *)&x.u)+128*i),512) != 512)
				return(-1);
		}
#endif
	}

	return(0);
}
