	.file	"status.s"

	.ident	"@(#)status.s	1.2"

/
/   
/                        s t a t u s . m o d   
/                        ===================   
/   
/	================================================================   
/               intel corporation proprietary information   
/       this software  is  supplied  under  the  terms  of  a  license
/       agreement  or non-disclosure agreement  with intel corporation
/       and  may not be copied nor disclosed except in accordance with
/       the terms of that agreement.
/	=================================================================   
/   
/
/	function:   
/	       operation cluster for 80287 status register   
/   
/	.globl:   
/		stcw			stenv			pop_free   
/		incr_top		decr_top		save_status   
/		restore_status		clear_p_error		init   
/		clex			stsw			ldcw   
/		get_precision		store_precision		get_rnd_control   
/		store_rnd_control	u_masked_		z_masked_   
/		d_masked_		i_masked_		o_masked_   
/		get_reg_tag		affine_infinity_	get_top   
/		set_p_error		p_error_		set_u_error   
/		set_o_error		set_z_error		set_i_error   
/		i_error_		d_error_		set_d_error   
/		set_z_bit		clear_z_bit		set_s_bit   
/		clear_s_bit		set_a_bit		clear_a_bits   
/		set_c_bit		clear_c_bit		store_reg_tag   
/		set_i_masked_		set_d_masked_		set_z_masked_   
/		clear_cond_bits   
/   
/   
/   


#include   "e80287.h"   

		.text

		.globl	incr_top
		.globl	decr_top
		.globl	save_status
		.globl	pop_free
		.globl	ldcw
		.globl	stenv
		.globl	clex   
		.globl	stsw
		.globl	stcw
		.globl	get_precision
		.globl	store_precision
		.globl	get_rnd_control   
		.globl	store_rnd_control
		.globl	u_masked_
		.globl	d_masked_
		.globl	get_top   
		.globl	i_masked_
		.globl	o_masked_
		.globl	get_reg_tag
		.globl	affine_infinity_   
		.globl	set_p_error
		.globl	set_u_error
		.globl	set_o_error
		.globl	init
		.globl	emul_sinit
		.globl	clear_p_error   
		.globl	set_i_error
		.globl	set_d_error
		.globl	i_error_
		.globl	d_error_
		.globl	p_error_   
		.globl	set_z_bit
		.globl	clear_z_bit
		.globl	set_s_bit
		.globl	clear_s_bit
		.globl	set_a_bit   
		.globl	clear_cond_bits
		.globl	set_c_bit
		.globl	clear_c_bit
		.globl	store_reg_tag   
		.globl	set_i_masked_
		.globl	set_d_masked_
		.globl	set_z_masked_
		.globl	clear_a_bit   
		.globl	restore_status   
/   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	the routines in this section manipulate the status and control   
/ 	information stored in the 80287 status data segment, a_msr.   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

/	transfer field group:	get_precision, store_precision, get_rnd_control   
/	 store_rnd_control, get_top, get_reg_tag, store_reg_tag   
/   
/	inputs:	the "get" routines require no input parameters, except for the   
/		procedures: and store_reg_tag which expect the   
/		register number to be in %al andb %cl, respectively   
/		the "store" routines input the value to be stored in al.   
/   
/	outputs:  get_precison returns the 2-bit precision field in dl.   
/		  all other routines return justified values in al.   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

get_precision:
		movb	precision_mask,%dl	/ load precision field mask   
		andb	sr_controls,%dl		/ mask in precision control   
		ret   
/   
store_precision:
		andb	nprecision_mask,sr_controls / clear precision   
		orb	%al,sr_controls		/ store new precision control   
		ret   
/   
get_rnd_control:
		movb	sr_controls,%al		/ load control byte   
		andb	rnd_control_mask,%al	/ mask in rounding control   
		ret   
/   
store_rnd_control:   
		andb	nrnd_control_mask,sr_controls / clear old field   
		orb	%al,sr_controls		/ store new rounding control   
		ret   
/   
get_top:
		movb	sr_flags,%al		/ load status flag byte   
		andb	top_mask,%al		/ mask in top field   
		shrb	$3,%al			/ right justify top field   
		ret   
/   
get_reg_tag:
		cbw				/ form word bit count   
		mov	%ax,%cx			/ store in %cx   
		shl	$1,%cx			/ bit count = 2*reg num   
		mov	sr_tags,%ax		/ load tag word   
		shr	%cl,%ax			/ shift to tag of interest   
		andb	empty,%al		/ mask out other tags   
		cmpb	inv,%al			/ if tag not = 2
		jne	got_tag			/  no further decoding needed   
		mov	%cx,%bx			/ else, must examine register   
		shl	$2,%bx			/ form index to sr_regstack   
		add	%cx,%bx			/ index = 10*register num   
		add	$sr_regstack,%bx	/NEED OFF +start of sr_regstack 
		mov	8(%bx),%ax		/ load register exponent   
		and	$0x7fff,%ax		/ mask off sign bit   
		jz	got_denormd		/ if exponent zero, denormal   
		and	6(%bx),%ax		/ test fraction for 8000...0   
		or	4(%bx),%ax   
		or	2(%bx),%ax   
		or	(%bx),%ax   
		movb	infinty,%al		/ load infinity tag   
		jz	got_tag			/ if fraction zero, infinity   
		movb	inv,%al			/ else, it's a nan   
