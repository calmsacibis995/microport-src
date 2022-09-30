	.file	"trap.s"
/
/ @(#)trap.s	1.27
/

#include	"../sys/param.h"
#include	"../sys/mmu.h"
#include	"../sys/trap.h"

	.text
/
/ 	All I/O interrupts enter at intr: after pushing their vector.
/	Double exception, Invalid TSS, Segment not present, Stack,
/	and General protection faults enter at fault:, after pushing
/	an error code onto the stack and their vector.
/	All system calls enter at systemc: after pushing their arguments.
/
/	Because all interrupts have an Interrupt Gate in the
/	IDT, all interrupts are disabled on entry at intr:, and fault:.
/	MMU faults, however, are never disabled.
/
/	If we were in user mode when the interrupt happened,
/	we are now on our kernel stack because the ss:sp
/	for CPL 0 is in the processes TSS, and ss:sp is
/	loaded in the event of a privilege level transition,
/	and the stack looks like one of these:
/
/	    I/O Interrupt		       Fault
/	+--------------------+	 	+--------------------+
/	|      old %ss       |	 	|      old %ss       |
/	+--------------------+	 	+--------------------+
/	|      old %sp       |	 	|      old %sp       |
/	+--------------------+	 	+--------------------+
/	|     old flags      |	 	|     old flags      |
/	+--------------------+	 	+--------------------+
/	|      old %cs       |	 	|      old %cs       |
/	+--------------------+	 	+--------------------+
/	|      old %ip       |	 	|      old %ip       |
/	+--------------------+	 	+--------------------+
/	|  interrupt vector  |	 	|     error code     |
/	+--------------------+	 	+--------------------+
/	                      	 	|  interrupt vector  |
/	                      	 	+--------------------+
/
/	If we were in system mode when the interrupt occurred,
/	we are still on our kernel stack, because no privilege
/	level transition happened. At this point,
/	the stack looks like one of these:

/	    I/O Interrupt		       Fault
/	+--------------------+	 	+--------------------+
/	|     old flags      |	 	|     old flags      |
/	+--------------------+	 	+--------------------+
/	|      old %cs       |	 	|      old %cs       |
/	+--------------------+	 	+--------------------+
/	|      old %ip       |	 	|      old %ip       |
/	+--------------------+	 	+--------------------+
/	|  interrupt vector  |	 	|     error code     |
/	+--------------------+	 	+--------------------+
/	                      	 	|  interrupt vector  |
/	                      	 	+--------------------+
/
/	The stack will be slightly different if we got here as
/	a result of a system call. If this was the case, the
/	stack will look like:
/	+--------------------+
/	|      old %ss       |
/	+--------------------+
/	|      old %sp       |
/	+--------------------+
/	|     argument 9     |
/	+--------------------+
/	|     argument 8     |
/	+--------------------+
/	|     argument 7     |
/	+--------------------+
/	|     argument 6     |
/	+--------------------+
/	|     argument 5     |
/	+--------------------+
/	|     argument 4     |
/	+--------------------+
/	|     argument 3     |
/	+--------------------+
/	|     argument 2     |
/	+--------------------+
/	|     argument 1     |
/	+--------------------+
/	|     argument 0     |
/	+--------------------+
/	|       old %cs      |
/	+--------------------+
/	|       old %ip      |
/	+--------------------+
/	
/
/		 After the interrupt or system call does the
/		 its stack manipulation and saving of registers,
/	         the stack will look like one of the following,
/		 and bp will point to the base of the frame.
/
/             Interrupt	 	             System call
/	+--------------------+	 	+--------------------+
/	|      old %ss       |    46 	|      old %ss       |
/	|If intr in user mode|	 	|                    |
/	+--------------------+	 	+--------------------+
/	|      old %sp       |    44 	|      old %sp       |
/	|If intr in user mode|	 	|                    |
/	+--------------------+	 	+--------------------+
/	|     old flags      |    42 	|     argument 9     |
/	|                    |          |overwritten by flags|
/	+--------------------+	 	+--------------------+
/	|      old %cs       |    40 	|     argument 8     |
/	+--------------------+	 	+--------------------+
/	|      old %ip       |    38 	|     argument 7     |
/	+--------------------+	 	+--------------------+
/	|  interrupt vector  |    36 	|     argument 6     |
/	|  (or error code)   |          |                    |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    34 	|     argument 5     |
/       |or vect if err code |          |                    |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    32 	|     argument 4     |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    30 	|     argument 3     |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    28 	|     argument 2     |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    26 	|     argument 1     |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    24 	|     argument 0     |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    22 	|      old %cs       |
/	+--------------------+	 	+--------------------+
/	|      Padding       |    20 	|      old %ip       |
/	+--------------------+	 	+--------------------+
/	|      old %ax       |    18 	|      old %ax       |
/	+--------------------+	 	+--------------------+
/	|      old %cx       |    16 	|      old %cx       |
/	+--------------------+	 	+--------------------+
/	|      old %dx       |    14 	|      old %dx       |
/	+--------------------+	 	+--------------------+
/	|      old %bx       |    12 	|      old %bx       |
/	+--------------------+	 	+--------------------+
/	|%sp before the pusha|    10 	|%sp before the pusha|
/	+--------------------+	 	+--------------------+
/	|      old %bp       |    8 	|      old %bp       |
/	+--------------------+	 	+--------------------+
/	|      old %si       |    6 	|      old %si       |
/	+--------------------+	 	+--------------------+
/	|      old %di       |    4 	|      old %di       |
/	+--------------------+	 	+--------------------+
/	|      old %ds       |    2 	|      old %ds       |
/	+--------------------+	 	+--------------------+
/	|      old %es       |    0 	|      old %es       |
/	+--------------------+	 	+--------------------+
/

/
/ 	I/O Interrupts enter here
/
	.globl	intr
intr:
	sub	$16,%sp			/ pad interrupt frame to make it look
					/ like system call frame
	pusha				/ save all the regular registers
	push	%ds			/ save the segment registers
	push	%es
	mov	%sp,%bp			/ set bp to base of frame
/ 
/	set up params for trap(), called after calling I/O 
/	interrupt routine
/
	push	40(%bp)			/ old cs to check for usr or system mode
	push	$0			/ "code"
	push	%ss			/ selector of base of stack frame
	push	%bp			/ offset of base of stack frame
	push	36(%bp)			/ vector number
	cmp	$CLKVEC,36(%bp)		/ clock is handled differently
	jz	clkint			/ it is the clock, so go do it
	jmp	ioint			/ handle all the other I/O interrupts

/
/	Faults and traps enter here. Stack must be coerced
/	to look like the other stacks. In other words,
/	we have to pad less to keep the offsets the same.
/
	.globl	fault
fault:
	sub	$14,%sp			/ pad interrupt frame to make it look
					/ like system call frame
	pusha				/ save all the regular registers
	push	%ds			/ save the segment registers
	push	%es
	mov	%sp,%bp			/ set bp to base of frame
/ 
/	set up params for trap()
/
	push	40(%bp)			/ old cs to check for usr or system mode
	push	36(%bp)			/ Error code
	push	%ss			/ selector of base of stack frame
	push	%bp			/ offset of base of stack frame
	push	34(%bp)			/ vector number
	jmp	common			/ call trap

/
/	Traps that enter here do not have an error code 
/	on stack, so we	need more padding
/
	.globl	trp
trp:
	sub	$16,%sp			/ pad interrupt frame to make it look
					/ like system call frame
	pusha				/ save all the regular registers
	push	%ds			/ save the segment registers
	push	%es
	mov	%sp,%bp			/ set bp to base of frame
/ 
/	set up params for trap()
/
	push	40(%bp)			/ old cs to check for usr or system mode
	push	$0			/ "code"
	push	%ss			/ selector of base of stack frame
	push	%bp			/ offset of base of stack frame
	push	36(%bp)			/ vector number
	jmp	common			/ call trap

/
/	All system calls enter here
/
	.globl	systemc
systemc:
	pusha				/ save all the regular registers
	push	%ds			/ save the segment registers
	push	%es
	mov	%sp,%bp			/ set bp to base of frame
/
/	save flags for syscall return
/
	pushf
	pop	42(%bp)			/ goes into arg9 placeholder on stack
/ 
/	set up params for trap()
/
	push	22(%bp)			/ old cs to check for usr or system mode
	push	$0			/ "code"
	push	%ss			/ selector of base of stack frame
	push	%bp			/ offset of base of stack frame
	push	$SYSCALL		/ identify a system call to trap()
/
/	Before trap() return from a system call, the syscall code
/	moves cs to arg8, ip to arg7. This makes the iret
/	work for all cases.
/

common:
	cld				/ insure forward mode for kernel
	sti				/ turn interrupts on
	lcall	<s>trap,trap		/ trap() handles all traps and syscalls
/
/	On return from trap, compress the stack to remove the
/	arguments to trap, pop off all the saved registers, and
/	remove the padding or system call arguments. Before
/	loading ( executing pop ) any segment registers, validate
/ 	cs:ip, and es, and ds. We must verify that the value that
/	we are going to load into the segment register is mapped in.
/	Otherwise, we could panic the kernel loading an illegal value
/	into a segment register. If we find that a value is illegal,
/	we simply load that register with a zero ( if the register is
/	ds or es ). That way, the kernel will not panic, but the first
/	time the user attempts to use the register ( without setting it )
/	the user will fault himself. Cs:ip / ss:sp are validated by calling
/	validate(). If the users cs:ip or ss:sp is not valid, and the
/	user is not attempting to catch SIGSEGV, validate will not
/	return, because we will send the process a SIGSEGV. If validate()
/	returns with a zero value, cs:ip and ss:sp is ok. Otherwise,
/	validate will return with a 1, which means that the users addresses
/	are bad, but the user is trying to catch the SIGSEGV. In this case,
/	before iret, validate is called again.
/
int_ret:
	add	$10,%sp			/ get rid of args to trap()
	mov	%sp,%bp			/ to get at segment registers
/
/	to see if we are going back to user mode, simulate a
/	subraction of the interrupt frame from the stack, and see
/	if we are at the beginning of the stack. If we are, we must
/	be going back to user mode
/
	mov	%bp,%ax
	add	$48,%ax			/ size of interrupt frame
	cmp	$[KSTACKSZ\*2],%ax	/ beginning of stack
	jne	int_ret0
again:
	push	%ss			/ give validate() base of regs
	push	%bp
	lcall	validate		/ validate cs:ip and possibly ss:sp
	add	$4,%sp
	or	%ax,%ax			/ check validate return
	jnz	again			/ non-zero, have to validate again
	mov	%sp,%bp			/ to get at segment registers
int_ret0:
	verr	0(%bp)			/ verify es
	jz	esgood			/ accessible?
	mov	$0,0(%bp)		/ no, zero es
esgood:
	verr	2(%bp)			/ verify ds
	jz	int_ret1		/ accessible?
	mov	$0,2(%bp)		/ no, zero ds
int_ret1:
	pop	%es			/ pop off (possibly modified) seg regs
	pop	%ds
	popa				/ restore general regs
	add	$18,%sp			/ remove padding or syscall args
	iret				/ get out of here

/
/	Final cleanup for return to interrupted user after calling
/	a user interrupt handler comes here.  See sigcode(os/sigcode.c)
/	for first part of this cleanup.  _sigcode verifies that es and ds
/	are NULL or valid segment descriptors.  If not it returns to
/	sigcode to drop core and exit.  If es and ds are reasonable,
/	restore them, restore the user's general registers and iret to
/	restore flags, ss:sp, and return to the interrupted user.
/
	.globl	_sigcode
_sigcode:
	push	%bp			/ save bp in case es or ds are invalid
					/	and we have to return
	mov	%sp,%bp
	mov	6(%bp),%bx		/ verify that es and ds are reasonable
	test	%ss:(%bx)
	jz	esok			/ OK if es is zero
	verr	%ss:(%bx)
	jz	esok			/ OK if es is valid for reading
badesords:				/ es or ds is invalid
	pop	%bp			/ restore stack frame
	lret				/ return to sigcode
esok:
	test	%ss:2(%bx)		/ check ds
	jz	dsok			/ OK if ds is zero
	verr	%ss:2(%bx)
	jnz	badesords		/ bad if ds is not valid for reading
dsok:
	mov	%bx,%sp			/ reset sp to address in stack of es
	pop	%es			/ restore es and ds
	pop	%ds
	popa				/ restore di, si, bp, bx, dx, cx, and ax
	iret				/ restore cs, ip, ss, sp, and flags

/
/	An I/O interrupt comes here. This code decides which
/	interrupt handler to call, calls it, and on return,
/	checks for rescheduling opportunities
/
ioint:
	mov	36(%bp),%bx		/ get interrupt vector
	mov	$<s>handlers,%ax	/ get selector base of handler table
	mov	%ax,%es			/ put it where we can use it
	mov	$handlers,%di		/ get offset base of handler table
	push	%bx			/ pass vector to handler
	shl	$2,%bx			/ because addresses are four bytes
	cld				/ make sure forward
	sti				/ interrupts back on
	lcall	*%es:(%bx,%di)		/ call the handler
	add	$2,%sp			/ clean up argument to handler
	mov	40(%bp),%ax		/ get cs of interrupted process
	and	$SEL_RPL,%ax		/ going back to user mode?
	jz	ioint1			/ no, just return
	mov	$<s>runrun,%ax		/ yes, load segment ...
	mov	%ax,%ds			/ ... of runrun
	mov	runrun,%ax		/ get reschedule flag
	or	%ax,%ax			/ should we reschedule?
	jz	ioint1			/ no, return
	mov	$RESCHED,-10(%bp)	/ yes, tell trap that we want to resched
	lcall	<s>trap,trap		/ all the args are already on the stack
ioint1:
	jmp	int_ret			/ common exit

/
/ clkint
/	A clock interrupt comes here. It is special, because
/	we have to pass it more information than an ordinary
/	I/O interrupt
/
clkint:
	push	40(%bp)			/ give old cs:ip to clock ...
	push	38(%bp)			/ ... as a pointer
	push	40(%bp)			/ give just cs to clock for sys/usermode
	cld				/ make sure forward
	sti				/ interrupts back on
	lcall	<s>clock,clock		/ call the clock handler
	add	$6,%sp			/ clean up clock args
	mov	40(%bp),%ax		/ get cs of interrupted process
	and	$SEL_RPL,%ax		/ going back to user mode?
	jz	clkint1			/ no, just return
	mov	$<s>runrun,%ax		/ yes, load segment ...
	mov	%ax,%ds			/ ... of runrun
	mov	runrun,%ax		/ get reschedule flag
	or	%ax,%ax			/ should we reschedule?
	jz	clkint1			/ no, return
	mov	$RESCHED,-10(%bp)	/ yes, tell trap that we want to resched
	lcall	<s>trap,trap		/ all the args are already on the stack
clkint1:
	jmp	int_ret			/ common exit

/
/ idle
/	wait around for a interrupt ( if the kernel didn't
/	have anything better to do... )
/
	.globl	idle
	.globl	waitloc
idle:
	lcall	spl0
	hlt
waitloc:
	lret
