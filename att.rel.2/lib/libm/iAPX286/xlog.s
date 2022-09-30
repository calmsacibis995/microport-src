	.file "xlog.s"
/	@(#)	1.1

/*	

	iAPX286 @(#)xlog.s	1.1 - 85/06/24

	80287 Math Library Logarithm Functions

	Global Function:

		xlog
		xlog10
*/

#define CONST_VALUE	0.29289321881345247 	/ 1 - (sqrt(2) / 2)

	.data
	.even

	.globl	CONST_LOC

CONST_LOC:	.double 	CONST_VALUE

	.even
	.text

	.globl	xlog
	.globl  xlog10

/************************************************************************/
/*	xlog								*/
/************************************************************************/

xlog:
	enter	$0,$0			/ sp = bp
	push	%cx
	orb	%cl,%cl			/ clear xm1flag
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)			/ load argument into 80287 TOS
#else
	fldl	4(%bp)			/ load argument into 80287 TOS
#endif
	fld	%st(0)			/ copy TOS
	fld1				/ push 1.0 on TOS
	fsub				/ x - 1
	fabs				/ fabs(x-1)
	fldl	CONST_LOC		/ load CONST_VALUE
	fcomp	        	  	/ abs(x-1) < CONST_VALUE
	fstsw	%ax			/ load into ax register
	sahf				/ copy in flags
	jbe	l1
	fxch				/ exchange TOS
	orb	$1,%cl			/ xm1flag = true
l1:					/ x should be in %st(0)	
	fstp	%st(0)			/ pop TOS
	fldln2				/ load loge2 into %st(1)
	fxch
	orb	%cl,%cl			/ check for true value
	jz	l2			/ if xm1flag
	fyl2xp1				/ do it
	jmp	l3
l2:
	fyl2x

l3:
common_return:				/ answer is in %st(0)
	pop	%cx
	leave
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret				/ return to caller
#endif

/************************************************************************/
/*	xlog10								*/
/************************************************************************/

xlog10:
	enter	$0,$0			/ sp = bp
	push	%cx
	orb	%cl,%cl			/ clear xm1flag
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)			/ load argument into 80287 TOS
#else
	fldl	4(%bp)			/ load argument into 80287 TOS
#endif
	fld	%st(0)			/ copy TOS
	fld1				/ push 1.0 on TOS
	fsub				/ x - 1
	fabs				/ fabs(x-1)
	fldl	CONST_LOC		/ load CONST_VALUE
	fcomp	        	  	/ abs(x-1) < CONST_VALUE
	fstsw	%ax			/ load into ax register
	sahf				/ copy in flags
	jbe	l11
	fxch				/ exchange TOS
	orb	$1,%cl			/ xm1flag = true
l11:					/ x should be in %st(0)	
	fstp	%st(0)			/ pop the stack
	fldlg2				/ load log10 into %st(1)
	fxch
	orb	%cl,%cl			/ check for true value
	jz	l12			/ if xm1flag
	fyl2xp1				/ do it
	jmp	l13
l12:
	fyl2x
l13:
	jmp	common_return	
