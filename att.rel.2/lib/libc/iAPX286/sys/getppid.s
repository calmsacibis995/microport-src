	.file	"getppid.s"
/	@(#)	1.1
	.text
/
/ getppid();

#include "lib.s.h"
	.globl	getppid

getppid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$GETPID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	mov	%dx,%ax		/ getpid syscall returns pid and ppid
	LVRET			/ restore stack frame and return to caller
