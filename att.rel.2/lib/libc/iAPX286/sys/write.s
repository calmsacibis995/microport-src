	.file	"write.s"
/	@(#)	1.1
	.text
/
/ write(fildes, buf, nbyte);
/ int	fildes;
/ char	*buf;
/ unsigned nbyte;

#include "lib.s.h"
	.globl	write

write:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(4)	/ nbyte
	push	PARAM(3)	/ <s>buf
	push	PARAM(2)	/ buf
	push	PARAM(1)	/ fildes
#else
	push	PARAM(3)	/ nbyte
	push	%ds		/ <s>buf
	push	PARAM(2)	/ buf
	push	PARAM(1)	/ fildes
#endif

	push	$WRITE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
