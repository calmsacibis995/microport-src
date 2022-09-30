		 .file "divmul.s"


/
/   
/			m u l d i v . m o d    
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
/         @(#)divmul.s	1.1 - 85/09/06
/   
/	function:   
/		preforms floating-point divide of unpacked   
/		non-zero, valid numbers.   
/		performs floating-point multiply of unpacked   
/		non-zero, valid numbers.   
/   
/	.globl:   
/		mulx			mult   
/		divx			divid   
/   

#include   "e80287.h"   

		.globl	divx
		.globl	log_divx
		.globl	divid
		.globl	mulx
		.globl	mult   

quotient_length:	.byte	28,36,57,68	/ incremented and changed to   
log_quotient_length:	.byte	28,36,60,68	/ a byte table on 12/02/82.   
						/ for unknown reasons, divx   
						/ doesn't work with 53-bit   
						/ precision from log function   
low_quotient_byte:	.byte	[offset_result+6],[offset_result+4]   
			.byte	[offset_result+2],[offset_result+1]   

		.text

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			divx:   
/			""""   
/	function:   
/		 fractional divide. result_frac <-- frac1/frac2.   
/   
/	inputs:   
/		frac2 is assumed to be normalized and non-zero.   
/   
/	outputs:   
/		the sticky bit is set for result_frac   
/		the remainder is left in frac1   
/   
/	data accessed:   
/		- word_frac1		offset_operand2   
/		- word_frac2		result_word_frac   
/		- lsb_result   
/   
/	data changed:   
/		- word_frac1		result_word_frac   
/		- lsb_result   
/   
/	procedures:   
/		sticky_right_shift		get_precision   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

divx:
		push	$quotient_length	/ NEED OFFset normal entry ptr  
		jmp	clear_quotient   
log_divx:   
		push	$log_quotient_length	/ NEED OFFset log entry ptr   
clear_quotient:   
		mov	$result_word_frac,%di	/ NEED OFF   
		call	clear_5w   
		mov	$offset_operand2,%di		/shift frac2 (divisor)  
		movb	$1,%cl				/right by 1 bit   
		call	sticky_right_shift   
		call	get_precision   
		movb	%dl,%bl   
		xorb	%bh,%bh   
		pop	%di		/ retrieve quotient length table   
		movb	%cs:(%bx,%di),%al		/ bit_ct holds   
		movb	%al,bit_ct			/ the quotient bits   
		movb	%cs:low_quotient_byte(%bx),%al	/[di] points to low-   
		cbw					/order byte of quotient 
		mov	%ax,%di   
                mov     word_frac1(%bp),%ax		/load dividend (frac1)  
		mov	[word_frac1+2](%bp),%bx		/into %ax,%cx,%dx,%di,%s
		mov	[word_frac1+4](%bp),%cx   
		mov	[word_frac1+6](%bp),%dx   
		mov	[word_frac1+8](%bp),%si   
		shr	$1,%si				/shift dvdnd rt $2 bits 
		rcr	$1,%dx   
		rcr	$1,%cx   
		rcr	$1,%bx   
		rcr	$1,%ax   
		shr	$1,%si   
		rcr	$1,%dx   
		rcr	$1,%cx   
		rcr	$1,%bx   
		rcr	$1,%ax   
		jmp	subtract_divisor   
frac_divide_loop:   
		shl	$1,%ax				/shift dvdnd (partial   
		rcl	$1,%bx				/remainder) lft one bit 
		rcl	$1,%cx   
		rcl	$1,%dx   
		rcl	$1,%si   
		jc	quotient_bit_0			/jump if cy from shift  
 		orb	$0x20,(%bp,%di)			/"shift in" a 1-bit   
subtract_divisor:   
		sub	word_frac2(%bp),%ax		/subtract divisor from  
		sbb	[word_frac2+2](%bp),%bx		/partial remainder   
		sbb	[word_frac2+4](%bp),%cx   
		sbb	[word_frac2+6](%bp),%dx   
		sbb	[word_frac2+8](%bp),%si   
		jmp	shift_quotient_left   
quotient_bit_0:   
		call	add_divisor			/ add divisor to prem   
shift_quotient_left:   
		call	shift_result_left   
		decb	bit_ct				/ decrement bit count   
		jnz	frac_divide_loop		/  next quotient bit   
		and	%si,%si				/ branch if remainder   
		js	adjust_remainder		/  is negative   
 		orb	$0x20,(%bp,%di)			/"shift in" last 1-bit 
		jmp	store_remainder   
adjust_remainder:   
		call	add_divisor			/add divisor to prem   
store_remainder:   
		mov	%ax,word_frac1(%bp)		/store partial rmndr   
		mov	%bx,[word_frac1+2](%bp)		/in frac1   
		mov	%cx,[word_frac1+4](%bp)   
		mov	%dx,[word_frac1+6](%bp)   
		mov	%si,[word_frac1+8](%bp)   
		or	%bx,%ax				/set sticky bits if   
		or	%cx,%ax				/partial rmndr non-zero 
		or	%dx,%ax   
		or	%si,%ax   
		orb	%al,%ah   
		movb	%ah,lsb_result(%bp)
		ret   
add_divisor:   
		add	word_frac2(%bp),%ax		/add divisor to partial 
		adc	[word_frac2+2](%bp),%bx		/remainder   
		adc	[word_frac2+4](%bp),%cx   
		adc	[word_frac2+6](%bp),%dx   
		adc	[word_frac2+8](%bp),%si   
		ret   
shift_result_left:   
		shl	$1,result_word_frac(%bp)   
		rcl	$1,[result_word_frac+2](%bp)   
		rcl	$1,[result_word_frac+4](%bp)   
		rcl	$1,[result_word_frac+6](%bp)   
		rcl	$1,[result_word_frac+8](%bp)   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			divid:   
/			""""""   
/	function:   
/		floating-point divide.   
/   
/	inputs:   
/		assumes operands are unpacked, non-zero, and valid   
/   
/	outputs:   
/		calculates unpacked result and returns with %al   
/		set to true if underflow is possible, false if   
/		overflow is possible.  the quotient is left in   
/		the result, and the remainder is left in frac1.   
/   
/	data accessed:   
/		- expon1			expon2   
/		- msb_frac2			offset_result   
/		- result_sign			result_expon   
/		- result_word_frac   
/   
/	data changed:   
/		- result_sign			result_expon   
/		- result_word_frac   
/   
/	procedures:   
/		divx				one_left_normalize   
/		set_i_error			get_precision   
/		left_shift_result_cl   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

divid:
		mov	expon1(%bp),%ax		/ stack underf possible 
		sub	expon2(%bp),%ax		/ flag (sign bit)   
		cwd   
		push	%dx   
		add	$exponent_bias,%ax	/form biased exponent   
		mov	%ax,result_expon(%bp)   
		movb	sign1(%bp),%al		/ sign = '+' if sign1 = sign2   
		xorb	sign2(%bp),%al   
		movb	%al,result_sign(%bp)   
		testb	$0x80,msb_frac2(%bp)	/if divisor unnormalized, give  
		jnz	fractional_divide	/ invalid error and return   
		pop	%ax			/ load underflow possible flag  
		jmp	set_i_error		/ set i-error and exit   
fractional_divide:   
		call	divx   
		call	get_precision   
		cmpb	prec53,%dl		/if double precision, shift   
		jne	norm_quotient		/ quotient left 3 bits   
		movb	$3,%cl   
		call	left_shift_result_cl   
norm_quotient:   
		mov	$offset_result,%di	/normalize by 1 left   
		pop	%ax			/ load flag (%ax unaffected)   
		jmp	one_left_normalize	/ shift, if unnormalized   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			mult:   
/			""""   
/	function:   
/		floating point multiply.   
/   
/	inputs:   
/		assumes operands are unpacked, valid, non-zero.   
/   
/	outputs:   
/		calculates unpacked result and returns with %al set   
/		to true if underflow is possible, false if overflow   
/		is possible.   
/   
/	data accessed:   
/		- sign1			expon1   
/		- sign2			expon2   
/		- offset_result		result_sign   
/		- result_expon   
/   
/	data changed:   
/		- result   
/   
/	procedures:   
/		mulx				norm_quotient   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

mult:

		mov	expon1(%bp),%ax	/form doubly-biased exponent   
		add	expon2(%bp),%ax	/if high bit set, underflow   
		cwd				/ not possible   
		not	%dx			/invert underflow possible flag 
		push	%dx			/ stack underflow possible flag 
		sub	$[exponent_bias-1],%ax	/form singly-biased exponent   
		mov	%ax,result_expon(%bp)   
		movb	sign1(%bp),%al		/ result sign = sign1 xor sign2 
		xorb	sign2(%bp),%al   
		movb	%al,result_sign(%bp)   
		call	mulx   
		jmp	norm_quotient		/ normalize product and exit   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			mulx:   
/			"""""   
/	function:   
/		fractional multiply. result_frac <-- frac1*frac2.   
/   
/	inputs:   
/		assumes the operands are unpacked, valid, non-zero.   
/   
/	outputs:   
/		product in result_frac (sticky indicator left in low bit)   
/   
/	data accessed:   
/		- word_frac1			offset_operand1   
/		- word_frac2			offset_operand2   
/		- extra_word_reg		lsb_result   
/		- offset_result   
/   
/	data changed:   
/		- extra_word_reg		offset_result   
/   
/	procedures:   
/		clear_5w			set_5w   
/		test_3w				test_5w   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

mulx:
		push	%ds			/ save a_msr   
		mov	$extra_word_reg,%di /NEED%ss:bp+di=>extra_word_reg   
		push	%ss   
		pop	%ds   
		lea	(%bp,%di),%bx		/ %ds:bx also => extra_word_reg 
		call	clear_5w		/ clear extra_word_reg   
		mov	$offset_result,%di	/ clear result_frac   
		call	set_5w   
		mov	$6,%cx			/ load s.p.   
		mov	$offset_operand2,%di	/di  to multiplier   
		call	test_3w			/if low 3 words <> zero
                jnz     examine_frac1		/ branch if non single   
		add	%cx,%di			/frac2 is s. p.
		add	%cx,%bx			/so adjust pointers   
examine_frac1:   
		mov	$offset_operand1,%si	/si points to multiplicand   
		mov	4(%bp,%si),%ax	/if low 3 words = zero
		or	2(%bp,%si),%ax	/then single precision   
		or	(%bp,%si),%ax   
		jnz	do_frac_multiply   
		add	%cx,%si			/frac1 is s. p.
		add	%cx,%bx			/so adjust pointers   
do_frac_multiply:   
		push	%di			/ save frac2   
		xor	%cx,%cx			/clear %cx   
		mov	0(%bp,%di),%di		/ load multiplier   
		mov	0(%bp,%si),%ax		/ multiply first word   
		mul	%di   
		add	%ax,(%bx)		/add to partial product 
		adc	%dx,%cx			/ %cx initially 0   
		mov	2(%bp,%si),%ax	/ multiply second word   
		mul	%di   
		add	%cx,%ax   
		adc	$0,%dx   
		xor	%cx,%cx   
		add	%ax,2(%bx)		/add to partial product   
		adc	%dx,%cx   
		cmp	$offset_operand1,%si   
		je	mult_third   
		mov	%cx,4(%bx)		/multiplicand is s. p.
		jmp	end_of_mul_loop	/so add in final word   
mult_third:   
		mov	4(%bp,%si),%ax	/ multiply third word   
		mul	%di   
		add	%cx,%ax   
		adc	$0,%dx   
		xor	%cx,%cx   
		add	%ax,4(%bx)		/add to partial product   
		adc	%dx,%cx   
                mov     6(%bp,%si),%ax		/ multiply fourth word   
                mul	%di   
                add     %cx,%ax   
                adc     $0,%dx
                xor     %cx,%cx   
                add     %ax,6(%bx)		/add to partial product   
                adc     %dx,%cx   
		mov	8(%bp,%si),%ax	/ multiply fifth word   
		mul	%di   
		add	%cx,%ax   
		adc	$0,%dx   
		add	%ax,8(%bx)		/add to partial product   
		adc	$0,%dx   
		mov	%dx,10(%bx)		/final word to product   
end_of_mul_loop:   
		pop	%di			/ reload frac2   
		inc	%bx			/adjust pointers for   
		inc	%bx			/next iteration   
		inc	%di   
		inc	%di   
		cmp	$[offset_operand2+10],%di   
		jne	do_frac_multiply   
		mov	$extra_word_reg,%di / NEED set sticky bit if any extra  
		call	test_5w			/ reg words are nonzero   
		jz	frac_mult_done   
		orb	$0x01,lsb_result(%bp)
frac_mult_done:   
		pop	%ds			/ restore a_msr   

		ret   

