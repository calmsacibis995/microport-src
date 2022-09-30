	.file	"acct.s"
/	@(#)	1.1
	.text
/
/ acct(path);
/ char	*path;

#include "lib.s.h"
	.globl	acct

acct:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(2)	/ <s>path
	push	PARAM(1)	/ path
#else
	test	PARAM(1)
	jnz	seg
	push	$0		/ <s>(char *)0
	jmp	offset
seg:
	push	%ds		/ <s>path
offset:
	push	PARAM(1)	/ path
#endif

	push	$SYSACCT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
