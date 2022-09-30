	.file	"alarm.s"
/	@(#)	1.1
	.text
/
/ alarm(sec);
/ int	sec;

#include "lib.s.h"
	.globl	alarm

alarm:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(1)	/ time in seconds

	push	$ALARM
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
