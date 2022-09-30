	.file	"start.s"
/ uportid = "@(#)start.s	Microport Rev Id  1.3.8 10/19/86";
/
/ @(#)start.s	1.32
/	Enter protected mode and jump to main
/
/ NOTE:	All of memory is assumed to be zero.
/	The kernel at the present time does not
/	zero its own BSS.
/

/ Copyright @ 1985, 1986 Microport Systems. All Rights Reserved.
/ 
/ Upgraded for the IBM AT:	uport!dwight
/ 	1) Added code to gate address bit 20 right before prot. mode.
/
/ Note: This code is not completely backwards compatible with the
/	original 310 release. The sections dealing with the 215 have
/	been stripped out for clarity's sake.
/
#include	"sys/mmu.h"
#include	"sys/psl.h"
#include	"sys/param.h"

#ifdef	IBMAT
/* useful info for future porting (e.g. 386):	*/
/* #define	PHYS_KDATA	0x7E000		*/
#undef	PSLOT_SEL
#define	PSLOT_SEL		90	/* process slot selector	*/
#endif	IBMAT

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

#ifdef ATPROBE
/  debug bkpt #0
	mov	$0, %ax
	outb	0x80
#endif /* ATPROBE */

#ifdef ATMERGE
/
/ calculate the physical address of etext, and save it for later use
/ 
	mov	$<s>etext, %bx		/ get selector of etext
	and	$0xfff8, %bx		/ turn into byte offset
	mov	$gdt_base, %di		/ get base of gdt
	mov	2(%bx,%di), %ax		/ get low physical base of segment
	movb	4(%bx,%di), %cl		/ get hi base of segment
	xorb	%ch, %ch		/ high byte not used
	add	$etext, %ax		/ add offset of etext into low base
	adc	$0, %cx			/ propagate into hi base
	mov	%ax, aetext		/ store low word of address
	mov	%cx, aetext + 2		/ store high word of address
#endif /* ATMERGE */

#ifdef ATPROBE
/
/ Set up the Atron probe code descriptor, for debugging
/
	mov	$[PROBE_SEL << 3], %bx	/ get selector for Probe
	mov	$gdt_base, %di		/ get base of gdt
	xor	%ax, %ax
	mov	%ax, %es		/ address real mode vectors
	mov	%es:6, %ax		/ fetch CS from int 1 vector
	mov	%ax, %cx
	shrb	$4, %ah			/ ah is high byte of address
	shl	$4, %cx			/ cx is low word of address
	mov	%cx, 2(%bx,%di)		/ store low phys base of probe segment
	movb	%ah, 4(%bx,%di)		/ store high base (byte) of probe seg
#endif /* ATPROBE */

#ifdef ATMERGE 
/ There are several variables established during startup that are used
/ later on to initialize the kernel's memory map.  These are:
/
/	fulpage		First User Low memory page: first low memory
/			page to be placed in kernel's memory map.  If
/			we are running Merge, the kernel will be in
/			extended memory, and fulpage will be 0.  If
/			we are not running Merge (e.g. because there
/			is not enough extended memory), fulpage will
/			be the first page beyond the low memory kernel.
/
/	lulpage		Last User Low memory page:  actually this is
/			the first unavailable low memory page, rather
/			than the last available page.  It will either
/			be the F000 paragraph or some page where the
/			Atron probe real-mode code is running.
/
/	fuepage		First User Extended memory page: first high
/			memory page to be placed in kernel's memory map.
/			If we are running Merge, this will be the
/			first page beyond the high memory kernel.  If not
/			this will be 1 Meg (if there is any extended
/			memory).
/
/ Set lulpage according to the NMI real mode vector.  Will normally be
/ either the probe or the BIOS.  Prevents probe real mode code from being
/ allocated to users.
/
	xor	%ax, %ax
	mov	%ax, %es		/ address real mode vectors
	mov	%es:0xa, %ax		/ fetch int 2 CS
	shr	$5, %ax			/ convert to click number
	mov	%ax, lulpage
#endif /* ATMERGE */

#ifdef ATPROBE
/
/ Set up IDT descriptors 1, 2, and 3 for the Atron probe, if it is
/ seemingly there.
/
	cmp	$0x780, %ax		/ click corresponding to f000
	je	noprobe
	inc	probethere		/ statically initialized to zero
	push	%ds
	mov	$idt, %di
	mov	2(%di), %ax		/ get low phys address of idt
	movb	4(%di), %cl		/ get high base (byte) of idt
	movb	%cl, %ch
	movb	%ah, %cl
	shl	$4, %cx
	xorb	%ah, %ah
	mov	%ax, %di
	add	$24, %di		/ point at int 3 descriptor
	mov	%cx, %ds
	mov	$3, %cx
nextdesc:
	mov	%cx, %bx
	shl	$2, %bx
	mov	%es:(%bx), %ax		/ fetch offset from vector
	mov	%ax, 0(%di)		/ store in IDT descriptor
	mov	$[PROBE_SEL << 3], 2(%di)
	movb	$0, 4(%di)
	movb	$0xe6, 5(%di)		/ level 3 interrupt gate
	mov	$0, 6(%di)
	sub	$8, %di
	loop	nextdesc
	pop	%ds
