/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)u.c	1.4 - 85/08/13 */
#include	"crash.h"
#include	"sys/acct.h"
#include	"sys/file.h"
#include	"sys/lock.h"
#include	"sys/proc.h"


#if iAPX286
#define TSS	x.u.u_tss
#else
#define	PCB	x.u.u_pcb
#endif
#define	AP	(ip + 2)
#define	FP	(ip + 3)
#define	PC	(ip + 4)
#define	R5	(fp->f_r5 & 03777)	/* Overlay no. in top 5 bits */
#define STBASE  (U->n_value + sizeof u) /* Stack doesn't grow past here */
#ifdef	vax
#define	STKHI	(U->n_value + sizeof u) /* Stack doesn't grow past here */
#define	STKLO	(U->n_value + USIZ)	/* Start of stack */
#define	STKMASK	STKHI
#else
#define	STKMASK	0140000
#define	STKHI	0170000
#endif
#if iAPX286
char *ctime();
#endif

struct	user	u;
extern	struct	uarea	x;
#ifdef vax
extern	unsigned Kfp;
#else
#if iAPX286
extern  ADDR    Kfp;
#else
extern	int	Kfp;
#endif
#endif

pruarea(procslot)
int	procslot;
{
	register  int  i;
	register  int	ret;

	if(procslot > (int)v.ve_proc) {
		printf("%d out of range, use Process Table Slot\n", procslot);
		return(-1);
	}
	if((ret=getuarea(procslot)) == -1) {
		error("bad read of uarea");
		return(-1);
	}
	else if(ret == SWAPPED) {
		error("process is swapped");
		return(-1);
	}

	printf("PER PROCESS USER AREA:\n");
	printf("USER ID's:\t");
	printf("uid: %u, gid: %u, real uid: %u, real gid: %u\n",
		x.u.u_uid, x.u.u_gid, x.u.u_ruid, x.u.u_rgid);
	printf("PROCESS TIMES:\t");
	printf("user: %ld, sys: %ld, child user: %ld, child sys: %ld\n",
		x.u.u_utime, x.u.u_stime, x.u.u_cutime, x.u.u_cstime);
	printf("PROCESS MISC:\t");
	printf("proc slot: %d",
#if iAPX286
		((struct proc *)x.u.u_procp - (struct proc *)Proc->n_value));
#else
		((unsigned)x.u.u_procp - Proc->n_value) / sizeof (struct proc));
#endif
	if(x.u.u_ttyd != 0)
		printf(", cntrl tty: maj(%.2d) min(%2.2d)\n",
			major(x.u.u_ttyd), minor(x.u.u_ttyd));
	else
		printf(", cntrl tty: maj(%s) min(%s)\n", "??","??");
	printf("IPC:\t\t");
	printf("locks:%s%s%s%s",
		((x.u.u_lock & UNLOCK) == UNLOCK) ? " unlocked" : "",
		((x.u.u_lock & PROCLOCK) == PROCLOCK) ? " proc" : "",
		((x.u.u_lock & TXTLOCK) == TXTLOCK) ? " text" : "",
		((x.u.u_lock & DATLOCK) == DATLOCK) ? " data" : "");
#ifdef	pdp11
	printf("FILE I/O:\tuser addr: %0.1o ",x.u.u_base);
#endif
#ifdef	vax
	printf("FILE I/O:\tuser addr: %0.1x ",x.u.u_base);
#endif
#if iAPX286
	printf("\nFILE I/O:\tuser addr: %0.1lx ",x.u.u_base);
#endif
	printf("file offset: %ld, bytes: %u,\n", x.u.u_offset, x.u.u_count);
	printf("\t\tsegment: %s,", x.u.u_segflg == 0 ? "data" :
		(x.u.u_segflg == 1 ? "sys" : "text"));
	printf(" umask: %#03o, ulimit: %ld\n", x.u.u_cmask, x.u.u_limit);
	printf("ACCOUNTING:\t");
