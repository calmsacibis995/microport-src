	.file	"getpid.s"
/	@(#)	1.1
	.text
/
/ getpid();

#include "lib.s.h"
	.globl	getpid

getpid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$GETPID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
