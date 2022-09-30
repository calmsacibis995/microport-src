	.file	"memset.s"
/	@(#)	1.1

/ Set an array of n chars starting at sp to the character c.
/	
/	char *
/	memset(sp, c, n)
/	register char *sp, c;
/	register int n;
/	{
/		register char *sp0 = sp;
/
/		while (--n >= 0)
/			*sp++ = c;
/		return (sp0);
/	}

/ es,di = sp
/ al = c
/ cx = n
/ bx = n_left
/ dx = temp

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	memset

memset:
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
	mov	%ds,%dx		/ es,di = sp
	mov	%dx,%es
	mov	PARAM(1),%di
	mov	PARAM(2),%ax	/ al = c
	mov	PARAM(3),%cx	/ cx = n
#endif
	test	%cx		/ verify that n > 0
	jle	out		/	skip if not
setn:
	clr	%bx		/ initialize n_left
	mov	%di,%dx		/ check for sp segment overflow before n == 0
	neg	%dx
	jz	lp		/ skip if no overflow possible
	cmp	%cx,%dx
	jae	lp		/ skip if remainder of segment >= n
	add	%cx,%bx		/ reset n_left and n
	sub	%ax,%bx
	mov	%dx,%cx
lp:
	rep ;	sstob		/ *sp++ = c until n == 0
	test	%bx		/ done?
	jz	out		/	skip if done
#if HUGE_M
	mov	%es,%dx		/ reset sp for next segment
	add	$8,%dx
	mov	$dx,%es
	mov	%bx,%cx		/ set n = n_left
	jmp	setn		/ go again
#else
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
out:
	pop	%bx		/ restore registers
	pop	%cx
	pop	%di
	mov	PARAM(1),%ax	/ return original sp
#if LARGE_M | HUGE_M | COMPACT_M
	mov	PARAM(2),%dx
#endif
	LVRET			/ restore stack frame and return to caller
