	.file	"fstat.s"
/	@(#)	1.1
	.text
/
/ fstat(fildes, buf);
/ int fildes;
/ struct stat *buf;

#include "lib.s.h"
	.globl	fstat

fstat:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(3)	/ <s>buf
	push	PARAM(2)	/ buf
	push	PARAM(1)	/ fildes
#else
	push	%ds		/ <s>buf
	push	PARAM(2)	/ buf
	push	PARAM(1)	/ fildes
#endif

	push	$FSTAT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
