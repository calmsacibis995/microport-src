	.file	"misc.s"
/
/ @(#)misc.s	1.21
/
#include	"../sys/8259.h"
#include	"../sys/clock.h"

	.text

/
/ clkstart
/	initialize and start the clock
/
/ clockon
/	just turn on the clock
/
/ clockoff
/	turn off the clock
/
	.globl	clkstart
	.globl	clockon
	.globl	clockoff

clkstart:
clockon:
	movb	$RTCMODE,%al		/ get mode word
	mov	$CTCMODE,%dx
	outb	(%dx)			/ kick it
	mov	$CNT0MODE,%dx		/ CTC counter 0 port
	mov	$RTCCNT,%ax		/ interval count
	outb	(%dx)			/ output lsb
	movb	%ah,%al			/ output ...
	outb	(%dx)			/ ... msb
	lcall	spl0			/ enable interrupts from PIC
	sti				/ INTERRUPTS ON
	lret

clockoff:
	movb	$RTCMODE,%al		/ get mode word
	mov	$CTCMODE,%dx		/ CTC counter 0 port
	outb	(%dx)			/ kick it - clock will hang
	lret

/
/ spl?()
/	send the PIC the appropriate masks to simulate
/	priority level
/
/	returns previous level
/
/	spl0()	enables all interrupts ( clock, 544 sio, 215 disk, console )
/	spl4()	disables console
/	spl5()	disables console and 544 sio controller interrupts
/	spl6()	disables cons, 544, and 215 disk interrupts
/	spl7()	disables everything
/	splhi()	disables everything
/
/	splx( oldlevel )	restores interrupt to previous state
/
	.data
curlevel:	.value	0	/ current spl level

	.text
	.globl	spl0
spl0:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$SPL0MASTER,%ax		/ get master PIC mask for this level
	mov	$PICMASTER,%dx		/ get address of master PIC
	outb	(%dx)			/ kick master
	mov	$SPL0SLAVE,%ax		/ get slave PIC mask for this level
	mov	$PICSLAVE,%dx		/ get address of slave PIC
	outb	(%dx)			/ kick slave
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$0,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl4
spl4:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$SPL4MASTER,%ax		/ get master PIC mask for this level
	mov	$PICMASTER,%dx		/ get address of master PIC
	outb	(%dx)			/ kick master
	mov	$SPL4SLAVE,%ax		/ get slave PIC mask for this level
	mov	$PICSLAVE,%dx		/ get address of slave PIC
	outb	(%dx)			/ kick slave
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$4,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl5
spl5:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$SPL5MASTER,%ax		/ get master PIC mask for this level
	mov	$PICMASTER,%dx		/ get address of master PIC
	outb	(%dx)			/ kick master
	mov	$SPL5SLAVE,%ax		/ get slave PIC mask for this level
	mov	$PICSLAVE,%dx		/ get address of slave PIC
	outb	(%dx)			/ kick slave
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$5,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl6
spl6:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$SPL6MASTER,%ax		/ get master PIC mask for this level
	mov	$PICMASTER,%dx		/ get address of master PIC
	outb	(%dx)			/ kick master
	mov	$SPL6SLAVE,%ax		/ get slave PIC mask for this level
	mov	$PICSLAVE,%dx		/ get address of slave PIC
	outb	(%dx)			/ kick slave
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$6,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	spl7
	.globl	splhi
spl7:
splhi:
	pushf				/ save flags
	cli				/ no interrupts while messing with PIC
	mov	$SPL7MASTER,%ax		/ get master PIC mask for this level
	mov	$PICMASTER,%dx		/ get address of master PIC
	outb	(%dx)			/ kick master
	mov	$SPL7SLAVE,%ax		/ get slave PIC mask for this level
	mov	$PICSLAVE,%dx		/ get address of slave PIC
	outb	(%dx)			/ kick slave
	mov	$<s>curlevel,%ax	/ load segment ...
	mov	%ax,%ds			/ ... of curlevel
	mov	curlevel,%ax		/ return old level
	mov	$7,curlevel		/ set current level
	popf				/ restore cli/sti state
	lret

	.globl	splx
splx:
	push	%bp			/ establish ...
	mov	%sp,%bp			/ ... stack frame
	push	%bx			/ save what I'm gonna use
	pushf				/ save flags
	mov	6(%bp),%bx		/ get spl level to go to
	mov	$<s>mastbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of master PIC table
	mov	$mastbl,%si		/ load offset of master PIC table
	movb	%es:(%bx,%si),%al	/ get master mask for this PIC level
	mov	$PICMASTER,%dx		/ get address of PIC master port
	cli				/ no interrupts while messing with PIC
	outb	(%dx)			/ set master PIC to this level
	mov	$<s>slavtbl,%ax		/ load segment ...
	mov	%ax,%es			/ ... of slave PIC table
	mov	$slavtbl,%si		/ load offset of slave PIC table
	movb	%es:(%bx,%si),%al	/ get slave mask for this PIC level
	mov	$PICSLAVE,%dx		/ get address of slave PIC port
	outb	(%dx)			/ set slave pic to this level
	mov	$<s>curlevel,%dx	/ load segment ...
	mov	%dx,%ds			/ ... of curlevel
	mov	%bx,curlevel		/ set current level
	popf				/ restore cli/sti state
	pop	%bx			/ restore other regs
	pop	%bp
	lret

/
/ in, out, inb, outb
/	input and output routines to be called from C.
/
/	word in( port )
/		int port;
/
/	char inb( port )
/		int port;
/
/	out( port, word )
/		int port;
/		int word;
/
/	outb( port, byte )
/		int port;
/		char byte;
/
	.globl	in
	.globl	inb
	.globl	out
	.globl	outb

in:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	in	(%dx)		/ get data into ax
	pop	%bp
	lret			/ return

inb:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	inb	(%dx)		/ get data in to ax
	movb	$0,%ah		/ zero the high byte
	pop	%bp
	lret			/ return

out:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	mov	8(%bp),%ax	/ get data
	out	(%dx)		/ put it out
	pop	%bp
	lret			/ return

outb:	push	%bp		/ save bp
	mov	%sp,%bp		/ get the base of parameters
	mov	6(%bp),%dx	/ get port number
	mov	8(%bp),%ax	/ get data
	outb	(%dx)		/ put it out
	pop	%bp
	lret			/ return

/
/ reloadregs
/	force the 286 to reload various selector registers
/	and all the invisible associated registers.
/	(i.e. limit, base, and access)
/	we have to be real tricky with the stack
/
	.globl	reloadregs
reloadregs:
	sldt	%ax		/ get ldt selector
	lldt	%ax		/ put it back, ( loads the invisible regs also )
	str	%ax		/ get tss selector
	ltr	%ax		/ put it back, ( loads the invisible regs also )
				/ and turns on the busy bit
	mov	%bp,%si		/ save frame pointer
	mov	%sp,%bp		/ point at return address
	mov	%ss,%dx		/ get stack selector
	mov	%dx,%es		/ load %es with the new invisible regs
	mov	0(%bp),%ax	/ move offset from old stack ...
	mov	%ax,%es:0(%bp)	/ ... to new stack
	mov	2(%bp),%ax	/ move selector from old stack ...
	mov	%ax,%es:2(%bp)	/ ... to new stack
	mov	%dx,%ss		/ put stack selector back (load invisible regs)
	mov	%si,%bp		/ restore frame pointer
	lret

/
/ reloadptr() reloads the task register
/ with the parent process' tss gate selector
/
	.globl	reloadptr
reloadptr:
	push	%bp
	mov	%sp,%bp
	mov	6(%bp),%ax
	ltr	%ax
	pop	%bp
	lret

