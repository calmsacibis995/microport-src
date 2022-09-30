	.file	"strcmp.s"
/	@(#)	1.1
/ Fast assembler language version of the following C-program for
/			strcmp
/ which represents the `standard' for the C-library.

/	strcmp(s1, s2)
/	register char *s1, *s2;
/	{
/		if(s1 == s2)
/			return(0);
/		while(*s1 == *s2++)
/			if(*s1++ == '\0')
/				return(0);
/		return(*s1 - *--s2);
/	}

/ al = string work area
/ cx = segment length
/ dx = temp
/ ds,si = s2
/ es,di = s1

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strcmp

strcmp:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%cx		/ save registers
	push	%di
	push	%si
#if LARGE_M | HUGE_M | COMPACT_M
	mov	PARAM(1),%si	/ s1 offset
	mov	PARAM(2),%dx	/ s1 <s>
	mov	PARAM(3),%di	/ s2 offset
	mov	PARAM(4),%ax	/ s2 <s>
	cmp	%si,%di		/ s1 == s2 ?
	jne	go
	cmp	%dx,%ax
	je	ret0		/ s1 == s2 return 0
go:
	mov	%dx,%ds		/ ds,si	= s1
	mov	%ax,%es		/ es,di = s2
#else
	mov	PARAM(1),%si	/ es,si = s1
	mov	PARAM(2),%di	/ ds,di = s2
	cmp	%si,%di		/ s1 == s2 ?
	je	ret0		/ yes
	mov	%ds,%dx
	mov	%dx,%es
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
	jcxz	over
	movb	(%si),%al
	dec	%cx
	scmpb
	jne	retne
	cmpb	$0,%al
	jne	lp
	jmp	ret0
over:
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
	jnz	sets1		/	continue cmp if not
#if HUGE_M
	mov	%ds,%dx		/ reset s2 for next segment
	add	$8,%dx
	mov	%dx,%es
	jmp	sets1		/ continue cmp
#else
overflow:
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
ret0:
	clr	%ax		/ *s1 == *s2, return 0
ret:
	pop	%si
	pop	%di		/ restore registers
	pop	%cx
	LVRET			/ restore stack frame and return to caller
retne:
	mov	$1,%ax		/ set 1 or -1 for return
	jg	ret		/ depending on difference
	neg	%ax
	jmp	ret
