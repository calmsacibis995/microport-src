	.file	"execve.s"
/	@(#)	1.1
	.text
/
/ execve(path, argv, envp);
/ char	*path, *argv[], *envp[];

#include "lib.s.h"
	.globl	execve

execve:
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
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
