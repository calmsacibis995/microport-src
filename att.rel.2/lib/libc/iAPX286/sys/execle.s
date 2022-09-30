	.file	"execle.s"
/	@(#)	1.1
	.text
/
/ execle(path, arg0, arg1, ..., argn, (char *)0 , envp);
/ char	*path, *arg0, *arg1, ..., *argn, *envp[];

#include "lib.s.h"

/ ARG0 = param # of offset of arg0
/ PTRSIZE = # bytes in a data pointer
/ START = param # to use as initialization point for loop looking for (char *)0
#if LARGE_M | HUGE_M | COMPACT_M
#define	ARG0	3
#define	PTRSIZE	4
#define	START	2
#else
#define	ARG0	2
#define	PTRSIZE	2
#define	START	1
#endif
	.globl	execle

execle:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	lea	PARAM(START),%si/ initialize for loop looking for (char *)0
loop:
	add	$PTRSIZE,%si	/ set to next arg pointer
	test	%ss:(%si)
	jnz	loop		/ loop until NULL pointer found
#if LARGE_M | HUGE_M | COMPACT_M
	push	%ss:4(%si)	/ <s>envp
#else
	push	%ds		/ <s>envp
#endif
	push	%ss:2(%si)	/ envp
	push	%ss		/ <s>argv
	lea	PARAM(ARG0),%ax
	push	%ax		/ argv
#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(2)	/ <s>path
#else
	push	%ds		/ <s>path
#endif
	push	PARAM(1)	/ path

	push	$EXECE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
