static char *uportid = "@(#)trap.c	Microport Rev Id 1.3.8  10/19/86";
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)trap.c	1.39 */
/*		Copyright 1985 Microport Systems
 *		All Rights Reserved.
 *
 *	Modification History:
 *	Upgraded for the IBM AT. This consisted of:
 *		1) Moving strayint()'s printf to *beginning* of the function.
 *  		2) more useful kernel error printouts
 *		3) Changed EXTERRFLT trap cases to handle PIC eoi 
 *		   and clear fp fault.
 *	M001:	Clear busy condition on floating point chip
 */


#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/trap.h"
#include "sys/seg.h"
#include "sys/sysinfo.h"
#include "sys/mmu.h"
#include "sys/fp.h"
#ifdef ATMERGE
#include "sys/8259.h"
#endif /* ATMERGE */

#define	USER	0x4000		/* user-mode flag added to type */
#define	NSYSENT	64

extern int		*nofault;
unsigned int		traptype;
#ifdef	GDEBUGGER
extern int db_trace;	/* flag indicates kernel debugger trace exception */
#endif

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap( type, frame, err, cs )
unsigned int	type;
unsigned int	err;
unsigned int	cs;
int		*frame;
{
	int *saveframe;			/* needed if trap in system mode */
	static	dblfault = 0;		/* hardware doesn't quite work */
	register i;
	time_t syst;
	int	code = 0;
	int indx;
	int indx1;
	int *stkadr;
	/* The variable code ends up being passed as the second
	 * argument to the the user's signal handling routine.
	 * The user should consider a zero value to be meaningless.
	 */

	syst = u.u_stime;
	traptype = type;

	/*
	 * if this is a trap in user mode, u.u_ar0 is ok to overwrite.
	 * otherwise, this may be a trap on top of a trap, and u.u_ar0
	 * must be saved before overwriting, then restored on exit.
	 */
	if (USERMODE(cs))
		type |= USER;
	else
		saveframe = u.u_ar0;
	u.u_ar0 = frame;

	switch (type) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 */
	case NOSEGFLT:			/* segment not present	*/
		if ( nofault )
			longjmp( nofault );
		panicstr = "segment not present";
		goto noway;

	case GPFLT:			/* general protection	*/
		if ( nofault )
			longjmp( nofault );
		panicstr = "general protection";
		goto noway;

	case DBLFLT + USER:		/* double fault		*/
		printf( "User mode:\n" );
	case DBLFLT:		
		panicstr = "double fault";
		goto noway;

	default:
		panicstr = "trap";	/* fake it for printfs */
noway:
#ifdef	GDEBUGGER
		while (1) {
		    dbg (type, frame, err, cs);
		    printf ("No restart possible!\n");
		}
#endif
		if (dblfault) {
			panic("Software detects double fault!");
		} else dblfault = 1;
		printf("user = ");
		printf("0x%x ", u.u_procp->p_addr);
		printf("\n");

#ifdef IBMAT
		printf("cs=0x%x,ds=0x%x,es=0x%x,ss=0x%x,di=0x%x,si=0x%x\n", 
		cs, frame[DS], frame[ES], frame[SS], frame[DI], frame[SI]);
	printf("bp=0x%x,bx=0x%x,dx=0x%x,cx=0x%x,ax=0x%x,ip=0x%x,flags=0x%x\n", 
	       frame[BP], frame[BX], frame[DX], frame[CX], frame[AX], frame[IP],
	       frame[FLGS]);
		printf("trap type 0x%x\n", type);
		printf("err = 0x%x\n", err);
		printf("stack frame address = %lx\n",(long)stkadr);
		delay(250);
/* This should trace back along procedure calls, it's more useful */
		for (indx = 0; indx < 10; indx++) {
			for (indx1 = 0; indx1< 10; indx1++) {
				printf("%x ",(int)*stkadr++);
			}
			printf("\n");
		}
#endif IBMAT
		panic(NULL);

	case BADTSSFLT + USER:	/* bad TSS fault		*/
		code = 1;	/* tell user what error		*/
		i = SIGBUS;
		break;
	case NOSEGFLT + USER:	/* segment not present		*/
		code = 2;	/* tell user what error		*/
		i = SIGSEGV;
		break;
	case GPFLT + USER:	/* general protection fault	*/
		code = 3;	/* tell user what error		*/
		if ( err == 0 )
			i = SIGSEGV;
		else
		{
			if ( err & LDT_TI )
				i = SIGSEGV;
			else
				i = SIGBUS;
		}
		break;

	case INVOPFLT + USER:	/* invalid opcode fault */
		i = SIGILL;
		break;

	case SYSCALL + USER:	/* sys call */
	{
		register struct sysent *callp;
		struct a {
			caddr_t	dirp;
		};
		int	*regs;

		sysinfo.syscall++;
		u.u_error = 0;
		regs = u.u_ar0;
		regs[FLGS] &= ~PS_C;
		if(u.u_intflg & TR_OFF) {
			u.u_intflg &= ~TR_OFF;
			regs[FLGS] |= PS_T;
		}
		i = regs[ARG0];
		if (i >= NSYSENT)
			i = 0;
		callp = &sysent[i];
		u.u_rval1 = 0;
		u.u_rval2 = regs[DX];
		u.u_ap = &regs[ARG1];
		u.u_dirp = (caddr_t)((struct a *)u.u_ap)->dirp;
		if (setjmp(u.u_qsav)) {
			if (u.u_error==0)
				u.u_error = EINTR;
		} else {
			(*callp->sy_call)();
		}
		if (u.u_error) {
			regs[AX] = u.u_error;
			regs[FLGS] |= PS_C;	/* carry bit */
			if (++u.u_errcnt > 16) {
				u.u_errcnt = 0;
				runrun++;
			}
		} else {
			regs[AX] = u.u_rval1;
			regs[DX] = u.u_rval2;
		}
		/*
		** copy system call return address to the right place
		** for addupc and return to user
		*/
		regs[IP] = regs[SCIP];
		regs[CS] = regs[SCCS];
	}
	calcpri:
	{
		register struct proc *pp;

		pp = u.u_procp;
		pp->p_pri = (pp->p_cpu>>1) + PUSER + pp->p_nice - NZERO;
		curpri = pp->p_pri;
		if (runrun == 0)
			goto out;
	}

	case RESCHED + USER:	/* Allow process switch */
		qswtch();
		goto out;


	case DIVERR + USER:	/* divide error		*/
		code = 3;	/* tell user what error	*/
		i = SIGFPE;
		break;
	case INTOFLT + USER:	/* overflow error	*/
		code = 2;	/* tell user what error	*/
		i = SIGFPE;
		break;
	case BOUNDFLT + USER:	/* bound fault 		*/
		code = 1;	/* tell user what error	*/
		i = SIGFPE;
		break;

	/*
	 * If the user SP is below the stack segment,
	 * grow the stack automatically.
	 */
	case STKFLT + USER:	/* stack fault		*/
		/*
		** If the error code is null, the error must
		** have been stack overflow, or underflow.
		** Otherwise it must have been a stack segment not present.
		** In this case, kill the user.
		*/
		if ( err == 0 )
		{
			/*
			** If this was an overflow, try to grow the
			** stack.
			*/
			if ( ( u.u_ar0[ SP ] != 0xFFFF ) && 
			 	( u.u_ar0[ SP ] != 0xFFFE ) )
			{
				if ( grow( u.u_ar0[ SP ] ) )
					goto calcpri;
			}
		}
		i = SIGSEGV;
		break;

	case SGLSTP:		/* single step 			*/
#ifdef	GDEBUGGER
		if (db_trace) {
		    dbg (type, frame, err, cs);
		    goto out;
		}
#endif
		/*
		** if I had to turn off the trace bit because
		** it was set on entry to a syscall, set a flag to
		** notify syscall common code that I turned it off
		*/
		u.u_intflg |= TR_OFF;
		u.u_ar0[ FLGS ] &= ~PS_T;	/* turn off trace bit	*/
		goto out;

	case BPTFLT:			/* breakpoint instruction fault	*/
#ifdef	GDEBUGGER
		dbg (type, frame, err, cs);
		goto out;
#endif
	case SGLSTP + USER:		/* single step trap		*/
	case BPTFLT + USER:		/* breakpoint instruction fault */
singlestep:
		u.u_ar0[ FLGS ] &= ~PS_T;	/* turn off trace bit	*/
		i = SIGTRAP;
		break;

	case NMIFLT + USER:	/* NMI			*/
		printf( "NMI in user mode\n" );
		goto out;

	case NMIFLT:		/* NMI			*/
		printf( "NMI in system mode\n" );
		goto out;

#ifndef	PICFIX1
	case INTERRUPT:
		printf( "INTERRUPT in system mode\n" );
		eoi( type );
		goto out;

	case INTERRUPT + USER:
		printf( "INTERRUPT in user mode\n" );
		eoi( type & ~USER );
		goto out;
#endif /* ! PICFIX1 */ 

	case NOEXTFLT:		/* extension not avail	*/
	case NOEXTFLT + USER:
	{
		unsigned int	ts;

		/*
		** this fault is the one used to implement demand floating
		** point register saves and restores. If the task switched
		** flag is set, an interrupt of NOEXTFLT will be generated
		** the first time a process attempts to execute an 80287
		** instruction. This allows us to save the previous process'
		** floating point state, and restore the current process'
		** state ( if it is valid ).
		*/

		/*
		** get the current state of the
		** task switched flag, and reset it
		*/
		ts = clts();

		/*
		** if we don't have either a 287 or an emulator,
		** kill the user
		*/
		if ( fp_kind != FP_HW && fp_kind != FP_SW )
		{
			code = 4;
			i = SIGFPE;
			break;
		}

		/*
		** if we are not emulating, and
		** if the task switch flag was not set,
		** something is wrong, because the flag
		** had to be set for us to be in this
		** code
		*/
		if ( ( fp_kind == FP_HW ) && ( ! ts ) )
		{
			printf("ext. not avail.: ts not set\n");
			goto out;
		}

		/*
		** if somebody is using the fp unit ( fp_proc
		** is non-zero ), and if it is not the current
		** process, save the fp state to fp_proc's
		** user structure
		*/
		if ( fp_proc )
		{
			if ( fp_proc == u.u_procp )
			{
				/*
				** current process owns the fp
				** unit, see if we have to emulate
				*/
				goto emulate;
			}
			else
			{
				/*
				** somebody else owns fp unit
				** so save state into fp_proc's
				** user struct, and clear the
				** exception condition
				*/
				fpsave( FPNOCHECK );
				fpclex();
			}
		}

		/*
		** current process now owns fp unit
		*/
		fp_proc = u.u_procp;

		/*
		** if current process' state is valid, restore
		** it. Otherwise, this is the first time this
		** process has executed a fp instruction,
		** so initialize the fp unit for him
		*/
		if ( u.u_fpvalid )
			fprestore();
		else
			fpinit();

emulate:
		/*
		** if we have to emulate the instruction, go
		** do it
		*/
		if ( fp_kind == FP_SW )
		{
			emul_entry( u.u_ar0[ IP ], u.u_ar0[ CS ] );

			/*
			** if the trace bit is set, we have to notify
			** the parent ( by signalling the child )
			*/
			if ( u.u_ar0[ FLGS ] & PS_T )
				goto singlestep;
		}

		goto out;
	}

	case EXTERRFLT:
	case EXTERRFLT + USER:	/* extension fault 	*/
#ifndef	PICFIX1			/* not entered through I/O interface */
		asm("	cli");	
		eoi(45);
		asm("	sti");
#endif /* ! PICFIX1 */ 
	/*	asm("	fnclex");  */
		outb(0xF0, 0);	  /* Clear busy condition M001 */
		asm("	fclex");  /* Clear error condition on chip */
		outb(0xF0, 0);	  /* Clear busy condition M001 */
		i = SIGFPE;
		code = 6;
		goto fpflt;

	case EXTOVRFLT:		/* extension overrun	*/
	case EXTOVRFLT + USER:
	{
		struct proc	*tproc;
		unsigned int	ts;

		i = SIGSEGV;
		code = 5;
fpflt:

		/*
		** save the current fp processes proc struct
		*/
		tproc = fp_proc;

		/*
		** get the current state of the
		** task switched flag, and reset it
		*/
		ts = clts();

		/*
		** if nobody is using the fp unit,
		** something is wrong
		*/
		if ( fp_proc == 0 )
		{
			printf( "ext. error/overrun - no process\n" );
			goto out;
		}

		/*
		** if the process using the fp unit is
		** the current process, send the current
		** process the signal
		*/
		if ( fp_proc == u.u_procp )
		{
			break;
		}
		else
		{
			/*
			** current process is not the process
			** who was using the fp unit, so if
			** current process has a valid fp
			** state, restore it
			*/
			if ( ts && u.u_fpvalid )
			{
				fp_proc = u.u_procp;
				fprestore();
			}
			else
			{
				/*
				** current process does not
				** have a valid fp state, so
				** signify that nobody is using
				** fp unit now.
				*/
				fp_proc = 0;
			}
		}
		/*
		** send a signal to either the current or the
		** previous process. WARNING: If the signal
		** is sent to the previous process, 'code' will
		** not be sent to him.
		*/
		psignal( tproc, i );
		goto out;

	} /* end EXTOVRFLT case */

	} /* end trap type switch */

	psignal(u.u_procp, i);

