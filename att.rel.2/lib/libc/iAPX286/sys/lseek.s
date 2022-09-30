	.file	"lseek.s"
/	@(#)	1.1
	.text
/
/ error = lseek(fildes, offset, whence);
/ int	fildes, whence;
/ long	offset;

#include "lib.s.h"
	.globl	lseek

lseek:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(4)	/ whence
	push	PARAM(3)	/ offset(high)
	push	PARAM(2)	/ offset(low)
	push	PARAM(1)	/ fildes

	push	$SEEK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
