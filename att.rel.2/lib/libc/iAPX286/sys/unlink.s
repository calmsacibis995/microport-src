	.file	"unlink.s"
/	@(#)	1.1
	.text
/
/ unlink(path);
/ char	*path;

#include "lib.s.h"
	.globl	unlink

unlink:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$UNLINK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
