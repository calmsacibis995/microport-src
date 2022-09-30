	.file	"fork.s"
/	@(#)	1.1
	.text
/
/ fork();

#include "lib.s.h"
	.globl	fork

fork:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$FORK
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	test	%dx		/ determine whether we are parent or child
	jz	parent
	xor	%ax,%ax		/ return 0 in child
parent:
	LVRET			/ restore stack frame and return to caller
