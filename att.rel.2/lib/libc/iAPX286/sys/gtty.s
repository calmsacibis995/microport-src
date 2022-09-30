	.file	"gtty.s"
/	@(#)	1.1
	.text
/
/ Obsolete, unsupported, and undocumented.  Here for compatibility with V7.
/ gtty(fildes, argp);
/ int	fildes;
/ struct sgttyb	*argp;

#include "lib.s.h"
	.globl	gtty

gtty:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M
	push	PARAM(3)	/ <s>argp
	push	PARAM(2)	/ argp
	push	PARAM(1)	/ fildes
#else
	push	%ds		/ <s>argp
	push	PARAM(2)	/ argp
	push	PARAM(1)	/ fildes
#endif

	push	$GTTY
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
