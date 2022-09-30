		.file "remsc.s"

/
/   
/			r e m s c . m o d   
/			=================   
/   
/	===============================================================   
/               intel corporation proprietary information   
/       this software  is  supplied  under  the  terms  of  a  license
/       agreement  or non-disclosure agreement  with intel corporation
/       and  may not be copied nor disclosed except in accordance with
/       the terms of that agreement.
/	===============================================================   
/
/         @(#)remsc.s	1.1 - 85/09/06
/   
/	functions:   
/		implements 80287 fprem instruction.   
/		implements 80287 fscale instruction.   
/   
/	.globl:   
/		remr			scale   
/		add_to_frac		add_to_frac_2   
/   
/	internal:   
/		remrx   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""  
   
#include   "e80287.h"   

		.text

		.globl	remr
		.globl	add_to_frac
		.globl	add_to_frac_2
		.globl	scale   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			remrx:   
/			""""""   
/	function:   
/		 fractional remainder.   
/   
/	inputs:   
/		assumes dividend is in op1, divisor is in op2
/		number of quotient bits to generate is in ax
/		and q is cleared to all zeroes.   
/   
/	outputs:   
/		it returns the fractional remainder in op1, and   
/		the low bits of the quotient in q.   
/   
/	data accessed:   
/		- offset_operand1		offset_operand2   
/		- offset_result   
/   
/	date changed:   
/		- frac1			frac2   
/   
/	procedures:   
/		right_shift_frac1_cl	right_shift_frac2_cl   
/		mov_neg_frac		add_to_frac   
/		left_shift_frac1_1   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

remrx:
		push	%ax			/save the loop count   
		movb	$1,%cl			/shift frac1 right $1 bit   
		call	right_shift_frac1_cl   
		movb	$1,%cl			/shift frac2 right $1 bit   
		call	right_shift_frac2_cl	/ returns cfb clear, %ch = $0  
		mov	$offset_operand2,%si	/result_frac <-- -frac2   
		mov	$offset_result,%di   
		movb	$0x05,%cl			/ load loop count   
complement_frac2:   
		mov	$0x0000,%ax		/ clear %ax, leave cf intact   
		sbb	(%bp,%si),%ax		/ $0 - frac2   
		mov	%ax,(%bp,%di)		/ store into result frac   
		inc	%si			/ bump offsets   
		inc	%si			/ (doesn't affect cf)   
		inc	%di   
		inc	%di   
		loop	complement_frac2	/ loop until result = -frac2   
		jmp	enter_loop   
form_next_frac1:   
		push	%cx			/ stack loop count   
		call	left_shift_frac1_1	/ shift frac1 left one bit   
		mov	$offset_operand2,%si   
		testb	$0x01,q		 	/if lsb of q = $1 then   
		jz	do_add			/frac1 <-- frac1+frac2   
enter_loop:   
		mov	$offset_result,%si	/else, frac1 <-- frac1-frac2   
do_add:   
		mov	$offset_operand1,%di   
		call	add_to_frac		/move carry-out from add   
		rclb	$1,q			/shift carry-out into q   
		pop	%cx   
		loop	form_next_frac1		/if looping done
		testb	$0x01,q			/ then one last iteration   
		jnz	last_shift		/frac1 <-- frac1+frac2   
		mov	$offset_operand2,%si   
		mov	$offset_operand1,%di   
		call	add_to_frac   
last_shift:   
		jmp	left_shift_frac1_1	/shift frac1 left $1 bit   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			add_to_frac:   
/			"""""""""""   
/	function:   
/		adds a 10-byte fraction to another 10-byte fraction.   
/   
/	inputs:   
/		ss:bp+si points to the source fraction, and %ss:bp+di points   
/		to the destination fraction.   
/   
/	outputs:   
/		carry flag set if there was a carry out, else reset.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

add_to_frac:
		mov	(%bp,%si),%ax   
add_to_frac_2:   
		add	%ax,(%bp,%di)   
		mov	2(%bp,%si),%ax   
		adc	%ax,2(%bp,%di)
		mov	4(%bp,%si),%ax   
		adc	%ax,4(%bp,%di)
		mov	6(%bp,%si),%ax   
		adc	%ax,6(%bp,%di)
		mov	8(%bp,%si),%ax   
		adc	%ax,8(%bp,%di)
simple_return:   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			remr:   
/			"""""   
/	function:   
/		80287 remainder instruction   
/   
/	inputs:   
/		assumes the operand records are set up.   
/   
/	outputs:   
/		results in result record.   
/   
/	data accessed:   
/		- result_rec_offset		offset_operand1   
/		- tag1				expon1   
/		- word_frac1			tag2   
/		- expon2			msb_frac2   
/		- offset_result   
/   
/	data changed:   
/		- tag1				expon1   
/   
/	procedures:   
/		set_up_indefinite		put_max_nan   
/		remrx				subtraction_normalize   
/		move_op_to_result		underflow_response   
/		put_result			test_4w   
/		clear_cond_bits			i_masked_   
/		set_c_bit			set_d_masked_   
/		d_masked_			set_s_bit   
/		set_z_bit			set_a_bit   
/		set_i_masked_   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

remr:
		pushf				/ save stack error flag   
		mov	$0x0001,%bx		/ load min exponent constant   
		movb	%bh,q			/ set quotient bits to zero   
		call	clear_cond_bits		/ clear all condition bits   
		popf   
		jnz	remr_stack_error	/ branch if operand missing   
		jmp	catch_special_cases	/ branch if no stack error   
remr_stack_error:   
		call	i_masked_   
		jz	simple_return		/unmasked stack error, return   
		mov	$offset_result,%di	/masked stack error, so give   
		call	set_up_indefinite	/indefinite   
		jmp	move_result_to_result   
do_normalize:   
		mov	$offset_operand1,%di   
		call	subtraction_normalize   
		movb	valid,tag1(%bp)
		cmp	$0x0001,expon1(%bp) /if expon1 < 1, then underflow  
		jge	put_op1   
		mov	$offset_operand1,%si	/give std underflow response   
		call	move_op_to_result   
		call	underflow_response   
move_result_to_result:   
		mov	$offset_result,%di   
		jmp	put_the_result   
valid_case:   
		mov	expon1(%bp),%ax		/num q bits <- exp1-exp2 + 1   
		sub	expon2(%bp),%ax   
		inc	%ax			/if num_quotient_bits <= zero
		jle	put_op1			/then op1 is the answer   
		cmp	$64,%ax			/if num_quotient_bits > 64
		jle	calc_exponent		/ then set to $64   
		mov	$64,%ax   
		call	set_c_bit		/if num q bits > $64, set c_bit   
calc_exponent:   
		sub	%ax,expon1(%bp)		/remainder exponent <- expon1 - 
		inc	expon1(%bp)		/ num_quotient_bits+1   
		call	remrx			/ calculate remainder fraction  
		xor	%ax,%ax			/ detect zero result   
		mov	$word_frac1+2,%di   
		call	test_4w   
		jnz	do_normalize   
		mov	%ax,expon1(%bp)		/if remainder fraction = 0
		movb	special,tag1(%bp)	/ then set result to 0   
put_op1:   
		mov	$offset_operand1,%di   
put_the_result:   
		mov	offset_result_rec,%si   
		call	put_result   
		movb	q,%al   
		shlb	$6,%al			/move $3 low bits of q to   
		jnc	fix_z_bit		/ s, z, and a bits   
		call	set_s_bit   
fix_z_bit:   
		shlb	$1,%al   
		jnc	fix_a_bit   
		call	set_z_bit   
fix_a_bit:   
		shlb	$1,%al   
		jnc	remr_done   
		jmp	set_a_bit   
catch_special_cases:   
		movb	tag2(%bp),%al   
		movb	tag1(%bp),%ah   
                cmpb    denormd,%al   
                jne	catch_denormal		/ branch if dvsr not denormal   
		call	masked_denorm_		/ is there a masked d-error_   
		testb	$0x80,msb_frac2(%bp)	/ yes, is it a (denormal)_   
		jz	invalid_operation	/ no, denormal divisor invalid  
		mov	%bx,expon2(%bp)		/ yes, make divisor valid   
		movb	valid,%al   
		movb	%al,tag2(%bp)
catch_denormal:   
                cmpb    denormd,%ah
                jne     catch_invalid   
		call	masked_denorm_   
		mov	%bx,expon1(%bp)		/masked denorm error, so
		movb	%bh,tag1(%bp)		/make op1 valid   
catch_invalid:   
 		cmpb	inv,%ah   
		je	invalid_operand   
		cmpb	inv,%al   
		je	invalid_operand   
		cmpb	infinty,%ah   
		je	invalid_operation   
		cmpb	infinty,%al   
		je	put_op1   
		testb	$0x80,msb_frac2(%bp)	/unnormalized divisor_   
		jz	invalid_operation   
		cmpb	valid,%al   
		jne	invalid_operation   
		jmp	valid_case   
remr_done:   
		ret   
invalid_operation:   
		call	masked_invalid_   
		mov	$offset_operand1,%di	/masked invalid operation
		call	set_up_indefinite	/ so give indefinite   
		jmp	put_op1   
invalid_operand:   
		call	masked_invalid_   
		mov	offset_result_rec,%si	/masked invalid operand
		jmp	put_max_nan		/so give maximum nan   
masked_invalid_:   
		call	set_i_masked_		/ is the invalid error masked_   
		jmp	masked_error_		/ merge with masked_denorm_   
masked_denorm_:   
		call	set_d_masked_		/ d-error masked_   
masked_error_:   
		jnz	masked_error		/ yes, return to remr   
		pop	%ax			/ no, exit remr   
masked_error:   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			scale:   
/			""""""   
/	function:   
/		emulates the 80287 scale instruction.   
/   
/	inputs:   
/		operand1 and operand2   
/   
/	outputs:   
/		scaled operand in operand2 record.   
/		error indicators set.   
/   
/	data accessed:   
/		- result_rec_offset		tag1   
/		- word_frac1			offset_operand2   
/		- tag2				expon2   
/		- offset_result			extra_word_reg   
/   
/	data changed:   
/		- expon2   
/   
/	procedures:   
/		set_up_indefinite	fix16		put_result   
/		move_op_to_result	put_max_nan	underflow_response   
/		overflow_response	set_d_error	d_masked_   
/		store_rnd_control	d_error_	set_i_masked_   
/		clear_p_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

scale:
		movb	tag1(%bp),%al			/ %al = tag1  %ah = tag2
		movb	tag2(%bp),%ah   
		jnz	raise_stack_error		/ branch on stack error 
		cmpb	special,%al			/scale factor = 0_   
		je	give_op2   
		cmpb	special,%ah			/number = 0_   
		je	give_op2   
		cmpb	denormd,%al			/ set d-error if   
		jne	check_tag2			/ appropriate   
		call	set_d_error   
		movb	valid,%al   
		movb	%al,tag1(%bp)
check_tag2:   
		cmpb	denormd,%ah   
		jne	check_denorm_error   
		call	set_d_error   
		mov	$0x0001,expon2(%bp)
		movb	valid,%ah   
		movb	%ah,tag2(%bp)
check_denorm_error:   
		call	d_error_			/if unmasked d-error
		jz	check_tags			/ then return   
		call	d_masked_   
		jnz	check_tags			/continue if d masked   
exit_scale:   
		ret   
raise_stack_error:   
		call	i_masked_   
		jz	exit_scale			/done if error unmasked 
give_indef:   
		mov	$offset_operand2,%di		/set op2 to indefinite  
		call	set_up_indefinite   
		jmp	give_op2			/ give it as the result 
check_tags:   
		and	%ax,%ax				/find valid/valid case  
		jnz	special_cases   
		/ push	%ds:sr_masks			/save current controls  
		movb	sr_masks,%cl
		push	%cx
		movb	rnd_to_zero,%al		/set truncation control 
		call	store_rnd_control   
		call	fix16				/convert scale factor   
		/ pop	%ds:sr_masks			/ restore rnd control   
		pop	%cx
		movb	%cl,sr_masks
		jnz	over_or_underflow		/ if zf=0, oflow on fix 
 		cmpb	true,extra_word_reg(%bp)	/ old p-error flag_   
		je	add_scale_factor		/ yes, leave p-error   
		call	clear_p_error			/ no, exact if valid   
add_scale_factor:   
		mov	[word_frac1+8](%bp),%ax		/ int16 scale factor   
		add	%ax,expon2(%bp)			/add scale to op2   
		js	over_or_underflow		/if msb set, over/under 
		jz	scale_underflow			/ underflow if $0x0000  
		cmp	$0x7fff,expon2(%bp)	/overflow if 0x7fff   
		je	scale_overflow   
give_op2:   
		mov	$offset_operand2,%di   
put_scaled_result:   
		mov	offset_result_rec,%si   
		jmp	put_result   
special_cases:   
		cmpb	inv,%al			/catch i-error   
		je	handle_i_error   
		cmpb	inv,%ah
		je	handle_i_error   
		cmpb	infinty,%al			/op2=inf, op1=valid   
		jne	give_op2			/-> give inf as result  
		call	set_i_masked_			/op1=inf -> invalid   
		jnz	give_indef			/ err, indef if masked  
		ret   
handle_i_error:   
		call	set_i_masked_			/op1 and/or op2 invalid 
		jz	exit_scale   
		mov	offset_result_rec,%si   
		jmp	put_max_nan			/if masked, put max nan 
over_or_underflow:   
		cmpb	positive,sign1(%bp)		/ if scale -, underflow 
		jz	scale_overflow			/otherwise overflow   
scale_underflow:   
		push	$underflow_response	/ NEDD OFFunderflow response   
		jmp	scale_error		/ merge with overflow   
scale_overflow:   
		push	$overflow_response	/ NEED OFFoverflow response   
scale_error:   
		mov	$offset_operand2,%si		/ move op2 to result   
		call	move_op_to_result   
		pop	%si   
		call	*%si			/ call response rtn   
		mov	$offset_result,%di   
		jmp	put_scaled_result   
