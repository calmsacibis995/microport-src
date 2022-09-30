	.file	"qicsubs.s"
/ uportid = "@(#)qicsubs.s	Microport Rev Id  1.3.8 11/24/86";
/
/ @(#)qicsubs.s	1.21
/
/  Copyright 1987 by Microport. All Rights Reserved.
/ 
/  TVI TeleCAT-286 Streaming Tape Device Driver 
/  References:
/ 	TVI Streaming Tape Interface Hardware Specifications
/ 
/  Initial Coding:
/ 		unix-net!doug
/ 		unix-net!mark Mon Nov 10 22:19:51 PST 1986
/  Modification History:
/  To do:
/ 	1) test separation of generic (ct) and specific (qic) parts
/ 	   by adapting driver to another tape controller.
/ 

#define NOP .byte 0x90
#include "qic2tvi.h"

	.data
	.text
	.globl	resetape
resetape:
	cli				/ no interrupts while messing with PIC
	mov	$WR_CNTL,%dx		/ get address of write control port
	movb	$QC_ONLINE|QC_RST|QC_IRQ,%al
	outb	(%dx)			
	mov	$0x2000,%cx
resetloop:
	sub	$1,%cx
	NOP
	jne	resetloop
	mov	$WR_CNTL,%dx		/ get address of write control port
	movb	$QC_ONLINE|QC_IRQ,%al
	outb	(%dx)
	mov	$0,%cx
	mov	$RD_DR_STATUS,%dx	/ get address of drive status/ack port
okloop:
	inb	(%dx)
	testb	$QC_OK,%al
	je	resetok
	loop	okloop
	mov	$0,%ax
	sti
	lret
resetok:
	outb	(%dx)
	mov	$WR_COMREQ,%dx		/ get address of command port
	movb	$CTRSTU,%al
	outb	(%dx)
	mov	$1,%ax
	sti
	lret


	.globl waitRDY
waitRDY:
	mov	$0,%cx
	mov	$RD_DR_STATUS,%dx	/ get address of drive status/ack port
wrloop:
	inb	(%dx)
	andb	$0xf0,%al
	cmpb	$QC_OK|QC_ACK,%al
	je	waitok
	loop	wrloop
	mov	$1,%ax			/ return FAIL
	lret
waitok:
	mov	$0,%ax
	lret


	.globl delay50
delay50:
	push 	%cx
	mov	$0x20,%cx
delayloop:
	sub	$1,%cx
	NOP
	jne	delayloop
	mov	$INTACKTAPE,%dx	/ get address of drive status/ack port
	outb	(%dx)
	pop 	%cx
	ret

	.globl qc_rnstb
qc_rnstb:
	mov	$WR_COMREQ,%dx
	inb	(%dx)
	movb	%al,%ah
	mov	$WR_CNTL,%dx
	movb	$QC_ONLINE|QC_RDSTAT|QC_IRQ,%al
	outb	(%dx)
	call	delay50
	mov	$WR_CNTL,%dx
	movb	$QC_ONLINE|QC_IRQ,%al
	outb	(%dx)
	movb	%ah,%al
	movb	$0,%ah
      	lret 

	.data
	.text
