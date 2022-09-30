	.file	"stime.s"
/	@(#)	1.1
	.text
/
/ stime(tp);
/ long	*tp;

#include "lib.s.h"
	.globl	stime

stime:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

/
/ The stime syscall expects the time as an arg, not a pointer to it.
/
#if LARGE_M | HUGE_M | COMPACT_M
	mov	PARAM(2),%bx	/ get segment of tp into %ds
	mov	%bx,%ds
#endif
	mov	PARAM(1),%si	/ get offset in segment of base of time into %si
	push	2(%si)		/ time(high)
	push	(%si)		/ time(low)

	push	$STIME
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
