	.file	"strncmp.s"
/	@(#)	1.1
/ Fast assembler language version of the following C-program
/			strncmp
/ which represents the `standard' for the C-library.

/ Compare strings (at most n bytes): 
/ return: s1>s2 >0 , s1==s2 0, s1<s2 <0
/ characters in s1 and s2 which were compared.

/	strncmp(s1, s2, n)
/	register char *s1, *s2;
/	register n;
/	{
/		if(s1 == s2)
/			return(0);
/		while(--n >= 0 && *s1 == *s2++)
/			if(*s1++ == '\0')
/				return(0);
/		return((n < 0)? 0: *s1 - *--s2);
/	}

/ ax = temp
/ cx = n
/ dx = temp, n_left
/ ds,si = s1
/ es,di = s2

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strncmp

strncmp:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%cx		/ save registers
	push	%di
#if LARGE_M | HUGE_M | COMPACT_M
	mov	PARAM(1),%si	/ s1
	mov	PARAM(3),%di	/ s2
	mov	PARAM(2),%dx	/ <s>s1
	mov	PARAM(4),%ax	/ <s>s2
	cmp	%si,%di		/ s1 == s2?
	jne	go		/	no, skip segment check
	cmp	%dx,%ax
	je	ret0		/ s1 == s2, return 0
go:
	mov	%dx,%ds		/ ds,si = s1
	mov	%ax,%es		/ es,di = s2
	mov	PARAM(5),%cx	/ cx = n
#else
	mov	PARAM(1),%si	/ s1
	mov	PARAM(2),%di	/ s2
	cmp	%si,%di		/ s1 == s2?
	je	ret0		/ s1 == s2, return 0
	mov	%ds,%ax		/ ds,si = s1; es,di = s2
	mov	%ax,%es
	mov	PARAM(3),%cx	/ cx = n
#endif
	test	%cx		/ verify that n > 0
	jle	ret0		/	if not, return 0
setn:
	clr	%dx		/ initialize n_left
	mov	%si,%ax		/ check for s1 segment overflow before n == 0
	neg	%ax
	jz	destok		/ skip if no overflow possible
	cmp	%cx,%ax
	jae	destok		/ skip if remainder of segment >= n
	add	%cx,%dx		/ reset n_left and n
	sub	%ax,%dx
	mov	%ax,%cx
destok:
	mov	%di,%ax		/ check for s2 segment overflow before n == 0
	neg	%ax
	jz	lp		/ skip if no overflow possible
	cmp	%cx,%ax
	jae	lp		/ skip if remainder of segment >= n
	add	%cx,%dx		/ reset n_left and n
	sub	%ax,%dx
	mov	%ax,%cx
lp:
	jcxz	over		/ done this many
	movb	(%si),%al	/ s1[i]
	dec	%cx
	scmpb			/ compare two chars and increment si and di
	jne	retne
	cmpb	$0,%al
	jne	lp
	jmp	ret0
over:
	test	%dx		/ done?
	jz	ret0		/	skip if done
#if HUGE_M
	test	%si		/ check for s1 segment overflow
	jnz	s1ok		/	skip if not
	mov	%ds,%ax		/ set up for next segment
	add	$8,%ax
	mov	%ax,%ds
s1ok:
	test	%di		/ check for s2 segment overflow
	jnz	setn		/	skip if not
	mov	%es,%ax		/ set up for next segment
	add	$8,%ax
	mov	%ax,%es
	mov	%dx,%cx		/ reset n to n_left
	jmp	setn		/ try again
#else
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
ret0:
	clr	%ax		/ *s1 == *s2, return 0
ret:
	pop	%di		/ restore registers
	pop	%cx
	LVRET			/ restore stack frame and return to caller
retne:
	mov	$1,%ax		/ set 1 or -1 for return
	jg	ret		/ depending on difference
	neg	%ax
	jmp	ret
