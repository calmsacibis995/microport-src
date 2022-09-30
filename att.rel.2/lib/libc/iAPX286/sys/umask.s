	.file	"umask.s"
/	@(#)	1.1
	.text
/
/ umask(cmask);
/ int	cmask;

#include "lib.s.h"
	.globl	umask

umask:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ cmask

	push	$UMASK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
