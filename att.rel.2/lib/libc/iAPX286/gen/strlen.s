	.file	"strlen.s"
/	@(#)	1.1

/ Given string s, return length (not including the terminating null).

/	strlen(s)
/	register char *s;
/	{
/		register n;
/	
/		n = 0;
/		while (*s++)
/			n++;
/		return(n);
/	}

/ ax = end of string test character
/ cx = # bytes left in segment
/ dx = temp
/ es,di = s

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strlen

strlen:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%di		/ save registers
	push	%cx
#if LARGE_M | HUGE_M | COMPACT_M
	les	PARAM(1),%di	/ es,di = s
#else
	mov	PARAM(1),%di	/ es,di = s
	mov	%ds,%dx
	mov	%dx,%es
#endif
	clrb	%al		/ set end of string character
lp:
	mov	%di,%cx		/ set count to # bytes left in segment
	neg	%cx
	jnz	doit		/ check for whole segment, skip if not
	dec	%cx		/	else set to do all but 1 byte
doit:
	repnz ;	scab		/ loop until \0 found or count ran out
	jz	ret		/ skip if \0 found
	test	%di		/ end of segment?
	jnz	lp		/	if no, go again
#if HUGE_M
	mov	%es,%dx		/ set up for next segment
	add	$8,%dx
	mov	%dx,%es
	jmp	lp		/ continue in next segment
#else
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
ret:
	mov	%di,%ax		/ calculate length = end - start - 1
	sub	PARAM(1),%ax	/	NOTE:  Don't worry about overflow; we
	dec	%ax		/	are returning an int.
	pop	%cx		/ restore registers
	pop	%di
	LVRET			/ restore stack frame and return to caller
