	.file	"nexect.s"
/	@(#)	1.1
	.text
/
/ this is the same as execve described below.
/ It sets single step prior to exec call,
/ this will stop the user on the first instruction executed
/ and allow the parent to set break points as appropriate.
/ This is used by tracing mechanisms,such as sdb.
/ execve(path, argv, envp);
/ char	*path, *argv[], *envp[];

#include "sys/psl.h"
#include "lib.s.h"
	.globl	exect

exect:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(6)	/ <s>envp
	push	PARAM(5)	/ envp
	push	PARAM(4)	/ <s>argv
	push	PARAM(3)	/ argv
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	%ds		/ <s>envp
	push	PARAM(3)	/ envp
	push	%ds		/ <s>argv
	push	PARAM(2)	/ argv
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$EXECE
/ set the single step flag bit (trap flag)
	pushf
	pop	%ax
	or	$PS_T,%ax
	push	%ax
	popf
/ this has now set single step which should be preserved by the system
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
