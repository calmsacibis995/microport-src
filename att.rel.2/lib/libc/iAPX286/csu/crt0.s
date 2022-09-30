	.file	"crt0.s"
/	@(#)	1.1
#include	"lib.s.h"
	.globl	_start
	.globl	_mcount
	.globl	environ

#if LARGE_M | HUGE_M | COMPACT_M
#define	PTRSHIFT	2
#define	PTRSIZE		4
#else
#define	PTRSHIFT	1
#define	PTRSIZE		2
#endif

	.bss
environ:
	.set	.,. + PTRSIZE
	.text

/
/ The following code must be located at offset 0 in the 1st text segment of
/	the process.  It is used as a return address from user level interrupt
/	catching routines to clear the args for the interrupt routine and
/	do the interrupt return to restore the stack to the proper state.

	add	$4,%sp			/ remove args to user interrupt routine
	lcall	SIGCALL			/ return to kernel to return to user

/ The following routine is here in case any object module compiled with cc -p
/	was linked into this module.

_mcount:
	RET

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
	push	PARAM(1)		/ argc

	CALL	main
	add	$[2 \* PTRSIZE + 2],%sp	/ if main returns, clear args
	push	%ax			/	and call exit
	CALL	exit
	push	$EXIT			/ if user redefined exit, do the
	lcall	SYSCALL			/	system call here
	hlt
