/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.2	*/
	/*  machdep.h: 1.8 7/18/83 */

#include	<sys/param.h>
#include	<sys/types.h>
#include	<signal.h>

#if u3b
#include	<sys/macro.h>	/* has stob, sys/seg.h below (also has ptob) */
#else
#if vax || u3b5 || iAPX286
#include	<sys/sysmacros.h>
#endif
#endif

#include	<sys/dir.h>

#if u3b
#include	<sys/seg.h>	/* needed for USRSTACK */
#include	<sys/istk.h>	/* needed for istk structure in struct user */
#else
#if u3b5
#define USRSTACK	0xf00000
#endif
#endif
#if iAPX286
#include    	<sys/seg.h>
#define NOT_ACTIVE STYP_PAD /* re-use the pad flag for not activ */
#define CSSHIFT 3       /* no of system bits to be skipped       */
#define STARTSEG 10     /* no of segments before the start of list */
#define SEGSYS 0x10 	/* indicates if code/data or system gate */
#define SEGCODE 8	/* bit set if text segment		 */
#define EXPDOWN 4       /* expand down segment			 */
#define STACKNOS 9
#define STACKNOL 11
#define SSTKSEG 0x44
#define SSTKPTR 0x400
#define WLUSRSTACK(X) (X>=(((long)(STACKNOL<<3)+7)<<16) && X<((((long)(STACKNOL+1)<<3)+7)<<16))
#define WSUSRSTACK(X) (X>=(((long)(STACKNOS<<3)+7)<<16) && X<(((((long)(STACKNOS)<<3)+7)<<16) + GSSSIZE))
#define WUSRSTACK(x)  ((MODEL(model)==M_SMALL)?(WSUSRSTACK(x)?1:0):(WLUSRSTACK(x)?1:0))
#endif
#include	<sys/user.h>
#include	<sys/errno.h>
#if vax || u3b || iAPX286
#include	<a.out.h>
#else
#if u3b5
/* CHANGE THIS WHEN NEW HEADERS ARE IN 6.0 (makefile as well) */
#include	"a.myout.h"
#endif
#endif
#include	<sys/reg.h>
#include	<sys/stat.h>

#if u3b || u3b5 || iAPX286  /* I286 NOT COMPLETE YET */
#define SUBCALL		(isubcall(dot,ISP))	/* subroutine call instruction*/
#if u3b
#define SUBRET		get(SDBREG(SP)-8, DSP)	/* subroutine ret instruction */
#else
#if iAPX286
/*#define SUBRET		(((MODEL(model)==M_SMALL)?*/
			    /*(((ADDR)SDBREG(CS))<<16):((ADDR)get(CF+4,DSP)<<16))*/
			    /*+(ADDR)get(CF+2,DSP))*/
#define SUBRET		((MODEL(model)!=M_SMALL)? ((((ADDR)get(CF+4,DSP))<<16) +(ADDR)get(CF+2,DSP)): ((((ADDR)SDBREG(CS))<<16) +(ADDR)get(CF+2,DSP)))
#define ADDR_U		((((long)8<<3)+4)<<16)
#else
extern int regvals[];

#define SUBRET		get(regvals[12]-2*WORDSIZE,DSP)
#endif
#endif
#define RETREG		0			/* register with ret value    */

/* Given the current fp (frame), these are the next frame pc, ap, fp */
#if u3b
#define XTOB(x) (ptob(x))

#define NEXTCALLPC (frame - (13*WORDSIZE))
#define NEXTARGP   (frame - (12*WORDSIZE))
#define NEXTFRAME  (frame - (11*WORDSIZE))
#ifndef	USEG
#define	USEG	0x5
#endif

#define TXTRNDSIZ	0x20000L	/* = 128K = (1L<<17)	*/

/* Address of UBLOCK (absolute) is beginning of segment 5 */
#define ADDR_U ((unsigned) (USEG * TXTRNDSIZ))

#define BKOFFSET   10	/* offset from beginning of proc to bkpt (no -O) */

#define APNO	9	/* argument pointer register number */
#define FPNO	10	/* frame pointer register number */
#else
#define XTOB(x) (ctob(x))

#if iAPX286
#define BKOFFSET 0	/* offset from the start of the routine to the break*/
#else
#define NEXTCALLPC	(frame-(9*WORDSIZE))
#define NEXTARGP	(frame-(8*WORDSIZE))
#define NEXTFRAME	(frame-(7*WORDSIZE))

#define TXTRNDSIZ	0x80000L

#define BKOFFSET	9	/* offset from start of proc to bkpt */

#define APNO		10
#define FPNO		9
#endif
#endif
#define	PROCOFFSET	0	/* offset from label to first instruction */
#define ALTBKOFFSET 0	/* offset from beginning of proc to bkpt */

#if iAPX286
#define NUMREGLS 14     /* "14" registers for 286 inc flags    */
#else
#define NUMREGLS 16	/* number of "registers" in reglist[] */
#endif

#define ISREGVAR(regno)	(3<=(regno) && (regno) <= 8)

