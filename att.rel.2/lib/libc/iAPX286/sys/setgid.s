	.file	"setgid.s"
/	@(#)	1.1
	.text
/
/ setgid(gid);
/ int	gid;

#include "lib.s.h"

	.globl	setgid
setgid:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ gid

	push	$SETGID
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
