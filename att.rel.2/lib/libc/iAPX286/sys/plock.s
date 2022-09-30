	.file	"plock.s"
/	@(#)	1.1
	.text
/
/ plock(op);
/ int	op;

#include "lib.s.h"
	.globl	plock

plock:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ op

	push	$LOCK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
