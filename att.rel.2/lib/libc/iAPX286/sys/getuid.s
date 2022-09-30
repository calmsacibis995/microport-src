	.file	"getuid.s"
/	@(#)	1.1
	.text
/
/ getuid();

#include "lib.s.h"
	.globl	getuid

getuid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$GETUID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
