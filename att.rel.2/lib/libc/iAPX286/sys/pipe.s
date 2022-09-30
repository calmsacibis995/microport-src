	.file	"pipe.s"
/	@(#)	1.1
	.text
/
/ pipe(fildes);
/ int fildes[2];

#include "lib.s.h"
	.globl	pipe

pipe:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$PIPE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:

#if HUGE_M | LARGE_M | COMPACT_M
	mov	PARAM(2),%bx	/ get segment of destination
	mov	%bx,%ds
#endif
	mov	PARAM(1),%si	/ get offset of base of destination array
	mov	%ax,(%si)	/ store read end of pipe
	mov	%dx,2(%si)	/ store write end of pipe
	xor	%ax,%ax		/ return code is zero on success
	LVRET			/ restore stack frame and return to caller
