	.file	"exit.s"
/	@(#)	1.1
	.text
/
/ _exit(code)
/ int	code;

#include "lib.s.h"
	.globl	_exit

_exit:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ code

	push	$EXIT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
