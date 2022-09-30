	.file	"xexp.s"
/	@(#)	1.1

/*

	iAPX286 @(#)xexp.s	1.3 - 85/06/24

	80287 Math Library Exponential Function

	Global Functions:

		xexp		- exponentiation

		xldexp		- raise value to x power of 2	

*/

	.even
	.text
	.globl	xexp
	.globl  xldexp


/************************************************************************/
/*	xexp								*/
/************************************************************************/
xexp:
	enter	$0,$0
#ifdef LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)			/ load argument into 80287 TOS
#else
	fldl	4(%bp)			/ load argument into 80287 TOS
#endif
	f2xm1				/ TOS = 2x-1
	fld1				/ load 1.0 into TOS+1	
	fadd	           		/ TOS == 2 to the x

	leave
#ifdef LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret				/ return to caller
#endif

/************************************************************************/
/*	ldxexp								*/
/************************************************************************/
xldexp:
	enter	$0,$0
#ifdef LARGE_M | HUGE_M | COMPACT_M
	fild	14(%bp)			/ scale factor
	frndint
	fldl	6(%bp)			/ vector
#else
	fild	12(%bp)			/ scale factor
	frndint
	fldl	4(%bp)			/ vector
#endif
	fscale
	fstp	%st(1)			/ pop the scale factor
					/ answer is in %st(0)
	leave
#ifdef LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret				/ return to caller
#endif
