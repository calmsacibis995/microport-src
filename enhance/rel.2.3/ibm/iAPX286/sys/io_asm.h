/* @(#)io_asm.h	1.1 */

/*
 * Machine Common and Specific Definitions
 */

#ifdef	MP386
	.set	P1,8		/* offset to first function argument */
#define	AX	%eax		/* define integer register references */
#define	BP	%ebp
#define	CX	%ecx
#define	DI	%edi
#define	DSI			/* not used on 386 */
#define	DX	%edx
#define	ESI			/* not used on 386 */
#define	INT	4		/* sizeof (int) */
#define	LDS	movl		/* not used on 386 */
#define	LES	movl		/* not used on 386 */
#define	LOOPCNT	8000
#define	MOVINT	movl		/* move integer */
#define	POPDI	pushl	DI
#define	POPFL
#define	POPSI	pushl	SI
#define	PORT	4		/* sizeof (port adrs argument) */
#define	PUSHDI	pushl	DI
#define	PUSHFL
#define	PUSHSI	pushl	SI
#define	RET	ret
#define	SI	%esi
#define	STI	sti

#else
	.set	P1,6		/* offset to first function argument */
#define	addw	add
#define	andw	and
#define	AX	%ax
#define	BP	%bp
#define	CX	%cx
#define	DI	%di
#define	DSI	%ds:
#define	DX	%dx
#define	ESI	%es:
#define	INT	2		/* sizeof (int) */
#define	LDS	lds
#define	LES	les
#define	LOOPCNT	4000
#define	MOVINT	mov		/* move integer */
#define	movw	mov		/* move word (short) */
#define	POPDI
#define	POPFL
#define	POPSI
#define	PORT	2		/* sizeof (port adrs argument) */
#define	PUSHDI
#define	PUSHFL
#define	PUSHSI
#define	RET	lret
#define	SI	%si
#define	STI	sti
#define	subw	sub
#define	testw	test

#endif

