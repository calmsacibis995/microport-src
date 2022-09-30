/* uportid = "@(#)fp.h	Microport Rev Id  1.3.3 6/18/86" */
/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)fp.h	1.6 */
/*
** 80287 floating point processor definitions
*/

/*
** values that go into fp_kind
*/
#define	FP_NO	0	/* no 80287, no emulator ( no fp supported )	*/
#define	FP_HW	1	/* 80287 is present				*/
#define	FP_SW	2	/* 80287 not present; using emulator		*/

/*
** values that are passed to fpsave()
*/
#define	FPCHECK		1	/* check for validity before saving	*/
#define	FPNOCHECK	0	/* don't check for validity		*/

/*
** masks for 80287 control word
*/
#define	FPINV	0x0001		/* invalid operation			*/
#define	FPDNO	0x0002		/* denormalized operand			*/
#define	FPZDIV	0x0004		/* zero divide				*/
#define	FPOVR	0x0008		/* overflow 				*/
#define	FPUNR	0x0010		/* underflow				*/
#define	FPPRE	0x0020		/* precision				*/
#define	FPPC	0x0300		/* precision control			*/
#define	FPRC	0x0C00		/* rounding control			*/
#define	FPIC	0x1000		/* infinity control			*/
/*
** precision, rounding, and infinity options in control word
*/
#define	FPSIG24	0x0000		/* 24-bit significand precision	(short)	*/
#define	FPSIG53	0x0200		/* 53-bit significand precision	(long)	*/
#define	FPSIG64	0x0300		/* 64-bit significand precision	(temp)	*/
#define	FPRTN	0x0000		/* round to nearest or even		*/
#define	FPRD	0x0400		/* round down				*/
#define	FPRU	0x0800		/* round up				*/
#define	FPCHOP	0x0C00		/* chop (truncate toward zero)		*/
#define	FPP	0x0000		/* projective infinity			*/
#define	FPA	0x1000		/* affine infinity			*/

extern char		fp_kind;
extern struct proc	*fp_proc;
