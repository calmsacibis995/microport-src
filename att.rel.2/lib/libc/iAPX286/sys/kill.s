	.file	"kill.s"
/	@(#)	1.1
	.text
/
/ kill(pid, sig);
/ int	pid, sig;

#include "lib.s.h"
	.globl	kill

kill:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(2)	/ sig
	push	PARAM(1)	/ pid

	push	$KILL
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
