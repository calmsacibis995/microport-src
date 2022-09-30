	.file	"shmsys.s"
/	@(#)	1.1
	.text

#include "lib.s.h"
#define	SHMAT	0
#define	SHMCTL	1
#define	SHMDT	2
#define	SHMGET	3
	.globl	shmat
	.globl	shmctl
	.globl	shmdt
	.globl	shmget

/ shmat(shmid, shmaddr, shmflg)
/ int shmid;
/ char *shmaddr;
/ int shmflg;

shmat:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(4)	/ shmflg
	push	PARAM(3)	/ <s>shmaddr
	push	PARAM(2)	/ shmaddr
	push	PARAM(1)	/ shmid
#else
	jmp	smallshm	/ shm calls not allowed
#endif

	push	$SHMAT
	push	$SHMSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror

/ shmctl(shmid, cmd, buf)
/ int shmid, cmd;
/ struct shmid_ds *buf;

shmctl:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(4)	/ <s>buf
	push	PARAM(3)	/ buf
	push	PARAM(2)	/ cmd
	push	PARAM(1)	/ shmid
#else
/
/ shared memory can not be used unless multiple data segments are allowed
/	in the data area of the calling process.  Generate a SIGSYS signal,
/	and in case that is being ignored, return -1 with errno set to EINVAL.
/	This matches what will happen in all models when shm is not gened into
/	the system.
/
smallshm:
	push	$GETPID		/ get caller's pid to use as argument to kill
	lcall	SYSCALL
	add	$2,%sp

	push	$12		/ kill(pid, SIGSYS); 
	push	%ax
	push	$KILL
	lcall	SYSCALL
	add	$4,%sp

	mov	$22,%ax		/ errno = EINVAL;
	JMP	_cerror
#endif

	push	$SHMCTL
	push	$SHMSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller

/ shmdt(shmaddr)
/ char *shmaddr;

shmdt:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(2)	/ <s>shmaddr
	push	PARAM(1)	/ shmaddr
#else
	jmp	smallshm	/ shm calls not allowed
#endif

	push	$SHMDT
	push	$SHMSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror

/ shmget(key, size, shmflg)
/ key_t key;
/ int size, shmflg;

shmget:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(4)	/ shmflg
	push	PARAM(3)	/ size
	push	PARAM(2)	/ key(high)
	push	PARAM(1)	/ key(low)
#else
	jmp	smallshm	/ shm calls not allowed
#endif
	push	$SHMGET
	push	$SHMSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