#else
#if vax
#define XTOB(x) (ctob(x))
#define SUBCALL		((get(dot,ISP) & 0xff) == 0xfb) /* subroutine call */
#define SUBRET		get(SDBREG(SP)+16, DSP)	/* subroutine ret instruction */

#define RETREG		0			/* register with ret value */

/* Given the current fp (frame), these are the next frame pc, ap, fp */
#define NEXTCALLPC (frame + (4*WORDSIZE))
#define NEXTARGP   (frame + (2*WORDSIZE))
#define NEXTFRAME  (frame + (3*WORDSIZE))

#define TXTRNDSIZ 512L

#define ADDR_U ((unsigned) 0x7ffff800)	/* absolute address of UBLOCK */

#define	PROCOFFSET	2	/* offset from label to first instruction */
#define BKOFFSET    0	/* offset from beginning of proc to bkpt */
#define ALTBKOFFSET 0	/* offset from beginning of proc to bkpt */

#define APNO	12	/* argument pointer register number */
#define FPNO	13	/* frame pointer register number */
#define NUMREGLS 17	/* number of "registers" in reglist[] */

#define ISREGVAR(regno) (6 <= (regno) && (regno) < 12)
#endif
#endif

extern char uu[XTOB(USIZE)];

extern ADDR	callpc, frame, argp;	/* current stack frame */

#define WORDSIZE (sizeof(int))	/* wordsize in bytes on this machine */
#define REGSIZE WORDSIZE	/* register size in bytes on this machine */

#define NOBACKUP 0		/* set to 1 if machine does not back up */
			        /* to previous instruction at exception */
#if iAPX286
#define ADDRTYPE	"l"	/* type of address for getval */
#else
#define ADDRTYPE	"d"	/* type of address for getval */
#endif

#define MAXPOS	0x7ffffff		/* maximum address */

/*  two extra numbers to be printed with regs; in optab.c */
/*  removed because these are not offsets from R0; can't use SDBREG */
/*
#define VAL1	((unsigned)&(((struct user *) 0)->u_rval1)
#define VAL2	((unsigned)&(((struct user *) 0)->u_rval2)
*/

/* ptracew modes */
#define	SETTRC	0
#define	RDUSER	2
#define	RIUSER	1
#define	WDUSER	5
#define WIUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	CONTIN	7
#define	EXIT	8
#define SINGLE	9

extern REGLIST reglist [];

#if vax || u3b 
#define SDBREG(A) (((struct user *)uu)->u_ar0[A])
/* next line changed from int to char *  */
#define SYSREG(A) ((int) (((char *) (&SDBREG(A)) - ((char *) uu))))
#else
#if iAPX286
#define SDYREG(A) ((((struct user *)uu)->u_ar0[A]))
#define SDBREG(A) (((long)(((struct user *)uu)->u_ar0[A]))&0xffff)
#define SYSREG(A) ((char *) (&SDYREG(A)) - ((char *) uu) + (char *)ADDR_U)
#define SSSIZE    ((char*)(&(((struct user *)uu)->u_exdata.ux_ssize)) - ((char *) uu) + (char *)ADDR_U)
#define GSSSIZE (((long)ptrace(RUREGS,pid,SSSIZE,0))&0xffff)
#endif
#endif
#if iAPX286

#define MAXAUXEN 1

#define SF ((SDBREG(SS)<<16)+SDBREG(SP))
#define CF ((SDBREG(SS)<<16)+SDBREG(BP))
#define PCC ((SDBREG(CS)<<16)+SDBREG(IP))
#define PC PCC
#define FP CF
#define APS (FP+4)
#define APL (FP+6)
#define PCCS ((SYSREG(CS)<<16)+SYSREG(IP))
#define SUSERPC(A) SDYREG(CS)=((A>>16)&0xffff); SDYREG(IP)=(A&0xffff);
#define SFP(A) SDYREG(SS)=((A>>16)&0xffff); SDYREG(BP)=(A&0xffff);
#endif
#if iAPX286
#define NUMREGS 24      /* number of registers */
#define SEGTESIZ 8	/* number of bytes in segment table entry */
#else
#define NUMREGS 16	/* number of general purpose registers */
#endif

#if vax || u3b || iAPX286
#define ISREGN(reg)	(0<= (reg) && (reg) < NUMREGS)
#else
#if u3b5
#define ISREGN(reg)	(0 <= (reg) && (reg) < NUMREGS && (reg) != 13 && (reg) != 14)
#endif
#endif

#if vax || u3b
#define USERPC  SDBREG(PC)
#else
#if u3b5
#define USERPC	regvals[15]
#else
#if iAPX286
#define USERPC  PCC
#endif
#endif
#endif

union word {
	char c[WORDSIZE]; 
	unsigned char cu[WORDSIZE];
	short s[WORDSIZE/2];
	unsigned short su[WORDSIZE/2];
	int w;
	unsigned int iu;
	long l;
#if iAPX286
	unsigned long lu;
	double	d;
#endif
	float f;
};
union dbl {
#if iAPX286
	struct {
		long w1,w2;
	} ww;
	long l;
#else
	struct {
		int w1, w2;
	} ww;
#endif
	double dd;
	int i;
	float f;
	char cc[WORDSIZE*2];
};
