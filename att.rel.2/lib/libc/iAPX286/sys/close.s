	.file	"close.s"
/	@(#)	1.1
	.text
/
/ close(fildes);
/ int	fildes;

#include "lib.s.h"
	.globl	close

close:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ fildes

	push	$CLOSE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
