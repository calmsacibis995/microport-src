	.file	"cswitch.s"
/
/ @(#)cswitch.s	1.9
/	Routines to change context
/
	.text

	.globl	twitch
twitch:
	push	%bp			/ establish ...
	mov	%sp,%bp			/ ... stack frame
/
/	save this processes state in his TSS,
/	restore the new processes state, and
/	resume the new process
/
	ljmp	*6(%bp)			/ jump to TSS
/
/	return here when this process is context switched
/	back
/
	pop	%bp
	lret

/
/ setjmp
/
/	setjmp( u.u_?sav )
/
/	The order and offsets in the u.u_?sav area are:
/
/	u.u_?sav[ 0 ] = sp
/	u.u_?sav[ 1 ] = bp
/	u.u_?sav[ 2 ] = ss
/	u.u_?sav[ 3 ] = ds
/	u.u_?sav[ 4 ] = ip
/	u.u_?sav[ 5 ] = cs
/	u.u_?sav[ 6 ] = flags		/ only for save()
/
	.globl	setjmp
setjmp:
	push	%bp			/ establish stack frame
	mov	%sp,%bp			/     "       "     "
	push	%di			/ save what I'm gonna trash
	mov	6(%bp),%di		/ offset of u_?sav array
	mov	8(%bp),%es		/ selector of u_?sav array
	mov	%sp,%es:0(%di)		/ save sp
	mov	0(%bp),%ax		/ get old bp
	mov	%ax,%es:2(%di)		/ save bp
	mov	%ss,%es:4(%di)		/ save ss
	mov	%ds,%es:6(%di)		/ save ds
	mov	2(%bp),%ax		/ get old ip
	mov	%ax,%es:8(%di)		/ save ip
	mov	4(%bp),%ax		/ get old cs
	mov	%ax,%es:10(%di)		/ save cs
	xor	%ax,%ax			/ zero return code
	pop	%di			/ restore regs
	pop	%bp			/    "     "
	lret				/ return to caller


/
/ save
/
/	save( u.u_?sav )
/
	.globl	save
save:
	push	%bp			/ establish stack frame
	mov	%sp,%bp			/     "       "     "
	push	%di			/ save what I'm gonna trash
	mov	6(%bp),%di		/ offset of u_?sav array
	mov	8(%bp),%es		/ selector of u_?sav array
	mov	%sp,%es:0(%di)		/ save sp
	mov	0(%bp),%ax		/ get old bp
	mov	%ax,%es:2(%di)		/ save bp
	mov	%ss,%es:4(%di)		/ save ss
	mov	%ds,%es:6(%di)		/ save ds
	mov	2(%bp),%ax		/ get old ip
	mov	%ax,%es:8(%di)		/ save ip
	mov	4(%bp),%ax		/ get old cs
	mov	%ax,%es:10(%di)		/ save cs
	pushf				/ get old flags
	pop	%es:12(%di)		/ save flags
	xor	%ax,%ax			/ zero return code
	pop	%di			/ restore regs
	pop	%bp			/    "     "
	lret				/ return to caller


/
/ longjmp
/
/	longjmp( u.u_?sav )
/
	.globl	longjmp
longjmp:
	mov	%sp,%bp			/ so I can get at qsav address on stack
	mov	4(%bp),%di		/ get offset of qsav
	mov	6(%bp),%es		/ get selector of qsav
	cli				/ NO INTERRUPTS WHILE STACK INVALID
	mov	%es:0(%di),%sp		/ restore sp
	mov	%es:2(%di),%bp		/ restore bp
	mov	%es:4(%di),%ss		/ restore ss
	mov	%es:6(%di),%ds		/ restore ds
	sti				/ re-enable interrupts
	mov	$1,%ax			/ true return
	ljmp	*%es:8(%di)		/ non-local goto
