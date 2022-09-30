	.file	"memchr.s"
/	@(#)	1.1

/ Return the ptr in sp at which the character c appears;
/   NULL if not found in n chars; don't stop at \0.
/	
/	char *
/	memchr(sp, c, n)
/	register char *sp, c;
/	register int n;
/	{
/		while (--n >= 0)
/			if (*sp++ == c)
/				return (--sp);
/		return (0);
/	}

/ es,di = sp
/ cx = n
/ al = c
/ dx = temp
/ bx = n_left

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	memchr

memchr:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%di		/ save registers
	push	%cx
	push	%bx
#if LARGE_M | HUGE_M | COMPACT_M
	les	PARAM(1),%di	/ es,di = sp
	mov	PARAM(3),%ax	/ al = c
	mov	PARAM(4),%cx	/ cx = n
#else
	mov	%ds,%ax		/ es,di = sp
	mov	%ax,%es
	mov	PARAM(1),%di
	mov	PARAM(2),%ax	/ al = c
	mov	PARAM(3),%cx	/ cx = n
#endif
	test	%cx		/ verify that n > 0
	jle	retnull		/	skip if not
setn:
	clr	%bx		/ initialize n_left after this time thru loop
	mov	%di,%dx		/ check for segment overflow before n == 0
	neg	%dx
	jz	lp		/ skip if no overflow possible
	cmp	%cx,%dx
	jae	lp		/ skip if remainder of segment >= n
	add	%cx,%bx		/ reset n to end of segment and set n_left to
	sub	%dx,%bx		/	count for next time through loop
	mov	%dx,%cx
lp:
	repnz ; scab		/ loop until found or n == 0
	jz	retptr		/	skip if found
	test	%bx		/ done?
	jz	retnull		/	skip if done
#if HUGE_M
	mov	%es,%dx		/ set up for next segment
	add	$8,%dx
	mov	%dx,%es
	mov	%bx,%cx		/ set n for next segment
	jmp	setn		/ go back for next segment
#else
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
retnull:
	clr	%ax		/ not found, return NULL
#if LARGE_M | HUGE_M | COMPACT_M
	cwd
#endif
	jmp	ret
retptr:
	dec	%di		/ reset to point to last character checked
	mov	%di,%ax		/ return pointer
#if LARGE_M | HUGE_M | COMPACT_M
	mov	%es,%dx
#endif
ret:
	pop	%bx		/ restore registers
	pop	%cx
	pop	%di
	LVRET			/ restore stack frame and return to caller
