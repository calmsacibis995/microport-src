	.file	"signal.s"
/	@(#)	1.1
	.text
/
/ signal(sig, func);
/ int	sig;
/ void	(*func)();

#include "lib.s.h"
	.globl	signal

signal:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | MIDDLE_M
	push	PARAM(3)	/ <s>func
	push	PARAM(2)	/ func
	push	PARAM(1)	/ sig
#else
	cmp	$1,PARAM(2)
	ja	signal1		/ user function given
	push	$0		/ SIG_IGN or SIG_DFL
	jmp	signal2
signal1:
	push	%cs		/ <s>func
signal2:
	push	PARAM(2)	/ func
	push	PARAM(1)	/ sig
#endif

	push	$SIGNAL
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
