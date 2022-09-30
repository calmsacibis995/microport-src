	.file	"utime.s"
/	@(#)	1.1
	.text
/
/ utime(path, times);
/ char			*path;
/ struct utimbuf	*times;

#include "lib.s.h"
	.globl	utime
 
utime:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ <s>times
	push	PARAM(3)	/ times
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	test	PARAM(2)	/ if NULL then push long NULL
	jnz	pushds
	push	$0		/ top half of NULL
	jmp	pushparam2
pushds:
	push	%ds		/ <s>times
pushparam2:
	push	PARAM(2)	/ times
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$UTIME
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
