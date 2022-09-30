	.file	"strcpy.s"
/	@(#)	1.1

/ Copy string s2 to s1.  s1 must be large enough. Return s1.

/	char *
/	strcpy(s1, s2)
/	register char *s1, *s2;
/	{
/		register char *os1;
/	
/		os1 = s1;
/		while (*s1++ = *s2++)
/			;
/		return(os1);
/	}

/ al = string work area
/ cx = segment length
/ dx = temp
/ ds,si = s2
/ es,di = s1

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strcpy

strcpy:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%cx		/ save registers
	push	%di
#if LARGE_M | HUGE_M | COMPACT_M
	les	PARAM(1),%di	/ es,di = s1
	lds	PARAM(3),%si	/ ds,si = s2
#else
	mov	PARAM(1),%di	/ es,di = s1
	mov	%ds,%dx
	mov	%dx,%es
	mov	PARAM(2),%si	/ ds,si = s2
#endif
sets1:
	mov	%di,%cx		/ set cx to largest count possible without
	neg	%cx		/	segment overflow
	jnz	sets2
	dec	%cx
sets2:
	mov	%si,%dx		/ see if s2 will cross segment before s1
	neg	%dx
	jz	lp		/ skip if s2 is at start of segment
	cmp	%cx,%dx
	jae	lp		/ skip if count is OK
	mov	%dx,%cx		/ reset count
lp:
	slodb			/ get s2++
	sstob			/ put it at s1++
	testb	%al		/ end of string?
	loopne	lp		/ loop until end of string or count == 0
	jz	ret		/ skip if end of string copied
	test	%di		/ end of segment for s1?
#if HUGE_M
	jnz	s1ok		/	skip if not
	mov	%es,%dx		/ reset s1 for next segment
	add	$8,%dx
	mov	%dx,%es
s1ok:
#else
	jz	overflow	/ jmp if end of segment
#endif
	test	%si		/ end of segment for s2?
	jnz	sets1		/	continue copy if not
#if HUGE_M
	mov	%ds,%dx		/ reset s2 for next segment
	add	$8,%dx
	mov	%dx,%es
	jmp	sets1		/ continue copy
#else
overflow:
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
ret:
	mov	PARAM(1),%ax	/ return original s1
#if LARGE_M | HUGE_M | COMPACT_M
	mov	PARAM(2),%dx
#endif
	pop	%di		/ restore registers
	pop	%cx
	LVRET			/ restore stack frame and return to caller
