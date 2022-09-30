	.file	"abs.s"
/	@(#)	1.1
/	/* Assembler program to implement the following C program */
/	int
/	abs(arg)
/	int	arg;
/	{
/		return((arg < 0)? -arg: arg);
/	}

#include	"lib.s.h"
	.text
	.globl	abs

abs:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	mov	PARAM(1),%ax	/ arg < 0?
	test	%ax
	jns	.absl
	neg	%ax		/ yes, return -arg
.absl:
	LVRET			/ restore stack frame and return to caller
