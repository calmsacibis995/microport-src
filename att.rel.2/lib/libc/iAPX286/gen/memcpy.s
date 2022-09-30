	.file	"memcpy.s"
/	@(#)	1.1

/ Copy s2 to s1, always copy n bytes.
/ Return s1

/	char *
/	memcpy(s1, s2, n)
/	register char *s1, *s2;
/	register int n;
/	{
/		register char *os1 = s1;
/	
/		while (--n >= 0)
/			*s1++ = *s2++;
/		return (os1);
/	}

/ es,di = s1
/ ds,si = s2
/ cx = n
/ ax = temp
/ dx = n_left

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	memcpy

memcpy:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%cx		/ save registers
	push	%di
#if LARGE_M | HUGE_M | COMPACT_M
	les	PARAM(1),%di	/ es,di = s1
	lds	PARAM(3),%si	/ ds,si = s2
	mov	PARAM(5),%cx	/ cx = n
#else
	mov	PARAM(1),%di	/ es,di = s1
	mov	%ds,%ax
	mov	%ax,%es
	mov	PARAM(2),%si	/ ds,si = s2
	mov	PARAM(3),%cx	/ cx = n
#endif
	test	%cx		/ verify that n > 0
	jle	out		/	skip if not
setn:
	clr	%dx		/ initialize n_left
	mov	%di,%ax		/ check for s1 segment overflow before n == 0
	neg	%ax
	jz	destok		/ skip if no overflow possible
	cmp	%cx,%ax
	jae	destok		/ skip if remainder of segment >= n
	add	%cx,%dx		/ reset n_left and n
	sub	%ax,%dx
	mov	%ax,%cx
destok:
	mov	%si,%ax		/ check for s2 segment overflow before n == 0
	neg	%ax
	jz	lp		/ skip if no overflow possible
	cmp	%cx,%ax
	jae	lp		/ skip if remainder of segment >= n
	add	%cx,%dx		/ reset n_left and n
	sub	%ax,%dx
	mov	%ax,%cx
lp:
	rep ;	smovb		/ copy until n == 0
	test	%dx		/ done?
	jz	out		/	skip if done
#if HUGE_M
	mov	%dx,%cx		/ set n = n_left
	test	%si		/ check for s2 break into new segment
	jnz	srcok		/	skip if not
	mov	%ds,%ax		/ reset ds for next segment
	add	$8,%ax
	mov	%ax,%ds
srcok:
	test	%di		/ check for s1 break into new segment
	jnz	setn		/	skip if not
	mov	%es,%ax		/ reset es for next segment
	add	$8,%ax
	mov	$ax,%es
	jmp	setn		/ try again
#else
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
out:
	pop	%di		/ restore registers
	pop	%cx
	mov	PARAM(1),%ax	/ return original s1
#if LARGE_M | HUGE_M | COMPACT_M
	mov	PARAM(2),%dx
#endif
	LVRET			/ restore stack frame and return to caller
