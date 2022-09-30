	.file	"setuid.s"
/	@(#)	1.1
	.text
/
/ setuid(uid);
/ int	uid;

#include "lib.s.h"
	.globl	setuid

setuid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ uid

	push	$SETUID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
