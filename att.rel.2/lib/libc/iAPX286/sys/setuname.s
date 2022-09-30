	.file	"setuname.s"
/	@(#)	1.1
	.text
/
/ setuname(name);
/ struct utsname	*name;

#include "lib.s.h"
#define	SETUNAME	3
	.globl	setuname

setuname:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

	push	$SETUNAME	/ utssys syscall performs uname and ustat
	push	$0		/ utssys expects 3 arguments
#if LARGE_M | HUGE_M | COMPACT_M
	push	PARAM(2)	/ <s>name
	push	PARAM(1)	/ name
#else
	push	%ds		/ <s>name
	push	PARAM(1)	/ name
#endif

	push	$UTSSYS
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
