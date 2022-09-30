	.file	"ptrace.s"
/	@(#)	1.1
	.text
/
/ ptrace(request, pid, addr, data);
/ int	request, pid, data;
/ long	addr;

#include "lib.s.h"
	.globl	ptrace

ptrace:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M || HUGE_M
	push	PARAM(5)	/ data
	push	PARAM(4)	/ addr(high)
#else
	push	PARAM(4)	/ data

	/
	/	special cases for segment part of addr are:-
	/
	/	request #	segment
	/
	/	0		ignored
	/	1		%cs
	/	2		%ds
	/	3		ignored
	/	4		%cs
	/	5		%ds
	/	6		ignored
	/	7		0 if lo part 1 else %cs
	/	8		ignored
	/	9		0 if lo part 1 else %cs
	/	

	mov	PARAM(1),%ax	/ request
	cmp	$9
	je	pushcsor1
	cmp	$7
	je	pushcsor1
	cmp	$4
	je	pushcs
	cmp	$1
	je	pushcs
/
/ otherwise use %ds
/
	push	%ds
	jmp	pushaddr
pushcsor1:
	cmp	$1,PARAM(3)
	jne	pushcs
	push	$0
	jmp	pushaddr
pushcs:
	push	%cs
pushaddr:

#endif
	push	PARAM(3)	/ addr(low)
	push	PARAM(2)	/ pid
	push	PARAM(1)	/ request

	push	$PTRACE
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
