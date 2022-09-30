	.file	"siint.s"
/ Copyright 1985 by Microport. All Rights Reserved.
/ *uportid = "@(#)siint.s	Microport Rev Id  1.3.5 6/26/86";
/
/ @(#)siint.s	1.00
/
/ Special assembler handler for sio driver, for speed
/ Channels 0 and 1

	.globl	sd_array
	.text
/
/ 	SIO Channel 0 interrupts enter here
/
	.globl	siointr0
siointr0:
	cli
/ save the segment registers
/ call the handler with the parameters:
/ lcall sifast(unit #, sd_array[0])
	jmp	si_ret		/ jump to common sio return code

/
/ 	SIO Channel 1 interrupts enter here
/
	.globl	siointr1
siointr1:
/ lcall sifast(unit #, sd_array[1])

/ Common SIO return code
si_ret:		
	cli				/ not ints while eoi()
/ restore spl level
/ if (es: isn't good)
/ 	set zero es:
si_esgood:
/ if (ds: isn't good)
/	set zero ds:
si_dsgood:
	pop	%es			/ pop off (possibly modified) seg regs
	pop	%ds
	popa				/ restore general regs
	iret				/ get out of here