noprobe:
#endif /* ATPROBE */

#ifdef	IBMAT
/
/ Gate address bit 20. Must be done before entering hyperspace.
/
	call	gate_20			/ gate addr bit 20
#endif	IBMAT

#ifdef ATMERGE
/ Some BIOS's don't reinitialize the realtime clock and its interrrupt,
/ and we can't start-up if it isn't so.  Here is what the BIOS should
/ have done:
	movb	$0x0a, %al
	outb	0x70
	jmp	.+2
	movb	$0xaa, %al
	outb	0x71
	jmp	.+2
	movb	$0x0b, %al
	outb	0x70
	jmp	.+2
	movb	$0x02, %al
	outb	0x71
#endif ATMERGE
	

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
	mov	$[ KSTACKSZ \* 2 ],%sp		/ establish kernel stack

#ifdef ATPROBE
/  debug bkpt #1
/  must enable NMI in order for the probe to trap this
	xor	%ax, %ax
	outb	0x70
	jmp	.+2
	mov	$1, %ax
	outb	0x80
#endif /* ATPROBE */

#ifdef ATMERGE
/
/ Must move the kernel to extended memory in order to be able to load DOS
/
	.globl	inprot
inprot:					/ inprot is for debugging with the probe
	lcall	kreloc
#endif /* ATMERGE */

#ifdef ATPROBE
/  debug bkpt #2
	mov	$2, %ax
	outb	0x80
#endif /* ATPROBE */

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

#ifdef	IBMAT
/
/ gate_20: this routine controls a signal which gates address bit 20.
/		the gate A20 signal is an output of the 8042 slave
/		processor. address bit 20 should be gated on before 
/		entering protected mode. based strictly on the AT rom
/		bios code (p. 5-155).

gate_20:
	push	%ax			/ save ax
	push	%dx			/ save dx
	call	empty_8042		/ insure 8042 input buffer empty
	jnz	gate_a20_return		/ return if 8042 unable to accept cmd
	mov	$0x00D1,%ax		/ 8042 cmd to write output port
	mov	$0x0064,%dx		/ addr of 8042 port
	outb	(%dx)			/ output port data to 8042
	call	empty_8042		/ wait for 8042 to accept port data
	jnz	gate_a20_return		/ return if 8042 can't accept cmd
	mov	$0x00DF,%ax		/ addr bit 20 on
	mov	$0x0060,%dx		/ port data addr
	outb	(%dx)			/ output port data to 8042
/ 
/ 8042 output will switch within 20 usec of accepting port data
/
gate_a20_return:
	pop	%dx			/ restore dx
	pop	%ax			/ restore ax
	ret

/
/ empty_8042:	this routine waits for the 8042 input buffer to empty.
/	input:	none
/	output:	al = 0 - 8042 input buffer empty (zero flag set)
/		al = 2 - time out, 8042 input buffer full (non-zero flag set)
/
empty_8042:
	push	%dx			/ save dx
	push	%cx			/ save cx
	sub	%cx,%cx			/ cx = 0, used as timeout value
empty_loop:
	mov	$0x64,%dx		/ 8042 status port addr
	in	(%dx)			/ read 8042 status port
	andb	$0x02,%al		/ test input buf full flag (bit 1)
	loopnz	empty_loop		/ loop until input buf empty or timeout
	pop	%cx			/ restore cx
	pop	%dx			/ restore dx
	ret
#endif	IBMAT

#ifdef ATMERGE
/
/ To run ATMERGE, the kernel must be loaded in extended memory.  In this
/ routine, we take the just loaded kernel, move it to extended memory,
/ and relocate the gdt descriptors.
/
/ If we succeed, set	msm_ok to 1, fulpage to 0, and fuepage to the
/			first page beyond the extended memory kernel.
/
/ If we fail, set	msm_ok to 0, fulpage to first page beyond low memory
/			kernel, and fuepage to 0.
/
kreloc:
	/ set up for failure
	xor	%ax, %ax
	mov	%ax, msm_ok
	mov	%ax, fuepage
	mov	aetext, %ax		/ get low word of phys address of etext
	mov	aetext + 2, %dx		/ get high word
	add	$511, %ax		/ round up to next click boundary
	adc	$0, %dx
	mov	$9, %cx			/ prepare to convert to click address
rshloop:
	rcr	$1, %dx
	rcr	$1, %ax
	loop	rshloop
	mov	%ax, fulpage
	inc	%ax
	shr	$1, %ax
	mov	%ax, %dx		/ dx is K occupied by kernel

	/ determine how much extended memory there is from CMOS
	movb	$0x18, %al
	outb	0x70
	jmp	.+2
	inb	0x71
	movb	%al, %ah
	movb	$0x17, %al
	outb	0x70
	jmp	.+2
	inb	0x71   		/ ax is now extended memory size in K
	cmp	%dx, %ax	/ must be able to reloc kernel
	jge	krenough
	jmp	krelret
