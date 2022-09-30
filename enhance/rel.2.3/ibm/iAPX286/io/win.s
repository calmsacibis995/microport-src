	.file	"win.s"
/ *uportid = "@(#)win.s	Microport Rev Id  2.3 4/21/87";
/
#include	"sys/param.h"
#include	"sys/mmu.h"
/
#define	NBPWS	0x0200
#define	NBPFS	0x0400
#define WD0_DATA 0x01F0
/
	.text
	.globl	hdrdma
/
hdrdma:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	6(%bp),%ax		/ get from_physaddr
	mov	8(%bp),%bx
	mov	$NBPWS,%cx		/ click size
	mov	$<s>gdtptr,%di		/ load segment ...
	mov	%di,%ds			/ ... of GDT pointers
	les	gdtptr,%di		/ point to gdt
	mov	$NBPWS,%es:[WNDMASEL<<3](%di)	/ load limit
	mov	%ax,%es:[[WNDMASEL<<3]+2](%di)	/ load lo_phys_address
	movb	%bl,%es:[[WNDMASEL<<3]+4](%di)	/ load hi_phys_address
	mov	$ACC_KDATA,%es:[[WNDMASEL<<3]+5](%di)	/ load access byte
/
/ these ptr's are in the same segment as gdtptr above so don't reload %ds
/
	mov	$WD0_DATA,%dx		/ need an io address for buffer
	les	wndmaptr,%di		/ point to dest selector
	shr	$1,%cx			/ make a word count
	rep				/ do the copy
	ins
	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret
/
/
	.globl	hdwdma
/
hdwdma:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	6(%bp),%ax		/ get from_click_addr
	mov	8(%bp),%bx
	mov	$NBPWS,%cx		/ click size
	mov	$<s>gdtptr,%di		/ load segment ...
	mov	%di,%ds			/ ... of GDT pointers
	les	gdtptr,%di		/ point to gdt
	mov	$NBPWS,%es:[WNDMASEL<<3](%di)	/ load limit
	mov	%ax,%es:[[WNDMASEL<<3]+2](%di)	/ load lo_phys_address
	movb	%bl,%es:[[WNDMASEL<<3]+4](%di)	/ load hi_phys_address
	mov	$ACC_KDATA,%es:[[WNDMASEL<<3]+5](%di)	/ load access byte
/
/ these ptr's are in the same segment as gdtptr above so don't reload %ds
/
	mov	$WD0_DATA,%dx		/ need an io address for buffer
	lds	wndmaptr,%si		/ point to dest selector
	shr	$1,%cx			/ make a word count
	rep				/ do the copy
	outs
	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret
/
/
/
	.globl	hdfdma
/
/
hdfdma:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	6(%bp),%ax		/ get from_click_addr
	mov	8(%bp),%bx
	mov	$NBPWS,%cx		/ click size
	mov	$WD0_DATA,%dx		/ need an io address for buffer
	shr	$1,%cx			/ make a word count
fmt_data:
	outs
	loop	fmt_data
	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret
/
/
	.globl	hdrpeb
/
hdrpeb:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	6(%bp),%ax		/ get from_physaddr
	mov	8(%bp),%bx
	mov	$NBPWS,%cx		/ click size
	mov	$<s>gdtptr,%di		/ load segment ...
	mov	%di,%ds			/ ... of GDT pointers
	les	gdtptr,%di		/ point to gdt
	mov	$NBPWS,%es:[WNDMASEL<<3](%di)	/ load limit
	mov	%ax,%es:[[WNDMASEL<<3]+2](%di)	/ load lo_phys_address
	movb	%bl,%es:[[WNDMASEL<<3]+4](%di)	/ load hi_phys_address
	mov	$ACC_KDATA,%es:[[WNDMASEL<<3]+5](%di)	/ load access byte
/
/ these ptr's are in the same segment as gdtptr above so don't reload %ds
/
	mov	$WD0_DATA,%dx		/ need an io address for buffer
	les	wndmaptr,%di		/ point to dest selector
	mov	0x00DF,%cx
	rep
	in	(%dx)
	mov	0x0021,%cx
	rep				/ do the copy
	ins
	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret
/
	.globl	hdrpbb
/
hdrpbb:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	6(%bp),%ax		/ get from_physaddr
	mov	8(%bp),%bx
	mov	$NBPWS,%cx		/ click size
	mov	$<s>gdtptr,%di		/ load segment ...
	mov	%di,%ds			/ ... of GDT pointers
	les	gdtptr,%di		/ point to gdt
	mov	$NBPWS,%es:[WNDMASEL<<3](%di)	/ load limit
	mov	%ax,%es:[[WNDMASEL<<3]+2](%di)	/ load lo_phys_address
	movb	%bl,%es:[[WNDMASEL<<3]+4](%di)	/ load hi_phys_address
	mov	$ACC_KDATA,%es:[[WNDMASEL<<3]+5](%di)	/ load access byte
/
/ these ptr's are in the same segment as gdtptr above so don't reload %ds
/
	mov	$WD0_DATA,%dx		/ need an io address for buffer
	les	wndmaptr,%di		/ point to dest selector
	mov	10(%bp),%cx
	shr	$1,%cx
	rep				/ do the copy
	ins
	mov	0x0200,%cx
	mov	10(%bp),%ax
	sub	%ax,%cx
	shr	$1,%cx
	rep
	in	(%dx)
	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret

	.globl	flpyrdma	
flpyrdma:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	8(%bp),%ax		/ get from_physaddr
	mov	%ax,%ds			/ set source segment
	mov	6(%bp),%si
	mov	$NBPFS,%cx		/ click size
	mov	12(%bp),%ax
	mov	%ax,%es
	mov	10(%bp),%di
	rep				/ do the copy
	smovb
	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret
/

	.globl	flpywdma	
flpywdma:
	push	%bp			/ establish...
	mov	%sp,%bp			/ ...stack frame
	push	%cx			/ save what we are gonna trash
	push	%di
	mov	8(%bp),%ax		/ get from_physaddr
	mov	%ax,%es			/ set source segment
	mov	6(%bp),%di
	mov	$NBPFS,%cx		/ click size
	mov	12(%bp),%ax
	mov	%ax,%ds
	mov	10(%bp),%si
	rep				/ do the copy
	smovb
	pop	%di			/ restore what we trashed
	pop	%cx
	pop	%bp
	lret
/
