	.file	"msgsys.s"
/	@(#)	1.1
	.text

#include "lib.s.h"
#define	MSGGET	0
#define	MSGCTL	1
#define	MSGRCV	2
#define	MSGSND	3
	.globl	msgctl
	.globl	msgget
	.globl	msgrcv
	.globl	msgsnd

/ msgget(key, msgflg)
/ key_t key;
/ int msgflg;

msgget:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(3)	/ msgflg
	push	PARAM(2)	/ key(high)
	push	PARAM(1)	/ key(low)

	push	$MSGGET
	push	$MSGSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror

/ msgctl(msqid, cmd, buf)
/ int msqid, cmd;
/ struct msqid_ds *buf;

msgctl:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(4)	/ <s>buf
	push	PARAM(3)	/ buf
	push	PARAM(2)	/ cmd
	push	PARAM(1)	/ msqid
#else
	push	%ds		/ <s>buf
	push	PARAM(3)	/ buf
	push	PARAM(2)	/ cmd
	push	PARAM(1)	/ msqid
#endif

	push	$MSGCTL
	push	$MSGSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller

/ msgrcv(msqid, msgp, msgsz, msgtyp, msgflg)
/ int msqid;
/ struct msgbuf *msgp;
/ int msgsz;
/ long msgtyp;
/ int msgflg;

msgrcv:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(7)	/ msgflg
	push	PARAM(6)	/ msgtyp(high)
	push	PARAM(5)	/ msgtyp(low)
	push	PARAM(4)	/ msgsz
	push	PARAM(3)	/ <s>msgp
	push	PARAM(2)	/ msgp
	push	PARAM(1)	/ msqid
#else
	push	PARAM(6)	/ msgflg
	push	PARAM(5)	/ msgtyp(high)
	push	PARAM(4)	/ msgtyp(low)
	push	PARAM(3)	/ msgsz
	push	%ds		/ <s>msgp
	push	PARAM(2)	/ msgp
	push	PARAM(1)	/ msqid
#endif

	push	$MSGRCV
	push	$MSGSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror

/ msgsnd(msqid, msgp, msgsz, msgflg)
/ int msqid;
/ struct msgbuf *msgp;
/ int msgsz, msgflg;

msgsnd:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(5)	/ msgflg
	push	PARAM(4)	/ msgsz
	push	PARAM(3)	/ <s>msgp
	push	PARAM(2)	/ msgp
	push	PARAM(1)	/ msqid
#else
	push	PARAM(4)	/ msgflg
	push	PARAM(3)	/ msgsz
	push	%ds		/ <s>msgp
	push	PARAM(2)	/ msgp
	push	PARAM(1)	/ msqid
#endif

	push	$MSGSND
	push	$MSGSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
