	.file	"semsys.s"
/	@(#)	1.1
	.text

#include "lib.s.h"
#define	SEMCTL	0
#define	SEMGET	1
#define	SEMOP	2
	.globl	semctl
	.globl	semget
	.globl	semop

/ semctl(semid, semnum, cmd, arg)
/ int semid, cmd;
/ int semnum;
/ union semun {
/	int val;
/	struct semid_ds *buf;
/	ushort *array;
/ } arg;

semctl:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(5)	/ <s>arg
	push	PARAM(4)	/ arg
	push	PARAM(3)	/ cmd
	push	PARAM(2)	/ semnum
	push	PARAM(1)	/ semid
#else
	push	%ds		/ <s>arg
	push	PARAM(4)	/ arg
	push	PARAM(3)	/ cmd
	push	PARAM(2)	/ semnum
	push	PARAM(1)	/ semid
#endif

	push	$SEMCTL
	push	$SEMSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror

/ semget(key, nsems, semflg)
/ key_t key;
/ int nsems, semflg;

semget:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(4)	/ semflg
	push	PARAM(3)	/ nsems
	push	PARAM(2)	/ key(high)
	push	PARAM(1)	/ key(low)

	push	$SEMGET
	push	$SEMSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller

/ semop(semid, sops, nsops)
/ int semid;
/ struct sembuf (*sops)[];
/ int nsops;

semop:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(4)	/ nsops
	push	PARAM(3)	/ <s>sops
	push	PARAM(2)	/ sops
	push	PARAM(1)	/ semid
#else
	push	PARAM(3)	/ nsops
	push	%ds		/ <s>sops
	push	PARAM(2)	/ sops
	push	PARAM(1)	/ semid
#endif

	push	$SEMOP
	push	$SEMSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
