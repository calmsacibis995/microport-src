	.file	"mknod.s"
/	@(#)	1.1
	.text
/
/ mknod(path, mode, dev);
/ char	*path;
/ int	mode, dev;

#include "lib.s.h"
	.globl	mknod

mknod:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ dev
	push	PARAM(3)	/ mode
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	PARAM(3)	/ dev
	push	PARAM(2)	/ mode
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$MKNOD
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
