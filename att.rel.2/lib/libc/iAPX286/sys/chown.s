	.file	"chown.s"
/	@(#)	1.1
	.text
/
/ chown(path, owner, group);
/ char	*path;
/ int	owner, group;

#include "lib.s.h"
	.globl	chown

chown:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ group
	push	PARAM(3)	/ owner
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	PARAM(3)	/ group
	push	PARAM(2)	/ owner
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif

	push	$CHOWN
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
