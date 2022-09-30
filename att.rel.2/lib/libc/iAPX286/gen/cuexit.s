	.file	"cuexit.s"
/	@(#)	1.1
/
/ exit(code);
/ int	code;

#include	"lib.s.h"
	.globl	exit

exit:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	CALL	_cleanup	/ clean up stdio buffers
	push	PARAM(1)	/code

	push	$EXIT
	lcall	SYSCALL		/ call gate into OS
	hlt			/ exit sys call should never return
