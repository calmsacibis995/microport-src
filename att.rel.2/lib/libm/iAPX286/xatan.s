	.file "xatan.s"
/	@(#)	1.1

/*	

	iAPX286 @(#)xatan.s	1.1 - 85/06/24

	80287 Math Library Arctangent Function

	Global Function:

		xatan
*/

	.even
	.text
	.globl	xatan

/************************************************************************/
/*	xatan								*/
/************************************************************************/

xatan:
	enter	$0,$0
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	fldl	6(%bp)			/ load Y argument into 80287 TOS
	fldl	14(%bp)			/ load X argument into 80287 TOS
#else
	fldl	4(%bp)			/ load Y argument into 80287 TOS
	fldl	12(%bp)			/ load X argument into 80287 TOS
#endif
	fpatan				/ perform arctangent function
					/ answer is in %st(0)
	leave
#ifdef	LARGE_M | HUGE_M | COMPACT_M
	lret
#else
	ret				/ return to caller
#endif
