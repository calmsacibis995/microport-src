	.file	"subadd.s"

	.ident	"@(#)subadd.s	1.2"

/
/   
/			s u b a d d . m o d   
/			===================   
/   
/	===============================================================   
/               intel corporation proprietary information   
/       this software  is  supplied  under  the  terms  of  a  license
/       agreement  or non-disclosure agreement  with intel corporation
/       and  may not be copied nor disclosed except in accordance with
/       the terms of that agreement.
/	===============================================================   
/   
/	function:   
/		performs floating point add, subtract, or compare   
/		of  non-zero,,unpacked valid numbers.   
/   
/	.globl:   
/		addx		subx		subadd   
/		sp_subadd	compar		mov_neg_frac   
/   
#include   "e80287.h"   

		.text

		.globl	addx
		.globl	subx
		.globl	subadd
		.globl	sp_subadd
		.globl	compar   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			addx:   
/			"""""   
/	function:   
/		fractional add. result_frac <-- frac1+frac2.   
/   
/	inputs:   
/		assumes operand records are set up.   
/   
/	outputs:   
/		al = 1 if there was a carry-out; al=0 otherwise.    
/   
/	data accessed:   
/		- word_frac1		word_frac2   
/		- result_word_frac   
/   
/	data changed:   
/		 result_word_frac   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

addx:
		mov	word_frac1(%bp),%ax		/do fractional add   
		add	word_frac2(%bp),%ax   
		mov	%ax,result_word_frac(%bp)
		mov	[word_frac1+2](%bp),%ax   
		adc	[word_frac2+2](%bp),%ax   
		mov	%ax,[result_word_frac+2](%bp)
		mov	[word_frac1+4](%bp),%ax   
		adc	[word_frac2+4](%bp),%ax   
		mov	%ax,[result_word_frac+4](%bp)
		mov	[word_frac1+6](%bp),%ax   
		adc	[word_frac2+6](%bp),%ax   
		mov	%ax,[result_word_frac+6](%bp)
		mov	[word_frac1+8](%bp),%ax   
		adc	[word_frac2+8](%bp),%ax   
		mov	%ax,[result_word_frac+8](%bp)
		movb	$0,%al   
		rclb	$1,%al				/put carry into %al   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			subx:   
/			""""   
/	function:   
/		2's complement fractional subtract.   
/		result_frac <-- frac1	- frac2.    
/   
/	inputs:   
/		assumes operand variables are set up.   
/   
/	outputs:   
/		al=1 if borrow; al=0 otherwise.   
/   
/	data accessed:   
/		- word_frac1		word_frac2   
/		- result_word_frac   
/   
/	data changed:   
/		- result_word_frac   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

subx:
		mov	word_frac1(%bp),%ax		/ fractional subtract   
		sub	word_frac2(%bp),%ax   
		mov	%ax,result_word_frac(%bp)
		mov	[word_frac1+2](%bp),%ax   
		sbb	[word_frac2+2](%bp),%ax   
		mov	%ax,[result_word_frac+2](%bp)
		mov	[word_frac1+4](%bp),%ax   
		sbb	[word_frac2+4](%bp),%ax   
		mov	%ax,[result_word_frac+4](%bp)
		mov	[word_frac1+6](%bp),%ax   
		sbb	[word_frac2+6](%bp),%ax   
		mov	%ax,[result_word_frac+6](%bp)
		mov	[word_frac1+8](%bp),%ax   
		sbb	[word_frac2+8](%bp),%ax   
		mov	%ax,[result_word_frac+8](%bp)
		movb	$0,%al				/set %al to borrow   
		rclb	$1,%al   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			subadd:   
/			"""""""   
/	function:   
/		floating point add or subtract.calculates unpacked result.   
/		sp_subadd is the entry point for log functions.   
/   
/	inputs:   
/		assumes operands are unpacked, valid, non-zero.   
/   
/	outputs:   
/		al set to true if underflow is possible
/		false if overflow is possible.   
/   
/	data accessed:   
/		- operation_type		sign1   
/		- expon1			msb_frac   
/		- offset_operand1		sign2   
/		- expon2			msb_frac2   
/		- offset_operand2		offset_result   
/		- result_sign			result_expon   
/		- result_word_frac   
/   
/	data changed:   
/		- result_sign			result_expon   
/		- result_word_frac   
/   
/	procedures:   
/		sticky_right_shift		right_shift   
/		addition_normlized		subtraction_normalize   
/		addx				subx   
/		test_5w				get_rnd_control   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

subadd:
		mov	$sticky_right_shift,%si	/NEED OFF shift proc ptr   
		jmp	form_expon_difference	/ merge with sp_subadd   
sp_subadd:   
		mov	$sp_right_shift,%si	/NEED OFF shift proc ptr   
form_expon_difference:   
		mov	expon1(%bp),%cx   
		sub	expon2(%bp),%cx   
		js	expon2_larger   
		cmp	$67,%cx			/set %cl to shift amount =   
		jle	adjust_frac2		/min(67,expon1-expon2)   
		movb	$67,%cl   
