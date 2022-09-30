	.file	"arith.s"

	.ident	"@(#)arith.s	1.2"


/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/			a r i t h . m o d   
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
/         @(#)arith.s	1.1 - 85/09/06
/   
/	function:   
/		implements add, subtract, multiply, and divide.   
/   
/	.globl:   
/		move_op_to_result		overflow_response   
/		underflow_response		put_max_nan   
/		put_indefinite			arith   
/   
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

#include   "e80287.h"   

/...define some floating point constants...   

indefinite_pattern:	.value	0,0,0,0xc000,0xffff   
infinity_pattern:	.value	0,0,0,0x8000,0x7fff   
zero_pattern:		.value	0,0,0,0,0   
max_valid_pattern:	.value	0xffff,0xffff,0xffff,0xffff,0x7ffe   
arith_table:		.value	subadd,subadd,mult,divid   

/...the following jump tables refer to the following op1/op2 cases    
/   
/   (where v=valid, z=zero, and f=infinity):   
/		v/v,v/z,v/f,z/v,z/z,z/f,f/v,f/z,f/f, in that order ..   

/add_table   
special_table:	.value	handle_non_special_cases
		.value	handle_non_special_cases
	    	.value	second_operand
		.value	handle_non_special_cases
	       	.value	handle_non_special_cases
		.value	second_operand
	       	.value	first_operand
	       	.value	first_operand
		.value	add_sub_infinities
/sub_table   
              	.value	handle_non_special_cases
		.value	handle_non_special_cases
		.value	neg_second_operand
		.value	handle_non_special_cases
		.value	handle_non_special_cases
		.value	neg_second_operand
	       	.value	first_operand
	       	.value	first_operand
		.value	add_sub_infinities
/mul_table   
              	.value	handle_non_special_cases
		.value	exor_signed_zero   
		.value	exor_signed_infinity
		.value	exor_signed_zero
		.value	exor_signed_zero   
		.value	invalid_error_detected
		.value	exor_signed_infinity   
		.value	invalid_error_detected
		.value	exor_signed_infinity   
/div_table   
		.value	handle_non_special_cases
		.value	divide_by_zero   
		.value	exor_signed_zero
		.value	divide_into_zero   
		.value	invalid_error_detected
		.value	exor_signed_zero   
		.value	exor_signed_infinity
		.value	exor_signed_infinity   
		.value	invalid_error_detected   

		.text

		.globl	addition_normalize
		.globl	round
		.globl	gradual_underflow
		.globl	put_result
		.globl	pop_free
		.globl	subadd
		.globl	mult
		.globl	divid
		.globl	set_up_indefinite
		.globl	test_4w
		.globl	special_round_test
		.globl	directed_round_test
		.globl	get_precision
		.globl	u_masked_
		.globl	set_i_masked_
		.globl	d_masked_
		.globl	affine_infinity_
		.globl	i_masked_
		.globl	o_masked_
		.globl	set_p_error
		.globl	set_u_error
		.globl	set_o_error
		.globl	set_z_masked_
		.globl	set_d_masked_
		.globl	i_error_
		.globl	d_error_

                .globl  put_indefinite
		.globl  move_op_to_result
		.globl	put_max_nan   
		.globl	overflow_response
		.globl	underflow_response
		.globl	arith   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			put_indefinite:   
/   
/	function:   
/		sets unpacked result to indefinite.   
/   
/	inputs:   
/   
/	outputs:   
/		result set to indefinite; tag set to invalid.   
/   
/	data accessed:   
/		- result_sign			result_tag   
/   
/	data changed:   
/		- result   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

put_indefinite:	
		mov	$indefinite_pattern,%si   	
		movb	negative,result_sign(%bp)   
put_inv_tag:   
		movb	inv,result_tag(%bp)	/ fall into set_constant_result 
set_constant_result:   
		lea	[offset_result+2](%bp),%di
		push	%ds			/ save a_msr   
		push	%cs   
		pop	%ds   
		push	%ss   
		pop	%es   
		mov	$0x0004,%cx   
		rep	      			/ move fraction   
		smov
		mov	%cx,result_word_frac(%bp)	/ clear ls word   
		mov	(%si),%ax			/ move exponent 
		mov	%ax,result_expon(%bp)		/ to result   
		pop	%ds				/ reload a_msr   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			move_op_to_result:   
/   
/	function:   
/		moves operand to result.   
/   
/	inputs:   
/		assumes %ss:si points to op1 or op2   
/   
/	outputs:   
/		result in result   
/   
/	data accessed:   
/		- expon1			offset_result   
/		- result_expon   
/   
/	data changed:   
/		- result   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
move_op_to_result:
		push	%ds			/ save a_msr   
		add	%bp,%si   
		push	%ss   
		pop	%ds   
		push	%ss   
		pop	%es   
		lea	offset_result(%bp),%di   
		mov	$[[offset_operand2-offset_operand1]\/2],%cx   
		rep	     			/ move 7 words   
		smov
		pop	%ds			/ reload a_msr   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			overflow_response:   
/   
/	function:   
/		responds to detected overflow.   
/   
/	inputs:   
/   
/	outputs:   
/		overflow error indication(s) set when appropriate   
/		correct masked or unmasked result   
/   
/	data accessed:   
/		- result_sign			result_tag   
/		- result_expon			msb_result   
/   
/	data changed:   
/		- result   
/   
/	procedures:   
/		directed_round_test		special_round_test   
/		o_masked_			set_p_error   
/		set_o_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

overflow_response:
		call	o_masked_   
		jnz	masked_overflow			/ branch if o masked   
		call	set_o_error			/ set overflow error   
		sub	wrap_around_constant,result_expon(%bp)	/ subtract   
		jmp	give_valid_o_result	/wrap-around constant   
masked_overflow:   
		call	set_p_error   
		call	directed_round_test		/if rnd_up or rnd_down
		jnz	no_o_error			/ don't set o_error   
		call	set_o_error			/ else, overflow error  
		jmp	put_infinity   
no_o_error:   
		movb	result_sign(%bp),%al		/check for special   
		call	special_round_test		/ rounding case   
		jnz	special_round_not_overflow   
put_infinity:   
		mov	$infinity_pattern,%si  / NEED OFF 
		jmp	put_inv_tag   
special_round_not_overflow:   
		testb	$0x80,msb_result(%bp)		/result unnorm_   
		jz	give_max_expon   
		call	give_valid_o_result   
		mov	$max_valid_pattern,%si	/NEED OFFno,set result to max   
		jmp	set_constant_result		/ valid number   
give_max_expon:   
		mov	$0x7ffe,result_expon(%bp)	/yes, set expon to max  
give_valid_o_result:   
		movb	valid,result_tag(%bp)   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			underflow_response:   
/   
/	function:   
/		responds to a detected underflow    
/   
/	inputs:   
/   
/	outputs:   
/		sets underflow error indication(s) if appropiate   
/		correct masked or unmasked results in result   
/   
/	data accessed:   
/		- offset_result			result_sign   
/		- result_tag			result_expon   
/		- result_word_frac   
/   
/	data changed:   
/		- result   
/   
/	procedures:   
/		directed_round_test		gradual_underflow   
/		round				test_4w   
/		set_u_error			u_masked_   
/		get_precision   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

underflow_response:
		call	u_masked_			/is underflow masked_   
		jnz	masked_underflow		/ yes, don't set flag   
		call	set_u_error			/ no, set error flag   
		add	wrap_around_constant,result_expon(%bp) / add wrap-   
		jmp	give_valid_o_result		/around constant   
masked_underflow:   
		movb	result_sign(%bp),%al		/special rounding_   
		call	directed_round_test   
		jnz	do_grad_underflow		/ if not, no error   
		call	set_u_error			/ else, set error flag  
do_grad_underflow:   
		mov	$offset_result,%di		/do gradual underflow   
		mov	$0x0001,%ax			/minimum expon is 0001 
		call	gradual_underflow   
		call	get_precision   
		movb	true,%al   
		mov	$offset_result,%di		/do second round   
		call	round   
		xor	%ax,%ax   
		mov	%ax,result_expon(%bp)		/set exponent to zero   
		mov	$[result_word_frac+2],%di   
		call	test_4w				/ if fraction nonzero
		movb	inv,result_tag(%bp)   
		jnz	set_inv_tag			/ tag as invalid   
		movb	special,result_tag(%bp)	/ else, tag as zero   
set_inv_tag:   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			arith:   
/   
/	function:   
/		main: for implementation of 80287 add, subtract
/		multiply, and divide instructions   
/   
/	inputs:   
/		assumes operation type, operand(s), and unpacked   
/		status variables are set up   
/   
/	outputs   
/		result of operation in result   
/   
/	data accessed:   
/		- operation_type		offset_operand1   
/		- sign1				tag1   
/		- expon1			offset_operand2   
/		- sign2				tag2   
/		- expon2			offset_result   
/		- result_sign			result_tag   
/		- result_expon			result_word_frac   
/   
/	data changed:   
/		- result   
/   
/	procedures:   
/		put_indefinite		affine_infinity_   
/		put_max_valid		move_op_to_result   
/		overflow_response	underflow_response   
/		put_max_nan		divid   
/		set_up_indefinite	round   
/		addition_normalized	put_result   
/		pop_free		subadd   
/		mult			test_4w   
/		i_masked_		d_error_   
/		d_masked_		i_error_   
/		get_precision		set_i_masked_   
/		set_z_masked_		set_d_masked_   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

arith:

		jz	catch_denorm_error		/branch if no error   
		call	i_masked_			/stack error occurred   
		jz	go_home				/if unmasked, return   
		mov	$offset_result,%di		/otherwise, result =   
		call	set_up_indefinite		/ indefinite   
		jmp	finish_up   
catch_denorm_error:   
		call	d_error_			/ is there a denormal_  
		jz	weed_out_special_cases		/ no, don't sweat it   
		call	d_masked_			/ yes, is it unmasked_  
		jz	go_home				/ yes, abort arith   
weed_out_special_cases:   
		movb	tag1(%bp),%al			/ both operands valid_  
		orb	tag2(%bp),%al   
		jnz	special_cases			/ no, special case   
handle_non_special_cases:   
		movb	operation_type(%bp),%bl	/ to do the operation   
		subb	$add_op,%bl			/ call sub/add/mul/div 
		xorb	%bh,%bh
		shl	$1,%bx   
		call	*%cs:arith_table(%bx)   
		call	i_error_			/invalid if attempted   
		jz	no_invalid_error		/ division by denormal 
		jmp	invalid_error_detected   
no_invalid_error:   
		push	%ax				/al true if uflow poss 
		mov	$offset_result,%di		/round result   
		call	get_precision			/(indicate first, not   
		movb	false,%al			/ second, round)   
		call	round   
		mov	$offset_result,%di		/re-normalize   
		movb	$4,%ah				/(num words in frac=4)  
		call	addition_normalize		/(%al contains overflow
		mov	result_expon(%bp),%ax		/indication from round) 
		cmp	$0x7ffe,%ax			/over- or underflow if  
		ja	over_underflow			/expon>0x7ffe   
		call	give_valid_o_result		/ set tag to valid   
		and	%ax,%ax				/over- or underflow if  
		jnz	pop_finish_up			/expon=0 and frac <> 0 
		mov	$[result_word_frac+2],%di	/expon=0   
		call	test_4w   
		jnz	over_underflow   
		movb	special,result_tag(%bp)	/result is truly zero   
pop_finish_up:   
		pop	%ax				/ get rid of uflow flag 
finish_up:   
		mov	$offset_result,%di   
		mov	offset_result_rec,%si   
		jmp	put_arith_result   
over_underflow:   
		pop	%ax				/ retrieve uflow flag  
		cmp	$0x7fff,result_expon(%bp)   
		je	overflow_happened   
		cmpb	true,%al			/(%al is true iff uflow 
		je	underflow_happened		/ was possible)   
overflow_happened:   
		call	overflow_response		/ else, overflow   
		jmp	finish_up   
go_home:   
		ret   
underflow_happened:   
		call	underflow_response   
		jmp	finish_up   
special_cases:   
		mov	$offset_operand1,%di		/test op1 for   
		call	test_denormd			/ being denormalized   
		mov	$offset_operand2,%di		/test op2 for   
		call	test_denormd			/ being denormalized   
		cmpb	inv,tag1(%bp)			/separate invalid oprnd 
		je	invalid_operand			/ cases from zero
		cmpb	inv,tag2(%bp)			/ infinity, and denorm  
		jne	ops_zero_infinity_valid   
invalid_operand:   
		call	set_i_masked_			/invalid operand sets   
		jz	go_home				/ i-error, if unmasked  
		mov	offset_result_rec,%si		/result=max(nan1,nan2)  
put_max_nan:   
		movb	sign1(%bp),%al			/ set result to larger 
		movb	sign2(%bp),%ah			/ magnitude result   
		push	%ax				/ save signs   
		cmpb	infinty,tag1(%bp)		/if an operand is inf
		jne	op2_infinity_fix		/ clear msb for compare 
		andb	$0x7f,msb_frac1(%bp)   
op2_infinity_fix:   
		cmpb	infinty,tag2(%bp)   
		jne	subtract_fracs   
		andb	$0x7f,msb_frac2(%bp)   
subtract_fracs:   
		movb	positive,sign1(%bp)		/subtract the absolute  
		movb	positive,sign2(%bp)		/ values of the two ops 
		movb	$sub_op,operation_type(%bp)   
		push	%si   
		call	subadd   
		pop	%si   
		pop	%ax   
		movb	%al,sign1(%bp)			/restore sign1 and give 
		mov	$offset_operand1,%di		/ operand1 as answer   
		cmpb	positive,result_sign(%bp)	/ if difference is '+'  
		je	put_arith_result   
		movb	%ah,sign2(%bp)			/restore sign2 and give 
		mov	$offset_operand2,%di		/ operand2 as answer   
put_arith_result:   
		call	put_result   
		jmp	pop_free   
ops_zero_infinity_valid:   
		mov	$0x0303,%bx	/form index to special operation table  
		andb	tag2(%bp),%bl	/bx=2*(3*masked_tag1+masked_tag2)
		andb	tag1(%bp),%bh	/where masked_tag = 0 for valid
		addb	%bh,%bl		/                   1 for zero
		addb	%bh,%bl		/                   2 for infinity   
		addb	%bh,%bl   
		xorb	%bh,%bh   
		shl	$1,%bx   
		mov	$0x12fb,%ax			/ 18 byte/table -add_op
		addb	operation_type(%bp),%al	/ %al = normalized type 
		mulb	%ah				/ %ax = operation   
		add	%ax,%bx				/ %bx = case   
		jmp	*%cs:special_table(%bx)		/ jump to special case 
abort:   
		pop	%ax				/ discard first return 
exit_arith:   
		ret   
first_operand:   
		mov	$offset_operand1,%si		/give first operand as 
		jmp	set_result_to_operand	/ result   
neg_second_operand:   
		notb	sign2(%bp)			/negate second operand  
second_operand:   
		mov	$offset_operand2,%si		/give second operand   
set_result_to_operand:   
		call	move_op_to_result   
		jmp	go_to_finish_up   
divide_into_zero:   
		xor	%ax,%ax   
		mov	$[word_frac2+2],%di	/if frac2 = 0, invalid   
		call	test_4w				/ else, zero and xor   
		jz	invalid_error_detected		/ signs as the result   
exor_signed_zero:   
		mov	$zero_pattern,%si   	/ NEED OFF
		call	set_constant_result   
		movb	special,result_tag(%bp)   
		jmp	set_exor_sign   
divide_by_zero:   
		call	set_z_masked_			/set zero divide error  
		jz	exit_arith			/ if masked, return   
exor_signed_infinity:   
		call	put_infinity			/ else, give infinity   
set_exor_sign:   
		movb	positive,result_sign(%bp)	/set sign to exclusive  
		movb	sign1(%bp),%ah			/ or of operand signs   
		cmpb	sign2(%bp),%ah   
		je	go_to_finish_up   
		notb	result_sign(%bp)   
go_to_finish_up:   
		jmp	finish_up   
add_sub_infinities:   
		movb	sign2(%bp),%ah			/add or sub magnitude_  
		cmpb	$sub_op,operation_type(%bp)	/add mag if add_op and  
		jne	add_or_sub_mag_			/signs same, else sub   
		notb	%ah				/comp sign2 if sub   
add_or_sub_mag_:   
		cmpb	sign1(%bp),%ah   
		jne	invalid_error_detected		/ invalid if projective 
		call	affine_infinity_		/add magnitude inf's   
		jnz	first_operand			/first op res if affine 
invalid_error_detected:   
		call	set_i_masked_			/invalid op attempted   
		jz	exit_arith   
		call	put_indefinite			/if masked, indefinite  
		jmp	go_to_finish_up			/otherwise, just return
test_denormd:   
		cmpb	denormd,tag(%bp,%di)
		jne	exit_arith			/ return if not denormd
		call	set_d_masked_			/set denorm error   
		jz	abort				/ if unmasked, abort   
		mov	$0x0001,expon(%bp,%di)         /if masked, expon=1   
		movb	valid,tag(%bp,%di)       	/ tag=valid   
		ret   

