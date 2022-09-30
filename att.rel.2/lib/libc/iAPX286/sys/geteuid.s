	.file	"geteuid.s"
/	@(#)	1.1
	.text
/
/ geteuid();

#include "lib.s.h"
	.globl	geteuid

geteuid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$GETUID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	mov	%dx,%ax		/ getuid syscall returns real and effective uids
	LVRET			/ restore stack frame and return to caller
