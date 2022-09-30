	.file	"mcrt0.s"
/	@(#)	1.2
#include	"lib.s.h"
	.globl	_start
	.globl	environ
	.globl	exit
	.globl	IEH3exit

/ PTRSHIFT = shift count to convert data ptr count to data ptr # bytes
/ PTRSIZE = # bytes in ptr to data
/ TPTRSIZE = # bytes in ptr to text
#if LARGE_M | HUGE_M | COMPACT_M
#define	PTRSHIFT	2
#define	PTRSIZE		4
#else
#define	PTRSHIFT	1
#define	PTRSIZE		2
#endif
#if LARGE_M | HUGE_M | MIDDLE_M
#define	TPTRSIZE	4
#else
#define	TPTRSIZE	2
#endif

#define	CNTC	600
#define	CNT	8
#define	HDR	14

	.data
.enospc:
	.byte	0x4e,0x6f,0x20,0x73,0x70,0x61,0x63,0x65	/ `No space'
	.byte	0x20,0x66,0x6f,0x72,0x20,0x6d,0x6f,0x6e	/ ` for mon'
	.byte	0x69,0x74,0x6f,0x72,0x20,0x62,0x75,0x66	/ `itor buf'
	.byte	0x66,0x65,0x72,0x0a,0x00		/ `fer\n\0'
.em1:

	.bss
environ:
	.set	.,. + PTRSIZE

/
/ The following code must be located at offset 0 in the 1st text segment of
/	the process.  It is used as a return address from user level interrupt
/	catching routines to clear the args for the interrupt routine and
/	do the interrupt return to restore the stack to the proper state.

	.text
	add	$4,%sp			/ remove args to user interrupt routine
	lcall	SIGCALL			/ return to kernel to return to user

/
/ C language startup routine.

_start:
	cld				/ initialize string operations direction
	sub	$[PTRSIZE + 2],%sp	/ create an initial stack frame
	mov	%sp,%bp
#if SMALL_M | MIDDLE_M
	mov	$<s>environ,%ax		/ initialize data segment register
	mov	%ax,%ds
	mov	%ax,%es			/ and extra segment
#else
	push	%ss			/ <s>envp
#endif
	mov	PARAM(1),%ax		/ envp = (argc+1)*PTRSIZE+&PARAM(2)
	inc	%ax			/	%ax = (argc + 1)
	sal	$PTRSHIFT,%ax		/		* PTRSIZE
	lea	PARAM(2),%bx		/		+ &PARAM(2)
	add	%bx,%ax
	push	%ax			/ envp
#if LARGE_M | HUGE_M | COMPACT_M
	mov	$<s>environ,%dx		/ set up environ = envp
	mov	%dx,%es
	mov	%ss,%es:environ+2
	mov	%ax,%es:environ
	push	%ss			/ <s>argv
#else
	mov	%ax,environ		/set up environ = envp
#endif
	push	%bx			/ argv
#if LARGE_M | HUGE_M | COMPACT_M
	mov	$<s>spmin,%dx		/ set up min/max stacksize variables
	mov	%dx,%es
	mov	%sp,%dx
	mov	%dx,%es:spmin
	mov	%dx,%es:spmax
	mov	$<s>___Argv,%dx		/ save program name for profiling
	mov	%dx,%es
	mov	%ss,%es:___Argv+2
	mov	%bx,%es:___Argv
#else
	mov	%sp,%dx			/ set up min/max stacksize variables
	mov	%dx,spmin
	mov	%dx,spmax
	mov	%bx,___Argv		/ save program name for profiling
#endif
	push	PARAM(1)		/ argc

	push	$CNTC			/ nfunc (arg5) for monitor
#if LARGE_M | HUGE_M | MIDDLE_M
	mov	%cs,%ax			/ Verify that .eprol and etext are
	cmp	$<s>etext,%ax		/	in same segment.  If not,
	jne	.maxspac		/	get 32K.
#endif
	mov	$etext,%ax		/ calculate buffer size =
	sub	$.eprol,%ax		/	text size
	inc	%ax			/	rounded to a word
	and	$0xfffe,%ax
	js	.maxspac		/ get 32K if buffer >= 32K
	add	$[CNT \* CNTC + HDR],%ax/ add in hdr and cnt structures space
	js	.maxspac		/ get 32K if buffer >= 32K
	jmp	.alloc
.maxspac:
	mov	$0x7ffe,%ax
.alloc:
	sar	%ax			/ cnvt `byte' count to `short' count
	push	%ax			/ bufsize (arg4) for monitor

	sal	%ax			/ cnvt back to `byte' for sbrk
	push	%ax			/ incr for sbrk
	CALL	sbrk			/ allocate buffer
	add	$2,%sp
	cmp	$-1,%ax			/ if we couldn't allocate buffer,
	je	_nospace		/	get out

#if LARGE_M | HUGE_M | COMPACT_M
	push	%dx			/ <s>buffer (arg3) for monitor
	mov	$<s>_countbase,%bx	/ initialize <s>countbase
	mov	%bx,%es
	mov	%dx,%es:_countbase+2
#endif
	push	%ax			/ buffer (arg3) for monitor
	add	$HDR,%ax		/ initialize countbase
#if LARGE_M | HUGE_M | COMPACT_M
	mov	%ax,%es:_countbase
#else
	mov	%ax,_countbase
#endif
#if LARGE_M | HUGE_M | MIDDLE_M
	push	$<s>etext		/ <s>highpc (arg2) for monitor
#endif
	push	$etext			/ highpc (arg2) for monitor
#if LARGE_M | HUGE_M | MIDDLE_M
	push	%cs			/ <s>lowpc (arg1) for monitor
#endif
	push	$.eprol			/ lowpc (arg1) for monitor
	CALL	monitor			/ set up profiling
	add	$[TPTRSIZE \* 2 + PTRSIZE + 4],%sp	/ clear monitor args
	CALL	main
	add	$[2 \* PTRSIZE + 2],%sp	/ if main returns, clear args
	push	%ax			/	and call exit
	CALL	exit			/ exit will never return

_nospace:
	push	$[.em1 - .enospc]	/ can't allocate profiling buffer
#if LARGE_M | HUGE_M | COMPACT_M
	push	$<s>.enospc
#endif
	push	$.enospc		/ write error message on stderr and
	push	$2			/	exit(1)
	CALL	write
	add	$[2 + PTRSIZE + 2],%sp
	push	$1
	CALL	exit

exit:					/ replacement for exit sys call
IEH3exit:				/	to turn off profiling, flush
	push	%bp			/	stdio buffers, and do exit
	mov	%sp,%bp			/	system call
#if LARGE_M | HUGE_M | COMPACT_M
	push	$0			/ <s>NULL
#endif
	push	$0			/ NULL
	CALL	monitor			/ turn off profiling and dump mon.out
	add	$PTRSIZE,%sp
	CALL	_cleanup		/ flush stdio buffers
	push	PARAM(1)
	push	$EXIT
	lcall	SYSCALL			/ exit syscall
	hlt
.eprol:
