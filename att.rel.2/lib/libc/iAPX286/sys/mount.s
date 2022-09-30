	.file	"mount.s"
/	@(#)	1.1
	.text
/
/ mount(spec, dir, rwflag)
/ char	*spec, *dir;
/ int	rwflag;

#include "lib.s.h"
	.globl	mount

mount:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(5)	/ rwflag
	push	PARAM(4)	/ <s>dir
	push	PARAM(3)	/ dir
	push	PARAM(2)	/ <s>spec
	push	PARAM(1)	/ spec
#else
	push	PARAM(3)	/ rwflag
	push	%ds		/ <s>dir
	push	PARAM(2)	/ dir
	push	%ds		/ <s>spec
	push	PARAM(1)	/ spec
#endif

	push	$MOUNT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
