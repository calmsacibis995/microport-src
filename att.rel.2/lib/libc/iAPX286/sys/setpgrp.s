	.file	"setpgrp.s"
/	@(#)	1.1
	.text

#include "lib.s.h"
#define	GET	0
#define	SET	1

	.globl	setpgrp
	.globl	getpgrp

/
/ getpgrp();

getpgrp:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$GET
	push	$SETPGRP
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror

/
/ setpgrp();

setpgrp:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$SET
	push	$SETPGRP
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
