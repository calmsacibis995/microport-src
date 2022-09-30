	.file	"times.s"
/	@(#)	1.1
	.text
/
/ times(buffer);
/ struct tms	*buffer;

#include "lib.s.h"
	.globl	times

times:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M
	push	PARAM(2)	/ <s>buffer
	push	PARAM(1)	/ buffer
#else
	push	%ds		/ <s>buffer
	push	PARAM(1)	/ buffer
#endif

	push	$TIMES
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
