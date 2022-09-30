	.file	"ulimit.s"
/	@(#)	1.1
	.text
/
/ ulimit(cmd, newlimit);
/ int	cmd;
/ long	newlimit;

#include "lib.s.h"
	.globl	ulimit

ulimit:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(3)	/ newlimit(high)
	push	PARAM(2)	/ newlimit(low)
	push	PARAM(1)	/ cmd

	push	$ULIMIT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
