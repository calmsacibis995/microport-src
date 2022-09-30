	.file	"memccpy.s"
/	@(#)	1.1

/ Copy s2 to s1, stopping if character c is copied. Copy no more than n bytes.
/ Return a pointer to the byte after character c in the copy,
/ or NULL if c is not found in the first n bytes.

/	char *
/	memccpy(s1, s2, c, n)
/	register char *s1, *s2;
/	register int c, n;
/	{
/		while (--n >= 0)
/			if ((*s1++ = *s2++) == c)
/				return (s1);
/		return (0);
/	}

/ ax = copy work area
/ bl = c
/ cx = n
/ dx = temp
/ ds,si = s2
/ es,di = s1

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	memccpy

memccpy:
	enter	$2,$0		/ reserve space for n_left
	MCOUNT			/ call subroutine entry counter if profiling
	push	%bx		/ save registers
	push	%cx
	push	%di
#if LARGE_M | HUGE_M | COMPACT_M
	les	PARAM(1),%di	/ es,di = s1
	lds	PARAM(3),%si	/ ds,si = s2
	mov	PARAM(5),%bx	/ bl = c
	mov	PARAM(6),%cx	/ cx = n
#else
	mov	PARAM(1),%di	/ es,di = s1
	mov	%ds,%dx
	mov	%dx,%es
	mov	PARAM(2),%si	/ ds,si = s2
	mov	PARAM(3),%bx	/ bl = c
	mov	PARAM(4),%cx	/ cx = n
#endif
	test	%cx		/ verify that n > 0
	jle	null		/	skip if not
setn:
	mov	$0,-2(%bp)	/ initialize n_left after this time thru loop
	mov	%di,%dx		/ check for s1 segment overflow before n == 0
	neg	%dx
	jz	s1ok		/ skip if at start of segment
	cmp	%cx,%dx
	jae	s1ok		/	or remainder of segment >= n
	add	%cx,-2(%bp)	/ reset n_left and n
	sub	%dx,-2(%bp)
	mov	%dx,%cx
s1ok:
	mov	%si,%dx		/ check for s2 segment overflow before n == 0
	neg	%dx
	jz	lp1		/ skip if at start of segment
	cmp	%cx,%dx
	jae	lp1		/	or remainder of segment >= n
	add	%cx,-2(%bp)	/ reset n_left and n
	sub	%dx,-2(%bp)
	mov	%dx,%cx
lp1:
	slodb			/ get next character from s2
	sstob			/ put it in s1
	cmpb	%al,%bl		/ c == copied char?
	loopne	lp1		/ loop until c == copied char or n == 0
	je	ptr		/ skip if c == copied char
	test	-2(%bp)		/ done?
	jz	null		/	skip if yes
#if HUGE_M
	mov	-2(%bp),%cx	/ set n to n_left
	test	%si		/ check for s2 break into new segment
	jnz	srcok		/	skip if not
	mov	%ds,%dx		/ reset ds for next segment
	add	$8,%dx
	mov	%dx,%ds
srcok:
	test	%di		/ check for s1 break into new segment
	jnz	setn		/	skip if not
	mov	%es,%dx		/ reset es for next segment
	add	$8,%dx
	mov	$dx,%es
	jmp	setn		/ continue in next segment
#else
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
null:
	clr	%ax		/ return NULL
#if LARGE_M | HUGE_M | COMPACT_M
	cwd
#endif
	jmp	ret
ptr:
	mov	%di,%ax		/ return = &byte after last copied char
#if LARGE_M | HUGE_M | COMPACT_M
	mov	%es,%dx		/ <s>return = <s>&byte after last copied char
	test	%di		/ check for segment overflow
	jnz	ret		/	skip if not
	add	$8,%dx		/ 	else adjust <s>return
#endif
ret:
	pop	%di		/ restore registers
	pop	%cx
	pop	%bx
	LVRET			/ restore stack frame and return to caller
