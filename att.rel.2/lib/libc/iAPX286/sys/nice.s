	.file	"nice.s"
/	@(#)	1.1
	.text
/
/ nice(incr);
/ int	incr;

#include "lib.s.h"
	.globl	nice

nice:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ incr

	push	$NICE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
