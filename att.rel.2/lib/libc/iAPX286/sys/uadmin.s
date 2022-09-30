	.file "uadmin.s"
/	@(#)	1.1
	.text
/
/ error = uadmin(cmd, fcn, mdep)
/ int	cmd,fcn,mdep;

#include "lib.s.h"
	.globl	uadmin

uadmin:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	PARAM(3)	/ mdep
	push	PARAM(2)	/ fcn
	push	PARAM(1)	/ cmd

	push	$UADMIN
	lcall	SYSCALL		/ call gate into OS
	jnc 	noerror
	JMP 	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