got_tag:   

		ret   
got_denormd:   
		movb	denormd,%al		/ load denormal tag   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/	test status bit group:	u_masked_, z_masked_, d_masked_, i_masked_
/	 o_masked_, affine_infinity_, p_error_, i_error_, d_error_
/	 set_i_masked_, set_d_masked_   
/   
/	inputs:	 no input values are required.   
/   
/	outputs:  all boolean function return the complemented bit value   
/	 in the zf.  (test with jz on bit = 0.)   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

u_masked_:
		testb	underflow_mask,sr_masks	/ test the u mask   
		ret   
/   
set_d_masked_:
		call	set_d_error		/ set the d error   

d_masked_:   
		testb	denorm_mask,sr_masks 	/ test the d mask   
		ret   
/   
set_i_masked_:
		call	set_i_error		/ set the i error   

i_masked_:   
		testb	invalid_mask,sr_masks	/ test the i mask   
		ret   
/   
o_masked_:
		testb	overflow_mask,sr_masks	/ test the o mask   
		ret   
/   
affine_infinity_:   
		testb	infinity_control_mask,sr_controls / test ic   
		ret   
/   
p_error_:
		testb	inexact_mask,sr_errors	/ test the p error   
		ret   
/   
i_error_:
		testb	invalid_mask,sr_errors	/ test the i error   
		ret   
/   
d_error_:
		testb	denorm_mask,sr_errors	/ test the d error   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/	set and reset bit group:  set_p_error, set_u_error, set_o_error   
/	 set_z_masked_, set_i_error, set_d_error, clear_s_bit, clear_z_bit   
/	 set_s_bit, set_z_bit, set_a_bit, clear_a_bit, set_c_bit, clear_c_bit   
/	 clear_cond_bits, clear_p_error   
/   
/	inputs:	 no input values are required.   
/   
/	outputs:: procedures set or reset the indicated status bit   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

set_p_error:
		orb	inexact_mask,sr_errors	/ set the p-error   
		ret   
/   
set_u_error:
		orb	underflow_mask,sr_errors / set the u-error   
		ret   
/   
set_o_error:
		orb	overflow_mask,sr_errors	/ set the o-error   
		ret   
/   
set_i_error:
		orb	invalid_mask,sr_errors	/ set the i-error   
		ret   
/   
set_d_error:
		orb	denorm_mask,sr_errors	/ set the d-error   
		ret
/   
set_z_masked_:
		orb	zero_divide_mask,sr_errors / set the z-error   
		testb	zero_divide_mask,sr_masks   / test the z mask   
		ret   
/   
set_s_bit:
		orb	sign_mask,sr_flags	/ set the s-bit   
		ret   
/   
set_z_bit:
		orb	zero_mask,sr_flags	/ set the z-bit   
		ret   
/   
set_a_bit:
		orb	a_mask,sr_flags		/ set the a-bit   
		ret   
/   
set_c_bit:
		orb	c_mask,sr_flags		/ set the c-bit   
		ret   
/   
clear_s_bit:
		andb	nsign_mask,sr_flags	/ clear the s-bit   
		ret   
/   
clear_z_bit:
		andb	nzero_mask,sr_flags	/ clear the z-bit   
		ret   
/   
clear_a_bit:
		andb	na_mask,sr_flags	/ clear the a-bit   
		ret   
/   
clear_cond_bits:
		/ andb	not [c_mask+zero_mask+sign_mask+a_mask],sr_flags
		andb	ncombo_mask,sr_flags
		ret				/ clear all condition bits   
/   
clear_c_bit:
		andb	nc_mask,sr_flags	/ clear the c-bit   
		ret   
/   
clear_p_error:	
		andb	ninexact_mask,sr_flags / clear the p-error   

		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			pop_free:   
/	function:   
/		pops the stack and/or frees the register(s) as required   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

pop_free:
		call	i_masked_		/ is invalid masked_   
		jnz	check_op1		/ if so, forget error   
		call	i_error_		/ check for i-error   
		jnz	common_return		/ if error, don't pop   
check_op1:   
		movb	op1_use_up(%bp),%al	/ pop or free op1   
		call	process_use_up
		movb	op2_use_up(%bp),%al	/ pop or free op2   
process_use_up:
		cmpb	pop_stack,%al		/ is it a pop_stack_   
		je	pop_it			/: process it   
		xorb	free,%al			/ no, is it free reg_   
		jnz	exit_process		/: done with use_up   
		call	get_top			/ yes, get top pointer   
		addb	reg_num(%bp),%al		/ convert relative num  
		andb	$0x07,%al			/  to absolute reg num  
empty_reg_tag:   
		movb	%al,%cl			/ %cl = reg number   
		movb	empty,%al		/ %al = new tag value   
