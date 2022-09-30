	.file	"umount.s"
/	@(#)	1.1
	.text
/
/ umount(spec);
/ char	*spec;

#include "lib.s.h"
	.globl	umount

umount:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(2)	/ <s>spec
	push	PARAM(1)	/ spec
#else
	push	%ds		/ <s>spec
	push	PARAM(1)	/ spec
#endif

	push	$UMOUNT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