adjust_frac2:   
		movb	$0x00,%al		/al = bit to inject from left   
		mov	$offset_operand2,%di   
		push	%si   
		call	*%si		/shift op2 fraction   
		pop	%si   
		mov	expon1(%bp),%ax	/set result expon to expon1   
		mov	%ax,result_expon(%bp)
		testb	$0x80,msb_frac1(%bp)
		jnz	determine_operation_true / branch if frac1 is normal   
		jmp	frac2_normal_	/ test frac2   
expon2_larger:   
		neg	%cx			/set %cl to shift amount   
		cmp	$67,%cx   
		jle	adjust_frac1   
		movb	$67,%cl   
adjust_frac1:   
		push	%cx			/save shift amount   
		movb	$0x00,%al			/al = bit to inject   
		mov	$offset_operand1,%di   
		call	*%si   
		mov	expon2(%bp),%ax	/et result expon to expon2   
		mov	%ax,result_expon(%bp)
		pop	%cx   
frac2_normal_:   
		movb	false,%al   
		testb	$0x80,msb_frac2(%bp)	/ set %al to true if frac2 is   
		jz	determine_operation	/ normalized   
determine_operation_true:   
		movb	true,%al   
determine_operation:   
		cmpb	$sub_op,operation_type(%bp) /if op=sub_op, negate op2   
		jne	add_or_sub_   
		notb	sign2(%bp)
add_or_sub_:   
		movb	sign1(%bp),%ah   
		cmpb	sign2(%bp),%ah   
		jne	do_subtraction   
		call	addx			/do fractional add   
		mov	$offset_result,%di   
		call	addition_normalize	/normalize result after add   
		movb	sign1(%bp),%al   
		movb	%al,result_sign(%bp)	/set result sign to sign1   
		movb	false,%al		/indicate that underflow is not 
		ret				/ possible   
do_subtraction:   
		push	%ax			/save normalization indicator   
		movb	sign1(%bp),%al		/initially, set sign to sign1   
		movb	%al,result_sign(%bp)
		call	subx			/do fractional subtract   
		mov	$result_word_frac,%di   
		cmpb	$0,%al   
		je	detect_zero   
		xor	%ax,%ax			/form 2's comp of result_frac   
		not	(%bp,%di)   
		add	$1,(%bp,%di)   
		not	2(%bp,%di)   
		adc	%ax,2(%bp,%di)
		not	4(%bp,%di)   
		adc	%ax,4(%bp,%di)
		not	6(%bp,%di)   
		adc	%ax,6(%bp,%di)
		not	8(%bp,%di)   
		adc	%ax,8(%bp,%di)
		notb	result_sign(%bp)	/and complement result_sign   
detect_zero:   
		call	test_5w			/find true zero result   
		jnz	do_norm_		/ if not zero, test for denorm  
		call	get_rnd_control		/ sign is+unless rnd down   
		xorb	rnd_down,%al   
		dec	%ax   
		cbw   
		movb	%ah,result_sign(%bp)
		pop	%ax			/if al=true, then result should 
		cmpb	true,%al			/ be true zero   
		jne	subtract_done   
		mov	$0,result_expon(%bp)
		ret   
do_norm_:   
		pop	%ax			/if al=false, don't normalize   
		cmpb	true,%al			/ the result   
		jne	subtract_done   
		mov	$offset_result,%di   
		call	subtraction_normalize	/normalize the result   
subtract_done:   
		movb	true,%al		/indicate that underflow is   
		ret				/possible   
sp_right_shift:   
		call	right_shift   
		mov	(%bp,%di),%ax	/ %ss:bp+di -> word_frac after call   
		and	$0x1fff,%ax	/ mask word fraction   
		or	%ax,%dx		/ or it to the right_shift output   
		jz	sp_nonsticky	/ if zero, it must be nonsticky   
		or	$0x2000,(%bp,%di) / if nonzero, set bit of word_frac   
sp_nonsticky:   
		and	$0xe000,(%bp,%di) / now mask off most of word_frac   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			compar:   
/			"""""""   
/   
/	function:   
/		main: for compare and test instructions.   
/   
/	inputs:   
/		op1 and op2 operand records.   
/   
/	outputs:   
/		sets zero and sign status bits to indicate the result   
/		of the compare.	sets the invalid and denorm error bits   
/		if these errors are detected.   
/   
/	data accessed:   
/		- operation_type		tag1   
/		- expon1			tag2   
/		- expon2			result_sign   
/		- result_word_frac   
/   
/	data changed:   
/   
/	procedures:   
/		pop_free			subadd   
/		test_5w				clear_5w   
/		check_unnorm			check_denorm   
/		affine_infinity_		set_i_error   
/		set_s_bit			clear_s_bit   
/		set_z_bit			clear_z_bit   
/		i_error_   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