store_reg_tag:   
		shlb	$1,%cl			/ bit count = 2*reg num   
		ror	%cl,sr_tags		/ rotate tag to low bits   
 		andb	nempty,sr_tags / clear old reg num tag   
		andb	empty,%al		/ clear bits 2-7 of new tag   
 		orb	%al,sr_tags	/ store new reg num tag and   
		rol	%cl,sr_tags		/  rotate the tag word back   
exit_process:
		ret   
pop_it:   
		call	get_top			/ pop the stack once   
		call	get_reg_tag		/ get register tag   
		cmpb	empty,%al		/ is the top empty_   
		jne	pop_ok			/ no, stack may be popped   
		call	set_i_masked_		/ yes, stack error   
		jz	exit_process		/: if unmasked   
pop_ok:   
		call	get_top   
		call	empty_reg_tag		/set tag of top empty   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			incr_top:   
/	function:   
/		increments stack pointer   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

incr_top:
		movb	$0x08,%cl		/ load increment top constant   
		jmp	adjust_top		/ merge with decr_top   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			decr_top:   
/	function:   
/		decrements stack pointer   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

decr_top:
		movb	$0x38,%cl		/ load decrement top constant   
adjust_top:   
		movb	sr_flags,%al		/ get old top   
		andb	top_mask,%al   
		xorb	%al,sr_flags		/ clear old top field   
		addb	%cl,%al			/ increment/decrement top   
		andb	top_mask,%al		/ mask off bits 6-7   
		orb	%al,sr_flags		/ store new top field   
common_return:   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""" 
/			restore_status:   
/	function:   
/		implements the 80287 ldenv instruction.   
/		restores status register from memory.   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

restore_status:
		push	%ds   
		pop	%es			/ load destination base   
		lds	mem_operand_pointer(%bp),%si / load environment pointer
		mov	$sr_masks,%di / load dest offset   
		mov	$0x0007,%cx		/ load environment from memory  
		rep	     			/ move environment to a_msr   
		smov
		push	%es			/ reload a_msr base   
		pop	%ds   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			init:   
/	function:   
/		implements 80287 init instruction.  intializes   
/		status register including mode word and error mask.   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
init:
		mov	$0x03ff,%ax		/ initialize mode word   
		mov	%ax,sr_masks   
		cbw				/ %ax = $0x0ffff   
		mov	%ax,sr_tags		/ register tags = empty   
		inc	%ax			/ %ax = $0x0000   
		movb	%al,sr_errors		/ clear the error flags   
		andb	ntop_mask,sr_flags	/ top of stack = $0   

		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			ldcw:   
/	function:   
/		implements 80287 ldcw instruction.  80287 control word   
/		loaded (from memory location specified in instruction).   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

ldcw:
		les	mem_operand_pointer(%bp),%bx /get new mode word   
		mov	%es:(%bx),%ax   
		mov	%ax,sr_masks	/ store in status reg   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			stenv:   
/	function:   
/		implements 80287 fstenv instruction.  80287 environmemt   
/		stored (in memory location specified in the instruction).   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

stenv:
		call	save_status		/ store status to memory   
		orb	$0x3f,sr_masks		/ set all individual masks   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			clex:   
/	function:   
/		 clears all 80287 errors set in status register   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

clex:
		movb	$0,sr_errors		/ clear error byte   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/                        stsw   
/       function:   
/                stores status word in memory location.  (i.e., implements   
/                fstsw instruction.)  also implements 'fstsw ax'.   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

stsw:
		movb	op2_location(%bp),%al	/ load op2 location   
		cmpb	$reg,%al		/ is this fstsw ax_   
		mov	sr_errors,%ax		/ get status word   
		je	stsw_ax			/ branch on fstsw %ax   
store_word:   
		les	mem_operand_pointer(%bp),%bx   
		mov	%ax,%es:(%bx)          	/ store status to memory
		ret   
stsw_ax:   
		mov	%ax,saved_ax(%bp)	/ store into register %ax   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			stcw:   
/	function:   
/		stores control word in memory location. (i.e., implements   
/		fstcw instruction.)   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

stcw:
		mov	sr_masks,%ax	/get control word   
		jmp	store_word           	/store it   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			save_status:   
/	function:   
/		saves status register to location specified by   
/		memory operand pointer(in %es:di)   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

save_status:
		mov	$sr_masks,%si / load source offset   
		les	mem_operand_pointer(%bp),%di / load destination pointer
		mov	$0x0007,%cx   
		rep	     			/ store $80287 state to memory  
		smov
		ret   

emul_sinit:
		cld
		push	%bp
		mov	%sp,%bp
		mov	6(%bp),%ax
		mov	%ax,%ds
		mov	%ax,%es
		mov	$0,sr_flags
		call	init

		mov	8(%bp),%ax             /second argument in large model
		not	%ax                    /flip these off
		and	%ax,sr_masks           /
		mov	10(%bp),%ax       /flip third argument on
		or	%ax,sr_masks

		leave
		lret

