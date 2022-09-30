	.file	"access.s"
/	@(#)	1.1
	.text
/
/ access(path, amode);
/ char	*path;
/ int	amode;

#include "lib.s.h"
	.globl	access

access:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(3)	/ amode
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	push	PARAM(2)	/ amode
	push	%ds		/ <s>path
	push	PARAM(1)	/ path
#endif
	push	$ACCESS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
