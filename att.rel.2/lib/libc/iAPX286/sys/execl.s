	.file	"execl.s"
/	@(#)	1.1
	.text
/
/ execl(path, arg0, arg1, ..., argn, (char *)0);
/ char	*path, *arg0, *arg1, ..., *argn;

#include "lib.s.h"

	.globl	execl
execl:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	mov	$<s>environ,%ax	/ get access to environ
	mov	%ax,%es
	push	%es:environ+2	/ <s>envp
	push	%es:environ	/ envp
	push	%ss		/ <s>argv
	lea	PARAM(3),%ax	/ get address of arg0
	push	%ax		/ argv
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	%ds		/ <s>envp
	push	environ		/ envp
	push	%ss		/ <s>argv
	lea	PARAM(2),%ax	/ get address of arg0
	push	%ax		/ argv
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif
	push	$EXECE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
