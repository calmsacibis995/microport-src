	.file "xsqrt.s"
/	@(#)	1.1

/*	

	iAPX286 @(#)xsqrt.s	1.3 - 85/06/24

	80287 Math Library Square Root Function

	Global Function:

		xsqrt
*/

	.even
	.text
	.globl	xsqrt

/************************************************************************/
/*	xsqrt								*/
/************************************************************************/

xsqrt:
	enter	$0,$0
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)			/ load argument into 80287 TOS
#else
	fldl	4(%bp)			/ load argument into 80287 TOS
#endif
	fsqrt				/ perform square root function

	leave				/ answer is in %st(0)
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret				/ return to caller
#endif
