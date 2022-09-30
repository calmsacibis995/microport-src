	.file	"copy.s"
/ uportid = "@(#)copy.s	Microport Rev Id  1.3.3 6/18/86";
/
/ @(#)copy.s	1.13
/	Various copying routines
/

#include	"sys/param.h"
#include	"sys/mmu.h"

#define	READ	1
#define	WRITE	0

	.text

/
/ copyseg
/	copy a segment from one place to another
/
/	copyseg( from_click_addr, to_click_addr )
/
/	basic strategy is to turn click addresses into physical addresses,
/	point two scratch gdt segment descriptors to them, and do the
/	copy.
/
	.globl	copyseg
copyseg:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	6(%bp),%ax		/ get from_click_addr
	mov	$NBPC,%cx		/ click size
	mul	%cx			/ %dx = hi_phys_addr, %ax = lo_phys_addr
	mov	$<s>gdtptr,%di		/ load segment ...
	mov	%di,%ds			/ ... of GDT pointers
	les	gdtptr,%di		/ point to gdt
	mov	$NBPC,%es:[COPY0SEL<<3](%di)	/ load limit
	mov	%ax,%es:[[COPY0SEL<<3]+2](%di)	/ load lo_phys_address
	movb	%dl,%es:[[COPY0SEL<<3]+4](%di)	/ load hi_phys_address
	mov	$ACC_KDATA,%es:[[COPY0SEL<<3]+5](%di)	/ load access byte

	mov	8(%bp),%ax		/ get to_click_addr
	mul	%cx			/ %dx = hi_phys_addr, %ax = lo_phys_addr
	mov	$NBPC,%es:[COPY1SEL<<3](%di)	/ load limit
	mov	%ax,%es:[[COPY1SEL<<3]+2](%di)	/ load lo_phys_address
	movb	%dl,%es:[[COPY1SEL<<3]+4](%di)	/ load hi_phys_address
	mov	$ACC_KDATA,%es:[[COPY1SEL<<3]+5](%di)	/ load access byte

/
/ these ptr's are in the same segment as gdtptr above so don't reload %ds
/
	les	copy1ptr,%di		/ point to dest selector
	lds	copy0ptr,%si		/ point to source selector

	shr	$1,%cx			/ make a word count
	rep				/ do the copy
	smov

	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret

/
/ clearseg
/	zero out a segment
/
/	clearseg( click_addr )
/
	.globl	clearseg
clearseg:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	lcall	spl7			/ no interrupts
	push	%ax			/ save previous level
	mov	6(%bp),%ax		/ get click_addr
	mov	$NBPC,%cx		/ click size
	mul	%cx			/ %dx = hi_phys_addr, %ax = lo_phys_addr
	mov	$<s>gdtptr,%di		/ load segment ...
	mov	%di,%ds			/ ... of GDT pointers
	les	gdtptr,%di		/ point to gdt
	mov	$NBPC,%es:[CLSEGSEL<<3](%di)	/ load limit
	mov	%ax,%es:[[CLSEGSEL<<3]+2](%di)	/ load lo_phys_address
	movb	%dl,%es:[[CLSEGSEL<<3]+4](%di)	/ load hi_phys_address
	mov	$ACC_KDATA,%es:[[CLSEGSEL<<3]+5](%di)	/ load access byte
/
/ this ptr is in the same segment as gdtptr above so don't reload %ds
/
	les	clsegptr,%di		/ point to newly made descriptor
	xor	%ax,%ax			/ make a zero
	shr	$1,%cx			/ bytes to words
	rep				/ zero the click
	ssto
	lcall	splx			/ previous level is still on stack
	add	$2,%sp
	pop	%di			/ restore regs
	pop	%cx
	pop	%bp
	lret

/
/	NOTE:
/		Copyin and copyout are implemented differently than
/		most UNIX System ports. Both of these routines call
/		useracc to fully validate the user supplied address.
/		Other ports do much less validation, and rely on
/		some implementation of nofault to catch an illegal
/		access. If nofault is implemented here, simply take
/		out the calls to useracc.
/
/ copyin
/	copy stuff into the kernel from user space
/
/	copyin( useraddr, kerneladdr, count )
/
	.globl	copyin
copyin:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%di			/ save regs
	push	%cx
/
/	check if the user can access the given address
/
	push	$READ			/ read access
	push	14(%bp)			/ count
	push	8(%bp)			/ selector
	push	6(%bp)			/ offset
	lcall	<s>useracc,useracc	/ check access
	add	$8,%sp			/ fix up stack
	or	%ax,%ax			/ ok to access?
	jz	nogood			/ no, error and exit
	push	%ds			/ save this now
	mov	8(%bp),%ds		/ FINALLY, load the source selector
	mov	6(%bp),%si		/ and src offset
	mov	12(%bp),%es		/ load the destination selector
	mov	10(%bp),%di		/ and the destination offset
	mov	14(%bp),%cx		/ load the count
	call	copy			/ move the data
	xor	%ax,%ax			/ clear return value
	pop	%ds
	pop	%cx
	pop	%di
	pop	%bp
	lret

nogood:
	mov	$-1,%ax			/ error return value
	pop	%cx
	pop	%di
	pop	%bp
	lret

/
/ copyout
/	copy stuff from the kernel into user space
/
/	copyout( kerneladdr, useraddr, count )
/
	.globl	copyout
copyout:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%di			/ save regs
	push	%cx
/
/	check if the user can access the given address
/
	push	$WRITE			/ read access
	push	14(%bp)			/ count
	push	12(%bp)			/ selector
	push	10(%bp)			/ offset
	lcall	<s>useracc,useracc	/ check access
	add	$8,%sp			/ fix up stack
	or	%ax,%ax			/ ok to access?
	jz	nogood			/ no, error and exit
	push	%ds			/ save this now
	mov	12(%bp),%es		/ FINALLY, load the destination selector
	mov	10(%bp),%di		/ and the destination offset
	mov	8(%bp),%ds		/ load the source selector
	mov	6(%bp),%si		/ and the source offset
	mov	14(%bp),%cx		/ load the count
	call	copy			/ move the data
	xor	%ax,%ax			/ clear return value
	pop	%ds
	pop	%cx
	pop	%di
	pop	%bp
	lret

/
/ copy
/	copy data from one place to another
/
/ expects:
/	%ds = source selector
/	%si = source offset
/	%es = destination selector
/	%di = destination offset
/	%cx = byte count
copy:
	mov	%cx,%ax			/ check to see if count,
	or	%si,%ax			/            src offset,
	or	%di,%ax			/           dest offset,
	test	$1,%ax			/ are even.
	jz	wmv			/ yes, do word move
bmv:
	rep				/ no, do byte move
	smovb
	jmp	finish			/ clean up and return
wmv:
	shr	$1,%cx			/ make word count
	rep
	smov
finish:
	ret
