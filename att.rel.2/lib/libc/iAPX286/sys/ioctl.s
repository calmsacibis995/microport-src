	.file	"ioctl.s"
/	@(#)	1.1
	.text
/
/ ioctl(fildes, request, arg);
/ int	fildes, request;
/ VARIOUS	arg;

#include "lib.s.h"
	.globl	ioctl

ioctl:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if HUGE_M | LARGE_M | COMPACT_M
	push	PARAM(4)	/ <s>arg
	push	PARAM(3)	/ arg
	push	PARAM(2)	/ request
	push	PARAM(1)	/ fildes
#else
	push	%ds		/ <s>arg
	push	PARAM(3)	/ arg
	push	PARAM(2)	/ request
	push	PARAM(1)	/ fildes
#endif

	push	$IOCTL
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
