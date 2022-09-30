	.file	"asyfast.s"
/* @(#)Microport Rev ID: asyfast.s	1.3.3 6/18/86 */
	/ Originally, sio.c compiled to assembler. 
	/ Collapse code out of siointr, sioxintr, and eventually siorintr.
	.data
	.globl	sd_array
/	.globl	asydebug
#ifdef	LCCSIO
	.globl	_lccsio
_lccsio:	.byte	0
#endif	LCCSIO

	.text
#ifdef	LCCSIO
	.globl	lccsio0
	.globl	lccsio1
lccsio1:
	lcall	_asyintr1	/* Handle serial port 1 if SIO interrupt  */
	jmp	lccsiocomm
lccsio0:
	lcall	_asyintr0	/* Handle serial port 0 if SIO interrupt */
lccsiocomm:
	push	%bp
	push	%ax
	mov	%ds, %bp
	mov	$<s>_lccsio, %ax
	mov	%ax, %ds
	cmp	$0, _lccsio	/* Check if there was valid SIO interrupt */
	mov	%bp, %ds	/* CAREFUL!! Don't mess up flags in here! */
	pop	%ax		/* ^---- !!!! */
	pop	%bp		/* ^---- !!!! */
	jnz	lccsioret	/* Was serial IO, already processed, return */
	push	%bp
	mov	%sp, %bp	/* Setup to continue msm_decode... */
	push	%ds
	push	%ax
	ljmp	lccdecode	/* Was no serial IO, go decode interrupt */
lccsioret:
	add	$2, %sp		/* Remove vector */
	iret
#endif	LCCSIO	

	.def	asyfast; .val asyfast; .scl 2; .type 044; .endef
	.globl	asyfast
/ cx = unit number on entry, ds:si = &sd_array[unit]
/ we can trash every register, and we will
asyfast:
	cli
#ifdef	LCCSIO
	mov	%ds, %bp
	mov	$<s>_lccsio, %ax
	mov	%ax, %ds
	mov	$0, _lccsio	/* Clear SIO interrupt occurence flag */
	mov	%bp, %ds
#endif	/* LCCSIO */
	mov	%ds:4(%si),%ax	/* ax = sd_array[unit].sd_base */
  	add $2,%ax
	mov	%ax,%bp		/* save stat port, bp is preserved by C funcs */
	mov	%ds:22(%si),%cx /* cx = unit number */
sf_getreason:
	mov	%bp,%dx			/* get stat port */
  	inb	(%dx)		/* get reason: inb(baseport + 2) */
  	and	$7,%ax		/* zero hi byte sneakily, it is baseport */
  	cmp	$1,%ax
  	je	sf_noreason	/* no reason for interrupt (end of string) */
	les	%ds:18(%si),%di		/* es:di = common data for int */
	or	$1,%es:(%di)	/* there was activity on this chain */
#ifdef	LCCSIO
	push	%ax
	push	%ds
	mov	$<s>_lccsio, %ax
	mov	%ax, %ds
	mov	$1, _lccsio	/* Set SIO interrupt occurence flag */
	pop	%ds
	pop	%ax
#endif	LCCSIO
	cmp	$2,%ax
  	jne	sf_rintr
  	ljmp	sfxintr		/* it jumps to sf_getreason */
sf_rintr:				/* check if it's a receive intr */
	push	%si	/* preserve sd_array pointer, for possible subsequent */
	push	%ds			/* calls to sfxintr */
	push	%cx			/* %cx is restored from here */
	push	%cx			/* push unit for arg, it's trashed */
  	cmp	$4,%ax
  	jne	sf_srintr
  	lcall	asyrintr	/* It's a receive intr */
  	jmp	sf_postCcall
sf_srintr:			/* check if it's an error condition int */
  	cmp	$0,%ax
  	je	sf_eintr
  	lcall	asysrintr	/* Do error intr handler */
  	jmp	sf_postCcall
sf_eintr:
  	lcall	asyeintr	/* It's an external intr */
sf_postCcall:
	pop	%cx			/* pop arg */
	pop	%cx			/* pop unit */
	pop	%ds
	pop	%si
 	jmp	sf_getreason			/* M000 */
  
sf_noreason:	/* short people got... */
#ifdef	DEBUG
  	mov	$<s>asydebug,%ax
  	mov	%ax,%ds
  	test	%ds:asydebug
  	je	sf_out
  	push	$33
  	lcall	putchar
  	add	$2,%sp
#endif	DEBUG
sf_next:
/ check for more interrupts on this chain.
	test	%ds:16(%si)	/ test seg of next-port pointer
	jz	sf_out
	lds	%ds:14(%si),%si	/ get pointer to next sd in chain
	jmp	asyfast		/ start checking next port
sf_out:
  	lret	

sfxintr:
	cli	
	/ ds:si already points to sd_array[unit], %cx = unit
	les	%ds:10(%si),%di		/ es:di = &tp->t_state
	test	$1|0x100,%es:(%di)	/ check TIMEOUT | TTSTOP
	jne	sfx_stopout
	mov	%ds:2(%si),%bx
	test	%bx			/ any chars left in queue?
	jl	sfx_getdma		/ no, get more
sfx_putc:
	mov	%ds:4(%si),%dx		/ get sdp->sd_port
	mov	%ds:2(%si),%bx		/ get index
	dec	%ds:2(%si)		/ dec index (index-- implementation)
	add	%si,%bx			/ add sd base address
	movb	%ds:24(%bx),%al		/ add queue off in sd, get byte
	outb	(%dx)			/ kick port
/	sti				/ enable ints from now on
	ljmp	sf_getreason 	/ T_OUTPUT would stomp char we just sent
				/ so just wait til next tx int to load up queue
sfx_getdma:
	and	$0xffdf,%es:(%di)	/ state &= ~BUSY; (es:di is still state) 
/	sti				/ enable ints from now on
	push	%ds			/ ds:si must remain as sd pointer
	push	%si
	push	%cx			/ preserve unit number
	push	$0			/ T_OUTPUT
	push	%ds:8(%si)		/ seg of tp
	push	%ds:6(%si)		/ off of tp
	lcall	asyproc			/ asyproc(tp, T_OUTPUT);
	add	$6,%sp
	pop	%cx
	pop	%si
	pop	%ds
/	sti					/ enable ints from now on
	ljmp	sf_getreason	/ T_OUTPUT sent a char, we just return

sfx_stopout:			/ tty struct has told us to stop
	and	$0xffdf,%es:(%di) / state &= ~BUSY; keeps ttywait() from hanging
/	sti					/ enable ints from now on
	ljmp	sf_getreason	/ T_OUTPUT sent a char, we just return

	.def	asyfast; .val .; .scl -1; .endef
	.set	.F9,2


