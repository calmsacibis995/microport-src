	.file	"getegid.s"
/	@(#)	1.1
	.text
/
/ getegid();

#include "lib.s.h"
	.globl	getegid

getegid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$GETGID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	mov	%dx,%ax		/ getgid syscall returns real and effective gids
	LVRET			/ restore stack frame and return to caller
