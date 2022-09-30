	.file	"sync.s"
/	@(#)	1.1
	.text
/
/  sync();

#include "lib.s.h"
	.globl	sync

sync:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$SYNC
	lcall	SYSCALL		/ call gate into OS
	LVRET			/ restore stack frame and return to caller