compar:
		jnz	set_uncertain			/ branch on stack error 
		cmpb	$compar_op,operation_type(%bp) /if not compar_op, then 
		je	set_up_check			/ it's test_op   
		movb	positive,sign2(%bp)	/test_op, set operand2  
		movb	special,tag2(%bp)	/ to zero   
		mov	$word_frac2,%di  / NEED OFFSET  
		call	clear_5w   
		mov	%ax,expon2(%bp)
set_up_check:   
		movb	tag1(%bp),%ah		/ %ah = value of tag1   
		movb	tag2(%bp),%al		/ %al = value of tag2   
		mov	$expon2,%di	/ %di = ptr to expon2   
		call	check_denorm		/ check denormalized tag2   
		xchgb	%ah,%al			/ %al = tag1 , %ah = tag2   
		mov	$expon1,%di	/ %di = ptr to expon1   
		call	check_denorm		/ check denormalized tag1   
		cmpb	inv,%al		/detect invalid operand(s)   
		je	invalid_error_detected   
		cmpb	inv,%ah   
		je	invalid_error_detected   
		mov	$offset_operand1,%di	/ %di = ptr offset_operand1   
		mov	$msb_frac1,%si	/ %si = ptr offset msb_frac1   
		call	check_unnorm		/ check unnormalized frac1   
		xchgb	%ah,%al			/ %al = tag2 , %ah = tag1   
		mov	$offset_operand2,%di	/ %di = ptr offset_operand2   
		mov	$msb_frac2,%si	/ %si = ptr offset msb_frac2   
		call	check_unnorm		/ check unnormalized frac2   
		call	affine_infinity_	/if affine, do compare   
		jnz	do_subtract   
		cmpb	infinty,%ah   
		jne	only_one_infinity_   
		cmpb	infinty,%al		/ if projective and both   
		je	operands_equal		/ infinity,then equal   
invalid_error_detected:   
		call	set_i_error		/set invalid error   
set_uncertain:   
		call	set_s_bit		/ set sign negative   
		jmp	zero_true		/ set zero to true   
operands_equal:   
		call	clear_s_bit		/ set sign positive   
zero_true:   
		call	set_z_bit		/ set zero to true   
		jmp	finish_up   
only_one_infinity_:   
		cmpb	infinty,%al			/ projective and only   
		je	invalid_error_detected		/ one infinity => i-err 
do_subtract:   
		movb	$sub_op,operation_type(%bp) /subtract op2 from op1  
		call	subadd   
		mov	$offset_result,%di		/detect a zero fraction 
		call	test_5w				/ in result   
		jz	operands_equal   
		call	i_error_   
		jnz	set_uncertain   
		call	clear_z_bit			/set zero to false   
		call	clear_s_bit   
		testb	$0x01,result_sign(%bp)	/ set sign to result   
		jz	finish_up   
		call	set_s_bit   
finish_up:   
		jmp	pop_free   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			check_denorm   
/   
/	function:   
/		this: procedure checks for a denormalized tag.   
/   
/	input:   
/		al = tag value   
/		di = ptr to exponent   
/   
/	output:   
/		if the tag is denormalized, the exponent is set to 0001
/		and the denorm_error is set to true.   
/   
/	procedures:   
/		set_denorm_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
check_denorm:
		cmpb	denormd,%al		/ is this operand denormalized_ 
		jne	denorm_done		/ no, exit immediately   
		call	set_denorm_error	/ yes, set error flag   
		mov	$0x0001,(%bp,%di) / exponent = $1   
denorm_done:   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			check_unnorm   
/   
/ 	function:   
/		this: procedure checks for unnormalized operands.   
/   
/	inputs:   
/		al = tag value of the operand being tested   
/		ah = tag value of the other operand   
/		di = ptr to of fraction   
/		si = ptr to msb of fraction   
/   
/	outputs:   
/		if the tag is invalid and the most significant bit of   
/		 the significand is zero, but the operand isn't a   
/		 pseudo-zero, the denorm_error is set true.   
/   
/	data accessed:   
/   
/	procedures:   
/		test_5w			affine_infinity_   
/		set_d_error   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

check_unnorm:
		cmpb	infinty,%al		/ is this operand infinity_   
		je	norm_infinity		/ yes, don't flag as invalid   
 		testb	$0x80,(%bp,%si)		/ no, is the msb $1 _   
		jnz	unnorm_done		/ yes, it isn't unnormal   
		push	%ax   
		call	test_5w			/ no, is it a pseudo-zero_   
		pop	%ax   
		jz	unnorm_done		/ yes, (0) behaves like true $0 
		call	affine_infinity_	/ the ndp doesn't raise a   
		jnz	set_denorm_error	/ d-error if there's also an   
		cmpb	infinty,%ah		/ i-error with unnormals   
		je	unnorm_done		/ so return without d-error   
set_denorm_error:   
		jmp	set_d_error		/ set the denormalized error   
norm_infinity:   
 		orb	$0x80,(%bp,%si)	/ set msb in case of pseudo-inf   
unnorm_done:   
		ret				/ check_unnorm/set_denorm_error   
