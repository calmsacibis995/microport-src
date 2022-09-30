	.file	"profil.s"
/	@(#)	1.3
	.text
/
/ profil(buff, bufsiz, offset, scale);
/ char	*buff;
/ int	bufsiz, scale;
/ void	(*offset)();

#include "lib.s.h"
#include "sys/mmu.h"
	.globl	profil

profil:
	push	%bp		/ enter subroutine
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling

#if LARGE_M | HUGE_M
	push	PARAM(6)	/ scale
	test	PARAM(5)	/ if <s>offset == 0
	jnz	realseg		/	supply segment of 1st code seg
	push	$[CODE1_SEL << 3 | LDT_TI | USER_RPL]
	jmp	offset
realseg:
	push	PARAM(5)	/ else use given <s>offset
offset:
	push	PARAM(4)	/ offset
	push	PARAM(3)	/ bufsiz
	push	PARAM(2)	/ <s>buff
	push	PARAM(1)	/ buff
#endif
#if COMPACT_M
	push	PARAM(5)	/ scale
	push	%cs		/ <s>offset
	push	PARAM(4)	/ offset
	push	PARAM(3)	/ bufsiz
	push	PARAM(2)	/ <s>buff
	push	PARAM(1)	/ buff
#endif
#if MIDDLE_M
	push	PARAM(5)	/ scale
	test	PARAM(4)	/ if <s>offset == 0
	jnz	realseg		/	supply segment of 1st code seg
	push	$[CODE1_SEL | LDT_TI | USER_RPL]
	jmp	offset
realseg:
	push	PARAM(4)	/ else use given <s>offset
offset:
	push	PARAM(3)	/ offset
	push	PARAM(2)	/ bufsiz
	push	%ds		/ <s>buff
	push	PARAM(1)	/ buff
#endif
#if SMALL_M
	push	PARAM(4)	/ scale
	push	%cs		/ <s>offset
	push	PARAM(3)	/ offset
	push	PARAM(2)	/ bufsiz
	push	%ds		/ <s>buff
	push	PARAM(1)	/ buff
#endif

	push	$PROF
	lcall	SYSCALL		/ call gate into OS
	jnc	noerror
	JMP	_cerror
noerror:
	LVRET			/ restore stack frame and return to caller