#ifdef vax
	printf("command: %.14s, memory: %ld, type: %s%s%s%s\n\t\tstart: %s",
#else
#if iAPX286
	printf("command: %.14s, memory: %ld, type: %s%s%s\n\t\tstart: %s",
#else
	printf("command: %.14s, memory: %ld, type: %s%s%s%s%s%s\n\t\tstart: %s",
#endif
#endif
		procslot ? x.u.u_comm : "swapper",
		x.u.u_mem, x.u.u_acflag & AFORK ? "fork" : "exec",
		x.u.u_acflag & ASU ? " su-user" : "",
#ifdef vax
		x.u.u_exdata.ux_mag == VAXWRMAGIC ? " lpd" : "",
		x.u.u_exdata.ux_mag == VAXROMAGIC ? " rd/only-txt" : "",
#else
#if iAPX286
		x.u.u_model == U_MOD_SMALL?"SMALL":
			(x.u.u_model == U_MOD_LARGE?"LARGE":"HUGE"),
#else
		x.u.u_exdata.ux_mag == A_MAGIC0 ? " lpd" : "",
		x.u.u_exdata.ux_mag == A_MAGIC2 ? " rd/only-txt" : "",
		x.u.u_exdata.ux_mag == A_MAGIC3 ? " sep-I/D" : "",
		x.u.u_exdata.ux_mag == A_MAGIC4 ? " overlay" : "",
#endif
#endif

		ctime(&x.u.u_start));
	printf("OPEN FILES:\t");
	printf("file desc: ");
	for(i = 0; i < NOFILE; i++)
#if iAPX286
		if(x.u.u_ofile[i] != NULL)
#else
		if(x.u.u_ofile[i] != 0)
#endif
			printf(" %3d", i);
	printf("\n\t\tfile slot: ");
	for(i = 0; i < NOFILE; i++)
#if iAPX286
		if(x.u.u_ofile[i] != NULL)
			printf(" %3d", ((char *)x.u.u_ofile[i] -
				(char *)File->n_value) / sizeof (struct file));
#else
		if(x.u.u_ofile[i] != 0)
			printf(" %3d", ((unsigned)x.u.u_ofile[i] -
				File->n_value) / sizeof (struct file));
#endif
	printf("\n");
}

prpcb(procslot)
int	procslot;
{
	register  int	ret;

#ifdef	pdp11
	printf("process control block not implemented on pdp11\n");
	return;
#endif

#if iAPX286
	if(procslot > (int)v.ve_proc) {
		printf("%d out of range, use Process Table Slot\n", procslot);
		return(-1);
	}
	if((ret=getuarea(procslot)) == -1) {
		error("bad read of uarea");
		return(-1);
	}
	else if(ret == SWAPPED) {
		error("process is swapped");
		return(-1);
	}

	if(procslot == -1)
		printf("\nPROCESS CONTROL BLOCK (TSS): for current process\n");
	else
		printf("\nPROCESS CONTROL BLOCK (TSS): for process %d\n",procslot);
	printf("\tsp0: %04x ss0: %04x sp1: %04x ss1: %04x sp2: %04x ss2: %04x\n"		,TSS.ts_sp0,TSS.ts_ss0,TSS.ts_sp1,TSS.ts_ss1,TSS.ts_sp2,
		TSS.ts_ss2);
	printf("\tblink: %04x ldt:   %04x flags: %04x cs:    %04x ip:    %04x\n"
		,TSS.ts_back,TSS.ts_ldt,TSS.ts_flags,TSS.ts_cs,TSS.ts_ip);
	printf("\tax:    %04x cx:    %04x dx:    %04x bx:    %04x\n",
		TSS.ts_ax,TSS.ts_cx,TSS.ts_dx,TSS.ts_bx);
	printf("\tsp:    %04x bp:    %04x si:    %04x di:    %04x\n",
		TSS.ts_sp,TSS.ts_bp,TSS.ts_si,TSS.ts_di);
	printf("\tes:    %04x cs:    %04x ss:    %04x ds:    %04x\n",
		TSS.ts_es,TSS.ts_cs,TSS.ts_ss,TSS.ts_ds);

#endif

#ifdef	vax
	if(procslot > (int)v.ve_proc) {
		printf("%d out of range, use Process Table Slot\n", procslot);
		return(-1);
	}
	if((ret=getuarea(procslot)) == -1) {
		error("bad read of uarea");
		return(-1);
	}
	else if(ret == SWAPPED) {
		error("process is swapped");
		return(-1);
	}

	printf("\nPROCESS CONTROL BLOCK:\n");
	printf("\tkfp: %08x esp: %08x ssp: %08x usp: %08x\n",
		PCB.pcb_ksp, PCB.pcb_esp, PCB.pcb_ssp, PCB.pcb_usp);
	printf("\tr0:  %08x r1:  %08x r2:  %08x r3:  %08x r4:  %08x\n",
		PCB.pcb_r0, PCB.pcb_r1, PCB.pcb_r2, PCB.pcb_r3, PCB.pcb_r4);
	printf("\tr5:  %08x r6:  %08x r7:  %08x r8:  %08x r9:  %08x\n",
		PCB.pcb_r5, PCB.pcb_r6, PCB.pcb_r7, PCB.pcb_r8, PCB.pcb_r9);
	printf("\tr10: %08x r11: %08x r12: %08x r13: %08x\n",
		PCB.pcb_r10, PCB.pcb_r11, PCB.pcb_r12, PCB.pcb_r13);
	printf("\tpc:  %08x psl: %08x\n",
		PCB.pcb_pc, PCB.pcb_psl);
	printf("\tp0br: %08x p0lr: %08x p1br: %08x  p1lr: %08x\n",
		PCB.pcb_p0br,PCB.pcb_p0lr,PCB.pcb_p1br,PCB.pcb_p1lr);
#endif
}

prstack(procslot)
int	procslot;
{
	register  int	*ip;
	register  int	ret;
#if iAPX286
	int	i,j;
#endif

	if(procslot > (int)v.ve_proc) {
		printf("%d out of range, use Process Table Slot\n", procslot);
		return(-1);
	}
	if((ret=getuarea(procslot)) == -1) {
		error("bad read of uarea");
		return(-1);
	}
	else if(ret == SWAPPED) {
		error("process is swapped");
		return(-1);
	}
	printf("KERNEL STACK:\n");

#if iAPX286
	for (i=0;i < KSTACKSZ;i++)
	{
		if((i%8) == 0)
		{
			for(j=0;((j<8)&&(x.u.u_stack[i+j]==0));j++) ;
			if( (j==8) && (i > 0))
			{
				i += 7;
				continue;
			}
			printf("\n");
			printf(FMT, ((long)(((UPAGE_SEL)<<3) + LDT_TI)<<16) +
						((unsigned long)i<<1));
			printf(":\t");
		}
		printf("  %.4x",x.u.u_stack[i]);
	}
	printf("\n");
#else
#define X	x.stk[0]
	for(ip = (int *)x.stk; ip != (int *)&x.stk[USIZ - sizeof u]; ip++)
		if(*ip)
			break;

	while(((unsigned) ip - (unsigned) &X + STBASE) % 16)
		ip--;	/* back up to even boundry */

	while(((unsigned) ip - (unsigned) &X + STBASE) % 16)
		ip--;	/* back up one more line */
#ifdef DEBUG
	printf("ip=%x, &x.stk[USIZ-sizeof u]=%x\n", ip, &x.stk[USIZ-sizeof u]);
#endif

	for(; ip != (int *)&x.stk[USIZ - sizeof u];) {
		if(((unsigned)ip - (unsigned)&X + STBASE) % 16 == 0) {
			printf("\n");
			printf(FMT, (unsigned)ip - (unsigned)&X + STBASE);
			printf(":\t");
		}
		printf("  ");
		printf(NBPW==4 ? "%8x" : "%6o", *ip++);
	}
	printf("\n");
#ifdef	pdp11
	printf("\nSTACK FRAMES:\n");
	printf("<-- r2 r3 r4 r5 ret args auto var r2 r3 r4 r5 ");
	printf("ret dev sp r1 nps r0 pc ps\n");
	printf("HW r5 _______|^|____________________________|^\n");
#endif
	printf("\n");
#ifdef	vax
	printf("STACK FRAME:\n");
	printf("|- - - - - - - - - - ->|\tKFP == %x\n",PCB.pcb_ksp);
	printf("________________________________________________________________________\n");
	printf("|  fp  |  pc  |  regs  |  cond  |   psl  |  ap  |  fp  |  pc  |  regs  |\n");
	printf("|______|______|________|________|________|______|______|______|________|\n");
#endif
#endif
}

prtrace(ptr,procslot)
	int	procslot;
	int	ptr;	/* If ptr = 1, use Kfp */
{
	register  int  *ip;
	register  int	ret;
	struct	syment	*sp, *search();
#if iAPX286
	ADDR csip;
	ADDR stackp;
	unsigned i,j;
#endif
#ifdef pdp11
	register int	funny;
	register struct frame	*fp;
#endif

	if(procslot > (int)v.ve_proc) {
		printf("%d out of range, use Process Table Slot\n", procslot);
		return(-1);
	}
	if((ret=getuarea(procslot)) == -1) {
		error("bad read of uarea");
		return(-1);
	}
	else if(ret == SWAPPED) {
		error("process is swapped");
		return(-1);
	}


#ifdef	pdp11
	if(ptr == 1) {
		printf("R5 STACK TRACE:\n");
		if((Kfp & STKHI) != STKMASK) {
			printf("\tHW R5 invalid\n");
			return;
		}
		fp = (unsigned)&x + (unsigned)(Kfp & ~STKMASK);
		do {
			sp = search(fp->f_ret, 042, 042);
			if(sp == 0)
				printf("\tno match\n");
			else if(sp->n_name[0] == '_')
				printf("\t%.7s\n", &sp->n_name[1]);
			else
				printf("\t%.8s\n", sp->n_name);
			if((fp->f_r5 & STKHI) != STKMASK) {
				printf("\tcurdled stack\n");
				return;
			}
			fp = (unsigned)&x + (unsigned)(fp->f_r5 & ~STKMASK);
		} while((fp->f_r5 & STKHI) == STKMASK);
		return;
	}
	printf("STACK TRACE:\n");
	funny = ((x.stk[USIZE * 64 - sizeof u - 1]) & 0377) == 0360 ?
		18 : 16;
	ip = fp = &x.stk[USIZE * 64 - sizeof u - funny];
	sp = search(fp->f_ret, 042, 042);
	if(sp == 0)
		printf("\tno match\n");
	else if(sp->n_name[0] == '_')
		printf("\t%.7s\n", &sp->n_name[1]);
	else
		printf("\t%.8s\n", sp->n_name);
	--ip;
	while(--ip != x.stk)
		if(*ip == STKMASK + (unsigned)fp - (unsigned)&x) {
			if(sp = search(((struct frame *)ip)->f_ret, 042, 042)) {
				fp = ip;
				if(sp->n_name[0] == '_')
					printf("\t%.7s\n", &sp->n_name[1]);
				else
					printf("\t%.8s\n", sp->n_name);
			}
		}
#endif

#ifdef	vax
	if(ptr) {
		printf("KFP STACK TRACE:\n");
		if(Kfp < STKHI || Kfp > STKLO) {
#ifdef DEBUG
	printf("STKHI=%x, STKLO=%x, Kfp=%x\n", STKHI, STKLO, Kfp);
	printf("Uvalue=%x, USIZ=%d, sizeof u=%d\n", U->n_value,USIZ,sizeof u);
#endif
			printf("\tInvalid KFP\n");
			return(-1);
		}
		ip = (int *)((Kfp - U->n_value) + (int)&x);
	}
	else {
		printf("STACK TRACE:\n");
		ip = (int *)((PCB.pcb_ksp - U->n_value) + (int)&x);
	}

#ifdef	DEBUG
	printf("ip=%x psl=%x ap=%x fp=%x pc=%x\n",*ip,*(ip+1),*AP,*FP,*PC);
#endif
	if((*FP < STKHI || *FP > STKLO) || (*AP < STKHI || *AP > STKLO)) {
		printf("\tcurdled stack\n");
		return(-1);
	}

	for(; ip < (int *)&x.stk[USIZ]; ) {

#ifdef	DEBUG
	printf("==>ip=%x psl=%x ap=%x fp=%x pc=%x\n",*ip,*(ip+1),*AP,*FP,*PC);
#endif
		if(*FP < USRSTACK)	/* i.e. points outside uarea */
			return;

#ifdef vax
		sp = search(*PC, N_TEXT, N_TEXT);
#else
		sp = search(*PC, (N_TEXT | N_EXT), (N_TEXT | N_EXT));
#endif

		if(sp == 0)
			printf("\tno match\n");
		else if(sp->n_name[0] == '_')
			printf("\t%.7s\n", &sp->n_name[1]);
		else
			printf("\t%.8s\n", sp->n_name);

		ip = (int *)((*FP - U->n_value) + (int)&x);

	}
#endif
#if iAPX286
#define KSTACK   ((unsigned long)((UPAGE_SEL<<3)+LDT_TI)<<16)
	if (ptr)
	{
		printf("KFP STACK TRACE:\n");
		stackp=Kfp;
	}
	else
	{
		printf("STACK TRACE:\n");
		stackp=((unsigned long)TSS.ts_ss<<16)+(unsigned long)TSS.ts_bp;	
	}
	if ((stackp > (KSTACK + (KSTACKSZ<<1))) 
		|| (stackp < KSTACK))
	{
		printf("\t Invalid stack trace pointer \n");
		return(-1);
	}
	i = ((unsigned)stackp)>>1;
	for(;i < KSTACKSZ;)
	{
		printf("prev. frame = %x, cs = %4x, ip = %4x \n",x.u.u_stack[i],
					x.u.u_stack[i+2],x.u.u_stack[i+1]);
		csip=((unsigned long)x.u.u_stack[i+2]<<16)
					|(unsigned)x.u.u_stack[i+1];
		sp = search(csip,N_TEXT,N_TEXT);
		if (sp == 0)
			printf("\tno match\n");
		else
			printf("\t%.8s\n", sp->n_name);
		j = x.u.u_stack[i]>>1;
		if (j <= (i+2))
			return;
		i = j;
	}
	return;
#endif
}
