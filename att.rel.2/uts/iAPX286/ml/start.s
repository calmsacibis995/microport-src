	.file	"start.s"
/
/ @(#)start.s	1.32
/	Enter protected mode and jump to main
/
/ NOTE:	All of memory is assumed to be zero.
/	The kernel at the present time does not
/	zero its own BSS.
/

#include	"../sys/mmu.h"
#include	"../sys/psl.h"
#include	"../sys/param.h"

FP_NO	=	0
FP_HW	=	1
FP_SW	=	2

/
/	The kernel in physical ram looks like this:
/
/	+-------------------------------+
/	|				|
/	|				|
/	|				|
/	|				|
/	|				|
/	+-------------------------------+
/	|        kernel code		|
/	|				|
/	+-------------------------------+
/	|        kernel data		|
/	|				|
/	| - - - - - - - - - - - - - - - |
/	|      215 wake-up block	|
/	+-------------------------------+
/	|           Padding		|
/	+-------------------------------+
/	|real mode intvects - 1024 bytes|
/	+-------------------------------+ 0000
/


/
/	Everything starts here
/
	.text
	.globl	strt
	.globl	_start
_start:
strt:
	cli				/ Interrupts off
	cld				/ forward mode
	xor	%ax,%ax			/ make a zero to send to the 215
					/ disk controller so it don't interrupt
	mov	$0x100,%dx		/ port of controller
	out	(%dx)			/ kick controller
	mov	$[PHYS_KDATA >> 4],%ax	/ get physical address of data and
					/ turn it into a segment value
	mov	%ax,%bx			/ save segment for later
	mov	%ax,%ds			/ to get at data
	mov	%ax,%ss			/ to get at stack
	mov	$kstack,%sp		/ to have a temp stack
/
/	fill in the gdt alias limit field
/
	mov	$gdt_end,%ax		/(end of gdt - begin of gdt) - 1 = limit
	sub	$gdt_base,%ax
	sub	$1,%ax
	mov	$gdtalias,%di		/ point to gdt alias descriptor
	mov	%ax,0(%di)		/ move limit into descriptor
/
/	calculate the physical address of the idt, and
/	fill in the idt alias descriptor with it.
/
	mov	$<s>idt_base,%bx	/ get selector of IDT
	and	$0xFFF8,%bx		/ turn into byte offset
	mov	$gdt_base,%di		/ get base of gdt
	mov	2(%bx,%di),%ax		/ get low physical base of segment
	movb	4(%bx,%di),%dl		/ get hi base of segment
	xorb	%dh,%dh			/ high byte not used
	add	$idt_base,%ax		/ add offset of IDT into low base
	adc	$0,%dx			/ propagate into hi base
	mov	$idt,%di		/ get offset of idt alias
	mov	%ax,2(%di)		/ store low phys base of idt
	movb	%dl,4(%di)		/ store high base ( byte ) of idt

/
/	calculate the physical address of the tss fault tss, and
/	fill in its tss descriptor with it.
/
	mov	$<s>tftss,%bx		/ get selector of tss
	and	$0xFFF8,%bx		/ turn into byte offset
	mov	$gdt_base,%di		/ get base of gdt
	mov	2(%bx,%di),%ax		/ get low physical base of segment
	movb	4(%bx,%di),%dl		/ get hi base of segment
	xorb	%dh,%dh			/ high byte not used
	add	$tftss,%ax		/ add offset of tss into low base
	adc	$0,%dx			/ propagate into hi base
	mov	$tftd,%di		/ get offset of tss descriptor
	mov	%ax,2(%di)		/ store low phys base of tss
	movb	%dl,4(%di)		/ store high base ( byte ) of tss

/
/	calculate the physical address of the double fault tss, and
/	fill in its tss descriptor with it.
/
	mov	$<s>dftss,%bx		/ get selector of tss
	and	$0xFFF8,%bx		/ turn into byte offset
	mov	$gdt_base,%di		/ get base of gdt
	mov	2(%bx,%di),%ax		/ get low physical base of segment
	movb	4(%bx,%di),%dl		/ get hi base of segment
	xorb	%dh,%dh			/ high byte not used
	add	$dftss,%ax		/ add offset of tss into low base
	adc	$0,%dx			/ propagate into hi base
	mov	$dftd,%di		/ get offset of tss descriptor
	mov	%ax,2(%di)		/ store low phys base of tss
	movb	%dl,4(%di)		/ store high base ( byte ) of tss

/
/	calculate the physical address of the wndump tss, and
/	fill in its tss descriptor with it.
/
	mov	$<s>wntss,%bx		/ get selector of tss
	and	$0xFFF8,%bx		/ turn into byte offset
	mov	$gdt_base,%di		/ get base of gdt
	mov	2(%bx,%di),%ax		/ get low physical base of segment
	movb	4(%bx,%di),%dl		/ get hi base of segment
	xorb	%dh,%dh			/ high byte not used
	add	$wntss,%ax		/ add offset of tss into low base
	adc	$0,%dx			/ propagate into hi base
	mov	$wntd,%di		/ get offset of tss descriptor
	mov	%ax,2(%di)		/ store low phys base of tss
	movb	%dl,4(%di)		/ store high base ( byte ) of tss

/
/	Prepare to enter hyperspace
/
	lgdt	gdtalias		/ point to our GDT
	lidt	idt			/ point to our IDT
	smsw	%ax			/ get MSW
	or	$MS_PE,%ax		/ set protected mode
	lmsw	%ax			/ PROTECTION ON
	jmp	flush			/ flush pre-fetch queue (required)
flush:
	xor	%ax,%ax			/ initial value for LDT
	lldt	%ax			/ initialize LDT
	ljmp	nxtinst			/ start execution in the GDT
nxtinst:
	mov	$[KDS_SEL << 3],%ax	/ get kernel protected mode DS
	mov	%ax,%ds			/ set data segment
					/ no stack for a while
/
/	calculate the physical addresses for process 0 tss, ldt, & upage
/	and fill in the pslot & upage descriptors with them.
/	we can use PHYS_KDATA instead of having to find the base
/	in the GDT because we know the base must equal PHYS_KDATA
/
	mov	$<s>p0tsssel,%ax	/ get the proc0 tss selector
	mov	%ax,%es			/ load the proc 0 tss selector
	mov	$p0tsssel,%di		/ and the offset
	mov	$PHYS_KDATA,%ax		/ get physical address of data
	add	$p0tss,%ax		/ add offset of p0tss to base
	adc	$0,%dx			/ propagate into hi base
	mov	%ax,%es:2(%di)		/ store low phys base of proc 0 tss
	movb	%dl,%es:4(%di)		/ store high base ( byte ) of tss
	mov	%ax,%es:10(%di)		/ store low base of proc 0 tss alias
	movb	%dl,%es:12(%di)		/ store high base ( byte ) of alias
	mov	$[PSLOT_SEL << 3],%bx	/ get proc 0 tss desc selector
	ltr	%bx			/ set tss register

	mov	$<s>p0ldtsel,%ax	/ get the proc 0 ldt sel
	mov	%ax,%es			/ load the proc 0 ldt selector
	mov	$p0ldtsel,%di		/ and the offset
	mov	$PHYS_KDATA,%ax		/ get physical address of data
	add	$p0ldt,%ax		/ add offset of p0ldt to base
	adc	$0,%dx			/ propagate into hi base
	mov	%ax,%es:2(%di)		/ store low phys base of proc 0 ldt
	movb	%dl,%es:4(%di)		/ store high base ( byte ) of ldt
	mov	%ax,%es:10(%di)		/ store low base of proc 0 ldt alias
	movb	%dl,%es:12(%di)		/ store high base ( byte ) of alias
	mov	$[[PSLOT_SEL + 2] << 3],%bx	/ get proc 0 ldt desc selector
	lldt	%bx			/ set LDT register

	mov	$<s>p0usel,%ax		/ get the proc0 u selector
	mov	%ax,%es			/ load the proc 0 u selector
	mov	$p0usel,%di		/ and the offset
	mov	$PHYS_KDATA,%ax		/ get physical address of data
	add	$p0u,%ax		/ add offset of p0u to base
	adc	$0,%dx			/ propagate into hi base
	mov	%ax,%es:2(%di)		/ store low phys base of proc 0 u
	movb	%dl,%es:4(%di)		/ store high base ( byte ) of u

	mov	$[[UPAGE_SEL << 3] | LDT_TI],%ax / get selector ...
	mov	%ax,%ss			/ ... for stack segment
	mov	$1024,%sp		/ establish kernel stack

/
/	Now, determine if we have a 80287, and put the appropriate
/	value into fp_kind
/
	smsw	%dx			/ get 286 status
	fninit				/ initialize 80287
	fstsw	%ax			/ get 80287 status
	orb	%al,%al			/ status zero? 0 = 80287 present
	jnz	em			/ no, use emulator
	and	$[0xFFFF-MS_EM],%dx
	or	$MS_MP,%dx		/ yes, set math present bit
	lmsw	%dx			/ in machine status word
	fsetpm				/ set the 80287 into protected mode
	mov	$<s>fp_kind,%ax		/ point to fp_kind
	mov	%ax,%es
	movb	$FP_HW,%es:fp_kind	/ signify that we have hardware
	jmp	cont
em:
	lcall	emul_present		/ do we have an emulator?
	test	%ax
	jz	noemul			/ emulator not present
	and	$[0xFFFF-MS_MP],%dx
	or	$MS_EM,%dx		/ set emulate math bit
	lmsw	%dx			/ in machine status word
	mov	$<s>fp_kind,%ax		/ point to fp_kind
	mov	%ax,%es
	movb	$FP_SW,%es:fp_kind	/ signify that we are emulating
	jmp	cont
noemul:
	mov	$<s>fp_kind,%ax		/ point to fp_kind
	mov	%ax,%es
	movb	$FP_NO,%es:fp_kind	/ no floating point allowed

cont:

/
/	call main to complete the initialization
/
	lcall	<s>main,main
/
/	Upon return from main, %dx is a selector and %ax is an offset.
/	A Non-zero selector means start the second kernel process by calling
/	the routine at the selector:offset. (xsched)
/	A zero selector means start the first process in user mode. (icode)
/	Unless the offset is zero which means halt.  (debugging)
/
	test	%dx			/ is it init process?
	jnz	proc2			/ ...no, go start process 2
	test	%ax			/ bogus stack offset?
	jz	death			/ ...yes, time for seppuku
	mov	$[[[CODE1_SEL + 1] << 3] | [LDT_TI | USER_RPL]],%dx
					/ save selector of data/stack
	push	%dx			/ selector of stack segment
	push	%ax			/ initial stack offset
	pushf				/ initial flags (just need IF on)
	push	$[[CODE1_SEL << 3] | [LDT_TI | USER_RPL]]
					/ selector of code segment
	push	$0			/ initial instruction pointer
	mov	%dx,%ds			/ proc 1 needs to get at data too
	iret				/ go for it

proc2:
	push	%dx			/ selector of routine
	push	%ax			/ offset of routine
	mov	%sp,%bp			/ get a pointer to the routine address
	lcall	*0(%bp)			/ call the routine

death:
	cli				/ shouldn't get to here
	hlt				/ so freeze up
	jmp	death			/ and stay that way
