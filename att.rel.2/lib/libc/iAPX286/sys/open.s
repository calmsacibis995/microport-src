	.file	"open.s"
/	@(#)	1.1
	.text
/
/ open(path, oflag [, mode]);
/ char	*path;
/ int	oflag, mode;

#include "lib.s.h"
	.globl	open

open:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ mode
	push	PARAM(3)	/ oflag
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	PARAM(3)	/ mode
	push	PARAM(2)	/ oflag
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$OPEN
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
