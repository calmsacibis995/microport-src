	.file	"wait.s"
/	@(#)	1.1
	.text
/
/ wait(stat_loc);
/ int	*stat_loc;

#include "lib.s.h"
	.globl	wait

wait:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$WAIT
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:

#if LARGE_M | HUGE_M | COMPACT_M
	test	PARAM(2)	/ store status at *stat_loc if non-zero
	jnz	copy
	test	PARAM(1)
	jz	nocopy
copy:
	mov	PARAM(2),%bx	/ get segment of *stat_loc into %ds
	mov	%bx,%ds
#else
	test	PARAM(1)	/ store status at *stat_loc if non-zero
	jz	nocopy
#endif
	mov	PARAM(1),%si	/ get offset of *stat_loc into %si
	mov	%dx,(%si)	/ copy the status
nocopy:
	LVRET			/ restore stack frame and return to caller
