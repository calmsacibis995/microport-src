	.file	"end.s"
/
/ @(#)end.s	1.10
/
#include	"../sys/mmu.h"

EXECE		=	59		/ exec system call number
ICODES		=	0x5F		/ selector for LDT slot 11

	.data

/
/ icode
/	Bootstrap program (small model)
/	executed in user mode to bring up the system
/
	.globl	icode
	.globl	szicode
icode:
	sub	$6,%sp				/ room for 10 args on stack
	push	$ICODES				/	when sys call entered
	push	$envpoff			/ environment
	push	$ICODES
	push	$argvoff			/ arg list
	push	$ICODES
	push	$initoff			/ name
	push	$EXECE				/ system call number
	lcall	[SCALL_SEL << 3],0		/ call thru call gate
loop:
	jmp	loop				/ loop if scall fails
init:
	.byte	0x2F,0x65,0x74,0x63		/ "/etc"
	.byte	0x2F,0x69,0x6E,0x69,0x74	/ "/init"
	.byte	0x00
	.even
argv:
	.value	[init-icode]			/ point to string
	.value	0				/ terminate with NULL pointer
envp:
	.value	0				/ terminate with NULL pointer

/
/ define constants here because
/ assembler won't allow a forward reference at the push
/
envpoff =	[envp-icode]			/ envp offset
argvoff	=	[argv-icode]			/ argv offset
initoff	=	[init-icode]			/ init offset

	.even					/ for copyout's sake
szicode:
	.value	[szicode-icode]			/ size of icode

/
/	type of machine we are on
/
	.globl	cputype
cputype:
	.value	286