krenough:
	/ We will succeed.  Set msm_ok to 1, fulpage to 0, and fuepage
	/ to the first user page in extended memory, which is 1M beyond
	/ the current fulpage.
	mov	$1, msm_ok
	mov	fulpage, %ax
	add	$0x800, %ax		/ add clicks in 1 Meg
	mov	%ax, fuepage
	mov	$0, fulpage
	/ Now we will copy the kernel to extended memory.  DX contains
	/ the number of K to move.
	mov	$<s>gdtptr, %di
	mov	%di, %ds
	les	gdtptr, %di
	mov	$0xffff, %es:[COPY0SEL << 3] (%di)
	mov	$0xffff, %es:[COPY1SEL << 3] (%di)
	mov	$0, %es:[[COPY0SEL << 3] + 2] (%di)
	movb	$0, %es:[[COPY0SEL << 3] + 4] (%di)
	mov	$0, %es:[[COPY1SEL << 3] + 2] (%di)
	movb	$0x10, %es:[[COPY1SEL << 3] + 4] (%di)
	mov	$ACC_KDATA, %es:[[COPY0SEL << 3] + 5] (%di)
	mov	$ACC_KDATA, %es:[[COPY1SEL << 3] + 5] (%di)
copyloop:
	push	%ds
	push	%es
	push	%di
	push	%cx
	mov	$[COPY0SEL << 3], %ax
	mov	%ax, %ds
	mov	$[COPY1SEL << 3], %ax
	mov	%ax, %es
	xor	%di, %di
	xor	%si, %si
	cmp	$64, %dx		/ >= 64K still to move?
	jge	do64
	mov	%dx, %cx		/ no, move exactly DX K
	shl	$9, %cx			/ convert KB to word count
	xor	%dx, %dx
	jmp	moveit
do64:
	sub	$64, %dx
	mov	$0x8000, %cx		/ move 32K words (64K bytes)
moveit:
	rep
	smov
	pop	%cx
	pop	%di
	pop	%es
	pop	%ds
	incb    %es:[[COPY0SEL << 3] + 4] (%di)
	incb	%es:[[COPY1SEL << 3] + 4] (%di)
	cmp	$0, %dx
	jg	copyloop

	/ Ok, we've got a copy of the kernel in extended memory.  Now
	/ we have to relocate the gdt descriptors.  Set up a descriptor
	/ in COPY0SEL that accesses the extended memory gdt.
	/ Copy the descriptor currently selected by es, which is the
	/ gdt alias.
	mov	%es, %bx
	mov	%es:(%bx,%di), %ax
	mov	%ax, %es:[COPY0SEL << 3] (%di)
	mov	%es:2(%bx,%di), %ax
	mov	%ax, %es:[[COPY0SEL << 3] + 2] (%di)
	mov	%es:4(%bx,%di), %ax
	addb	$0x10, %al
	mov	%ax, %es:[[COPY0SEL << 3] + 4] (%di)
	mov	$[COPY0SEL << 3], %ax
	mov	%ax, %es
	/ es now addresses, in extended memory, what it used to address
	/ in low memory
	mov	$reloclist, %si
krloop:
	movb	(%si), %bl		/ get next selector index
	cmpb	$0, %bl
	je	krdone			/ exit loop when 0 index found
	xorb	%bh, %bh
	shl	$3, %bx			/ convert to gdt offset
	add	$0x10, %es:4(%bx)	/ add 1M to base of this descriptor
	inc	%si
	jmp	krloop
krdone:
	/ mark the TSS descriptor in the extended memory GDT as available
	mov	$[PSLOT_SEL << 3],%bx	/ get proc 0 tss desc selector
	mov	$0x81, %es:5(%bx)	/ available TSS state segment
/
/ We are now ready to abandon the low memory kernel and run the extended
/ memory kernel.  To do this we have to reload the GDT, IDT, TR, and LDT,
/ and all the segment registers.  The GDT and IDT descriptors are stored
/ in the extended memory GDT, so we must convert the current low memory
/ DS descriptor to high memory and reload it.
/
	les	gdtptr, %di
	mov	%ds, %bx
	addb	$0x10, %es:4(%bx,%di)	/ relocate to 1M
	push	%ds
	pop	%ds			/ push and pop to reload new DS

	lgdt	gdtalias
	lidt	idt
	str	%ax
	ltr	%ax
	sldt	%ax
	lldt	%ax
	/ reload the segment registers (cs will be reloaded on return)
	push	%ds
	pop	%ds
	push	%es
	pop	%es
	/ relocate the p0usel ldt descriptor.
	mov	$p0usel, %bx		/ address the proc0 u selector
	add	$0x10, 4(%bx)		/ add 1M to the base
	mov	%ss, %ax		/ ss is p0usel in the LDT
	mov	%ax, %ss
krelret:
	lret

#endif /* ATMERGE */

