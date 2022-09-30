/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#ifndef MARK
#define MARK(L)	{}
#else
#undef MARK
#ifdef vax
#define MARK(L)	{\
		asm("	.data");\
		asm("	.align	4");\
		asm(".L.:");\
		asm("	.long	0");\
		asm("	.text");\
		asm("M.L:");\
		asm("	nop;nop");\
		asm("	movab	.L.,r0");\
		asm("	jsb	mcount");\
		}
#endif
#ifdef u3b
#define MARK(L)	{\
		asm("	.data");\
		asm("	.align	4");\
		asm(".L.:");\
		asm("	.word	0");\
		asm("	.text");\
		asm("M.L:");\
		asm("	movw	&.L.,%r0");\
		asm("	jsb	_mcount");\
		}
#endif
#ifdef pdp11
#define MARK(L)	{\
		asm("	.bss");\
		asm(".L.:");\
		asm("	.=.+2");\
		asm("	.text");\
		asm("M.L:");\
		asm("	mov	$.L.,r0");\
		asm("	jsr	pc,mcount");\
		}
#endif
#if iAPX286 && HUGE_M
#define MARK(L)	{\
		asm("	.data");\
		asm("	.even");\
		asm(".L.:");\
		asm("	.value	0,0");\
		asm("	.text");\
		asm("M.L:");\
		asm("	mov	$.L.,%si");\
		asm("	mov	$<s>.L.,%bx");\
		asm("	lcall	_mcount");\
		}
#endif
#if iAPX286 && ( LARGE_M || COMPACT_M )
#define MARK(L)	{\
		asm("	.data");\
		asm("	.even");\
		asm(".L.:");\
		asm("	.data");\
		asm("	.value	0");\
		asm("	.text");\
		asm("M.L:");\
		asm("	mov	$.L.,%si");\
		asm("	mov	$<s>.L.,%bx");\
		asm("	lcall	_mcount");\
		}
#endif
#if iAPX286 && ( SMALL_M || MIDDLE_M )
#define MARK(L)	{\
		asm("	.data");\
		asm("	.even");\
		asm(".L.:");\
		asm("	.value	0");\
		asm("	.text");\
		asm("M.L:");\
		asm("	mov	$.L.,%si");\
		asm("	call	_mcount");\
		}
#endif
#endif
