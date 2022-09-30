	.file	"ustat.s"
/	@(#)	1.1
	.text
/
/ ustat(dev, buf);
/ int		dev;
/ struct ustat	*buf;

#include "lib.s.h"
#define	USTAT	2
	.globl	ustat

ustat:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$USTAT		/ utssys syscall performs uname and ustat
#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(1)	/ dev
	push	PARAM(3)	/ <s>buf
	push	PARAM(2)	/ buf
#else
	push	PARAM(1)	/ dev
	push	%ds		/ <s>buf
	push	PARAM(2)	/ buf
#endif

	push	$UTSSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
