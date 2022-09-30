	.file	"setjmp.s"
/	@(#)	1.1

/	longjmp(env, val)
/ will generate a "return(val)" from
/ the last call to
/	setjmp(env)
/ by restoring registers cs, ip, sp, bp, di, cx, and bx from 'env'
/ and doing a return.

/ entry    reg	offset from (%si)
/ env[0] = %bx	 0
/ env[1] = %cx	 2	/ register variable in small and middle models
/ env[2] = %di	 4	/ register variable in small and middle models
/ env[3] = %bp	 6
/ env[4] = %sp	 8
/ env[5] = %ip	10
/ env[6] = %cs	12	/ middle, large, and huge models only

#include	"lib.s.h"
	.text
	.globl	longjmp
	.globl	setjmp

setjmp:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
#if LARGE_M | HUGE_M | COMPACT_M
	lds	PARAM(1),%si	/ ds,si = &env[0]
#else
	mov	PARAM(1),%si	/ ds,si = &env[0]
#endif
	mov	%bx,(%si)	/ save bx
	mov	%cx,2(%si)	/ save cx
	mov	%di,4(%si)	/ save di
	mov	0(%bp),%ax	/ save caller's bp
	mov	%ax,6(%si)
	lea	PARAM(1),%ax	/ save caller's sp
	mov	%ax,8(%si)
	mov	2(%bp),%ax	/ save caller's ip
	mov	%ax,10(%si)
#if LARGE_M | HUGE_M | MIDDLE_M
	mov	4(%bp),%ax	/ save caller's cs
	mov	%ax,12(%si)
#endif
	clr	%ax		/ return 0
	LVRET			/ restore stack frame and return to caller

longjmp:
	push	%bp		/ save old bp
	mov	%sp,%bp
	MCOUNT			/ call subroutine entry counter if profiling
#if LARGE_M | HUGE_M | COMPACT_M
	lds	PARAM(1),%si	/ ds,si = &env[0]
	mov	PARAM(3),%ax	/ ax = val
#else
	mov	PARAM(1),%si	/ ds,si = &env[0]
	mov	PARAM(2),%ax	/ ax = val
#endif
	mov	(%si),%bx	/ restore bx
#if SMALL_M | MIDDLE_M
	mov	0(%bp),%dx	/ if called from the same function as setjmp,
	cmp	%dx,6(%si)
	jz	afterregs	/	don't touch register variables
#endif
	mov	2(%si),%cx	/ restore cx
	mov	4(%si),%di	/ restore di
afterregs:
	mov	6(%si),%bp	/ restore caller's bp
	mov	8(%si),%sp	/ restore caller's sp

	test	%ax		/ if val != 0
	jnz	ret		/ 	return val
	inc	%ax		/ else return 1
ret:
	JMP	*10(%si)	/ return to caller
