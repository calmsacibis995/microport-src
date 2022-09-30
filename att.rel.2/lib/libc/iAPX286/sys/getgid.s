	.file	"getgid.s"
/	@(#)	1.1
	.text
/
/ getgid();

#include "lib.s.h"
	.globl	getgid
getgid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$GETGID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
