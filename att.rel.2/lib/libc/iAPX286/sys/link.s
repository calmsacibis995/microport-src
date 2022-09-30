	.file	"link.s"
/	@(#)	1.1
	.text
/
/ link(path1, path2);
/ char	*path1, *path2;

#include "lib.s.h"
	.globl	link

link:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ <s>path2
	push	PARAM(3)	/ path2
	push	PARAM(2)	/ <s>path1
	push	PARAM(1)	/ path1
#else
	push	%ds		/ <s>path2
	push	PARAM(2)	/ path2
	push	%ds		/ <s>path1
	push	PARAM(1)	/ path1
#endif

	push	$LINK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
