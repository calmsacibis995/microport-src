	.file	"pause.s"
/	@(#)	1.1
	.text
/
/ pause();

#include "lib.s.h"
	.globl	pause

pause:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$PAUSE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
