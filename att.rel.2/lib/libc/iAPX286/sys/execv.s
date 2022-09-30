	.file	"execv.s"
/	@(#)	1.1
	.text
/
/ execv(path, argv);
/ char	*path, *argv[];

#include "lib.s.h"
	.globl	execv

execv:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	mov	$<s>environ,%ax	/ get access to environ
	mov	%ax,%es
	push	%es:environ+2	/ <s>envp
	push	%es:environ	/ envp
	push	PARAM(4)	/ <s>argv
	push	PARAM(3)	/ argv
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	%ds		/ <s>envp
	push	environ		/ envp
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
