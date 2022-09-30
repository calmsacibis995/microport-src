	.file	"stat.s"
/	@(#)	1.1
	.text
/
/ stat(path, buf);
/ char		*path;
/ struct stat	*buf;

#include "lib.s.h"
	.globl	stat

stat:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ <s>buf
	push	PARAM(3)	/ buf
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	%ds		/ <s>buf
	push	PARAM(2)	/ buf
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$STAT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
