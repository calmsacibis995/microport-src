	.file	"chdir.s"
/	@(#)	1.1
	.text

/ chdir(path);
/ char	*path;

#include "lib.s.h"
	.globl	chdir

chdir:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$CHDIR
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
