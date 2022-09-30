		.file "lipsq.s"


/
/   
/			l i p s q . m o d   
/			=================   
/   
/   ===============================================================   
/               intel corporation proprietary information   
/    this software  is  supplied  under  the  terms  of  a  license   
/    agreement  or non-disclosure agreement  with intel corporation   
/    and  may not be copied nor disclosed except in accordance with   
/    the terms of that agreement.                                     
/   ===============================================================   
/
/         @(#)lipsq.s	1.1 - 85/09/06
/   
/	functions:   
/		implements the loading of the constants:   
/		 one, log base 2 of ten, log base 2 of e, pi
/		 log base 10 of 2, log base e of 2, and zero.   
/		implements the 80287 square root instruction.   
/		implements the 80287 integer part instruction.   
/   
/	.globl:   
/		load_con		sqrt			intpt   
/   
/

#include   "e80287.h"   

/   
/	temp real floating point numbers for push constant instructions   
/   

treal_table:   
		.value	0x00000,0x00000,0x00000,0x08000,0x03fff	/treal_one   
		.value	0x08afe,0x0cd1b,0x0784b,0x0d49a,0x04000	/treal_l2t   
		.value	0x0f0bc,0x05c17,0x03b29,0x0b8aa,0x03fff	/treal_l2e   
		.value	0x0c235,0x02168,0x0daa2,0x0c90f,0x04000	/treal_pi   
		.value	0x0f799,0x0fbcf,0x09a84,0x09a20,0x03ffd	/treal_lg2   
		.value	0x079ac,0x0d1cf,0x017f7,0x0b172,0x03ffe	/treal_ln2   
		.value	0x00000,0x00000,0x00000,0x00000,0x00000	/treal_0   

		.text

      		.globl	load_con
		.globl	intpt
		.globl	sqrt   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			load_con   
/			""""""   
/	function:   
/		implements the load constants instructions.   
/   
/	inputs:   
/		of fpn pointer in %bx register.   
/   
/	outputs:   
/		constant value on top of stack   
/   
/	data accessed:   
/		- offset_result   
/   
/	data changed:   
/		- mem_operand_pointer   
/   
/	procedures:   
/		getx   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/  all load constant instructions use the same entry point.   
/   
/   
/   the contents of %bx is used as an index to treal_table.   
/   getx unpacks the extended floating point number and put_result   
/   pushes the value to the top of the 80287 stack.   
   
load_con:
		movb	$0x05,%al			/treal_table ptr = %cs: 
		mulb	%bl				/ index*5 - (5 * 2)   
		add	$[treal_table-[load_1_op\*10]],%ax /  (load_1_op)   
		mov	%ax,mem_operand_pointer(%bp) / store treal ptr   
		mov	%cs,[mem_operand_pointer+2](%bp)   
		mov	$offset_result,%di		/di = destination ptr   
		call	getx				/unpack the extended fp 
		jmp	sqrt_give_result		/push to top of stack   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			intpt:   
/			"""""   
/	function:   
/		implements 80287 integer part instruction   
/   
/	inputs:   
/   
/	outputs:   
/   
/	data accessed:   
/		- offset_result			offset_operand1   
/		- tag1				expon1   
/		- word_frac1   
/   
/	data changed:   
/		- tag1				expon1   
/		- result   
/   
/	procedures:   
/		get_operand			gradual_underflow   
/		round				subtraction_normalize   
/		put_si_result			test_4w   
/		i_masked_			set_i_masked_   
/		set_d_masked_   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
intpt:
		movb	true,%ah		/ load constant   
                jz      separate_cases		/ branch if no stack error   
                call	i_masked_   
                jnz	give_op1		/masked stack error, indef   
intpt_done:   
		ret   
separate_cases:   
		movb	tag1(%bp),%al		/ load op1 tag   
		cmpb	valid,%al   
		je	valid_case   
		cmpb	denormd,%al   
		je	denormalized_operand   
		cmpb	inv,%al   
		jne	give_op1		/infinity or 0, same answer   
		call	set_i_masked_   
		jnz	give_op1   
		ret   
denormalized_operand:   
                call	set_d_masked_   
                jz	intpt_done   
		mov	$0x0001,expon1(%bp)	/if masked d-error, make valid  
		movb	valid,tag1(%bp)   
valid_case:   
		mov	$0x403e,%ax   
		cmp	%ax,expon1(%bp)	/if expon >=63, then number   
		jge	detect_zero		/ is already an integer   
		mov	$offset_operand1,%di	/gradual uflow until expon=63   
		push	%di   
		call	gradual_underflow   
		pop	%di			/round to precision 64   
		movb	prec64,%dl   
		movb	false,%al   
		call	round   
detect_zero:   
		xor	%ax,%ax   
		mov	$[word_frac1+2],%di /if fraction = 0, result = 0   
		call	test_4w   
		jz	zero_result   
		mov	$offset_operand1,%di	/normalize   
		call	subtraction_normalize   
		jmp	give_op1   
zero_result:   
		mov	%ax,expon1(%bp)		/set result to true zero
		movb	special,tag1(%bp)   
give_op1:   
		mov	$offset_operand1,%di   
		jmp	put_si_result   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			sqrtx:   
/	function:   
/		 fractional square root routine.   
/   
/	inputs:   
/		assumes valid, non-zero,positive, normalized   
/		fraction is in frac1.   
/   
/	outputs   
/		leaves fractional square root in result.   
/   
/	data accessed:   
/		- offset_operand1		lsb_frac1   
/		- offset_operand2		word_frac2   
/		- offset_result			lsb_frac2   
/		- result_word_frac		lsb_result   
/		- msb_frac1   
/   
/	data changed:   
/		- word_frac2			lsb_frac2   
/		- result_word_frac		lsb_result   
/   
/	procedures:   
/		left_shift_result_cl		left_shift_frac1_cl   
/		left_shift_frac2_cl		clear_5w   
/		set_5w				add_to_frac_2   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
sqrtx:
                mov	$result_word_frac,%di /during this computation
		call	clear_5w		/the lsb of the result will   
		mov	$word_frac2,%di	/ hold g and s, and the msb   
		call	set_5w			/ will hold carry-out bits   
		stc				/cf holds the quotient bit   
		push	$65			/ iterate 65 times   
		pushf				/ stack the quotient bit   
		jmp	enter_sqrt_loop   
sqrt_loop:   
		push	%cx   
		pushf				/ stack the q_bit   
		movb	$1,%cl   
		call	left_shift_result_cl	/shift result left one bit   
		popf				/ inject the new q_bit into   
		pushf				/ the least significant byte   
		adcb	%cl,%cl   
 		orb	%cl,[lsb_result+1](%bp)
		movb	$2,%cl			/ into the lsb   
		call	left_shift_frac2_cl	/shift frac2 left 2 bits   
enter_sqrt_loop:   
		movb	msb_frac1(%bp),%al	/g and s bits of frac2 <--   
		andb	$0xc0,%al		/ top 2 bits of frac1   
		movb	%al,lsb_frac2(%bp)   
		movb	$2,%cl			/ shift frac1 left 2   
		call	left_shift_frac1_cl   
		movb	$0xc0,%al   
                movb    [lsb_result+1](%bp),%ah   
		popf				/test q_bit   
		jc	q_bit_set   
		call	add_to_frac2		/frac2.gs <-- frac2.gs +   
		jmp	set_q_bit		/result.11   
q_bit_set:   
                notb     %ah			/frac2.gs <-- frac2.gs +   
                add     %ax,word_frac2(%bp)	/not(result).11   
		mov	[result_word_frac+2](%bp),%ax   
		not	%ax   
		adc	%ax,[word_frac2+2](%bp)   
		mov	[result_word_frac+4](%bp),%ax   
		not	%ax   
		adc	%ax,[word_frac2+4](%bp)   
		mov	[result_word_frac+6](%bp),%ax   
		not	%ax   
		adc	%ax,[word_frac2+6](%bp)   
		mov	[result_word_frac+8](%bp),%ax   
		not	%ax   
		adc	%ax,[word_frac2+8](%bp)   
set_q_bit:   
		pop	%cx			/ reload loop count   
		loop	sqrt_loop		/ loop until done   
		rcrb	$1,%cl			/ set g bit of result to q_bit  
		orb	%cl,lsb_result(%bp)
                mov     result_word_frac(%bp),%ax /frac2 <- frac2 +   
		incb	%ah			/ result+1   
		call	add_to_frac2   
		mov	$[word_frac2+2],%di /if frac2 = 0 then   
		xor	%ax,%ax			/s_bit of result = 0
		call	test_4w			/otherwise 1.   
                orb     [lsb_frac2+1](%bp),%al   
		jz	left_adjust_result   
		orb	$0x40,lsb_result(%bp)
left_adjust_result:   
                movb    $8,%cl			/shift result left 8 bits   
                jmp	left_shift_result_cl	/ to eliminate carry   
add_to_frac2:   
                mov	$result_word_frac,%si / %si points to the addend   
		mov	$word_frac2,%di	/ %di points to the result   
		jmp	add_to_frac_2		/ add result frac to frac2   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			sqrt:   
/   
/	function:   
/		implements the 80287 sqrt instruction   
/   
/	inputs:   
/		assume op1 is set up   
/   
/	outputs:   
/		result   
/   
/	data accessed:   
/		- result_record_offset		result_expon   
/		- offset_operand1		tag1   
/		- sign1				expon1   
/		- msb_frac1			offset_operand1   
/		- offset_result			result_sign   
/		- result_tag   
/   
/	data changed:   
/		- expon1			result_sign   
/		- result_tag			result_expon   
/   
/	procedures:   
/		set_up_indefinite		sticky_right_shift   
/		sqrtx				round   
/		addition_normalize		affine_infinity_   
/		set_i_masked_			get_precision   
/		set_d_masked_			put_si_result   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
sqrt:
		jz	sqrt_cont		/ if stack error, sqrt done   
sqrt_done:   
		ret   
sqrt_cont:   
		movb	tag1(%bp),%al		/ load tag for op1   
		cmpb	valid,%al   
		je	sqrt_valid_case   
		cmpb	denormd,%al		/if op1 denormalized, then give 
		je	d_error			/d_error   
		cmpb	special,%al		/if op1 = 0, then give 0 as th
		je	put_op1			/result   
		cmpb	inv,%al		/if op1 inv, then give i_error  
		je	i_operand   
		cmpb	positive,sign1(%bp)	/infinity case   
		jne	i_error			/ -infinity is invalid   
		call	affine_infinity_	/if +infinity and affine
		jnz	put_op1			/ give infinity, else invalid   
i_error:   
		call	set_i_masked_		/denormalized, unnormalized
		jz	sqrt_done		/ negative, or proj +infinity   
		mov	$offset_result,%di   
		call	set_up_indefinite	/if masked i_error, then give   
		jmp	sqrt_give_result	/indefinite   
i_operand:   
                call	set_i_masked_   
                jz	sqrt_done   
put_op1:					/if op1 = zero, nan, or +inf
		jmp	give_op1		/ then give op1 as the result   
d_error:   
		call	set_d_masked_		/op1 is denormalized   
		jz	sqrt_done   
		mov	$0x0001,expon1(%bp)	/if d_error masked, make valid  
sqrt_valid_case:   
		cmpb	positive,sign1(%bp) 	/i_error if negative or unnorm  
		jne	i_error   
		testb	$0x80,msb_frac1(%bp)
		jz	i_error   
		sub	$exponent_bias,expon1(%bp)   
		test	$0x0001,expon1(%bp)
		jz	even_expon   
		dec	expon1(%bp)		/if expon1 odd, then expon1 <-- 
		jmp	halve_exponent	/ expon1-1   
even_expon:   
		movb	$1,%cl			/if expon1 even, then shift   
		xorb	%al,%al			/ frac1 right one bit   
		mov	$offset_operand1,%di   
		call	sticky_right_shift   
halve_exponent:   
		mov	expon1(%bp),%ax   
		sar	$1,%ax   
		add	$exponent_bias,%ax   
		mov	%ax,result_expon(%bp)   
		call	sqrtx			/ calculate fraction   
		xor	%ax,%ax			/ not second rounding   
		mov	%ax,result_sign(%bp) / set sign and tag   
		mov	$offset_result,%di	/ round result   
		call	get_precision   
		call	round   
		mov	$offset_result,%di   
		call	addition_normalize	/(possible renormalize)   
sqrt_give_result:   
		mov	$offset_result,%di   
		jmp	put_si_result   
