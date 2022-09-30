	.file	"time.s"
/	@(#)	1.1
	.text
/
/ time(tloc);
/ long	*tloc;

#include "lib.s.h"
	.globl	time

time:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$TIME		/ gtime syscall takes no args, but returns time
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:

#if LARGE_M | HUGE_M | COMPACT_M
	test	PARAM(2)	/ if NULL pointer,
	jnz	copy		/	don't do the copy
	test	PARAM(1)
	jz	nocopy
copy:
	mov	PARAM(2),%bx	/ get segment of place to store the time
	mov	%bx,%ds
#else
	test	PARAM(1)	/ if NULL pointer,
	jz	nocopy		/	don't do the copy
#endif
	mov	PARAM(1),%si	/ get offset of place to store time into %si
	mov	%dx,2(%si)	/ copy high word of time
	mov	%ax,(%si)	/ copy low word of time
nocopy:
	LVRET			/ restore stack frame and return to caller