out:
	if (USERMODE(cs)) {
		if (u.u_procp->p_sig && issig())
			psig(code);
		if (u.u_prof.pr_scale)
			addupc((caddr_t)(*(int (**)())(&u.u_ar0[IP])),
					&u.u_prof, (int)(u.u_stime-syst));
	}
	else {
		/*
	 	* if this was a trap in system mode,
	 	* u.u_ar0 must be restored to its previous state.
	 	*/
		u.u_ar0 = saveframe;
	}
}

/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
	psignal(u.u_procp, SIGSYS);
}

/*
 * Ignored system call
 */
nullsys()
{
}

strayint(vec)
{
	printf("stray interrupt - vector 0x%x\n", vec);
	logstray(vec);
#ifndef	PICFIX1
	eoi( vec );
#endif /* ! PICFIX1 */ 
}

/*
** tssfault
**	separate task to kill off user, or panic
*/
tssfault()
{
	static struct tss 	*tssptr;/* pointer to tss back link	      */
	int			killuser();
	extern struct seg_desc	gdt[];
	extern struct tss	tftss;	/* tssfault tss			      */

top:
	/*
	** disable interrupts,
	** and throw away the selector that the 286
	** pushed on my stack
	*/
	asm( "	cli		" );
	asm( "	pop	%ax	" );

	/*
	** get a ptr to back link, + DSC_SZ because we have to look 
	** at tss alias
	*/
	tssptr = (struct tss *)( (long)( tftss.ts_back + DSC_SZ ) << 16 );	

	/*
	** Find out whether we were in system or
	** user mode as of the fault. This is done by
	** examining the back link of our tss and getting
	** the cs out of it.
	*/
	if ( USERMODE( tssptr->ts_cs ) )
	{
		/*
		** If we were in user mode, set up his tss to point
		** to killuser(), so that when we iret back to
		** his task ( because the NT flag is set ), he
		** will die
		*/
		tssptr->ts_flags &= ~PS_NT;	/* turn off nested task	*/
		tssptr->ts_ss = ( UPAGE_SEL << 3 ) | LDT_TI;
		tssptr->ts_sp = 0x400;
		tssptr->ts_cs = (unsigned long)killuser >> 16;
		tssptr->ts_ip = (int)killuser;
		asm( "	iret" );
		goto top;
	}
	asm( "	sti" );
	panic( "tss fault" );
}

killuser()
{
	exit( core() ? ( 0200 + SIGBUS ) : SIGBUS );
	/* no deposit, no return */
}

/*
** dblfault
*/
dblfault()
{
	panic( "double fault" );
}

