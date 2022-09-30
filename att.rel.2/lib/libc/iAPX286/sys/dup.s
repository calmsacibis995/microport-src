	.file	"dup.s"
/	@(#)	1.1
	.text
/
/ dup(fildes);
/ int	fildes;

#include "lib.s.h"
	.globl	dup

dup:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ fildes

	push	$DUP
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
