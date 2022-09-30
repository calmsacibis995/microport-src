	.file	"strchr.s"
/	@(#)	1.1

/ Return the ptr in sp at which the character c appears;
/   NULL if not found

/	char *
/	strchr(sp, c)
/	register char *sp, c;
/	{
/		do {
/			if (*sp == c)
/				return(sp);
/		} while (*sp++);
/		return(0);
/	}

/ al = string work area
/ bl = c
/ cx = segment length
/ dx = temp
/ ds,si = sp

#include	"lib.s.h"
#include	"sys/signal.h"
	.text
	.globl	strchr

strchr:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
	push	%cx		/ save registers
	push	%bx
#if LARGE_M | HUGE_M | COMPACT_M
	lds	PARAM(1),%si	/ ds,si = sp
	mov	PARAM(3),%bx	/ bl = c
#else
	mov	PARAM(1),%si	/ ds,si = sp
	mov	PARAM(2),%bx	/ bl = c
#endif
setc:
	mov	%si,%cx		/ set cx to max count before segment overflow
	neg	%cx
	jnz	lp
	dec	%cx
lp:
	slodb			/ *sp++ ==
	cmpb	%al,%bl		/	c?
	je	found		/ skip if yes
	testb	%al		/ end of string?
	loopnz	lp		/ loop until end of string or count == 0
	jz	notfound	/ skip if end of string
	test	%si		/ end of segment?
	jnz	setc		/ if not, go back and try again
#if HUGE_M
	mov	%ds,%dx		/ reset for next segment
	add	$8,%dx
	mov	%dx,%ds
	jmp	setc		/ continue in next segment
#else
	push	$SIGSEGV	/ segment overflow, generate fault
	CALL	getpid
	push	%ax
	CALL	kill		/ kill(getpid(), SIGSEGV);
	add	$4,%sp
#endif
notfound:
	clr	%ax		/ c not found; return NULL
#if LARGE_M | HUGE_M | COMPACT_M
	cwd
#endif
	jmp	ret
found:
	mov	%si,%ax		/ c found; return ptr to it
	dec	%ax
#if LARGE_M | HUGE_M | COMPACT_M
	mov	%ds,%dx
#endif
ret:
	pop	%bx		/ restore registers
	pop	%cx
	LVRET			/ restore stack frame and return to caller
