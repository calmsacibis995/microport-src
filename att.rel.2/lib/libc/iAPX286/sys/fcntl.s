	.file	"fcntl.s"
/	@(#)	1.2
	.text
/
/ fcntl(fildes, cmd, arg);
/ int	fildes, cmd;
/ VARIOUS arg;

#include "lib.s.h"
	.globl	fcntl

fcntl:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ <s>arg
#else
	push	%ds		/ <s>arg
#endif
	push	PARAM(3)	/ arg
	push	PARAM(2)	/ cmd
	push	PARAM(1)	/ fildes

	push	$FCNTL
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
