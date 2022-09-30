	.file	"divmul.s"

	.ident	"@(#)divmul.s	1.2"


/   
/                       m u l d i v . m o d   
/                       ===================   
/   
/       ===============================================================   
/               intel corporation proprietary information   
/    this software  is  supplied  under  the  terms  of  a  license   
/    agreement  or non-disclosure agreement  with intel corporation   
/    and  may not be copied nor disclosed except in accordance with   
/    the terms of that agreement.   
/       ===============================================================   
/   
/       function:   
/               performs floating-point divide of unpacked   
/               non-zero, valid numbers.   
/               performs floating-point multiply of unpacked   
/               non-zero, valid numbers.   
/   
/: procedures:
/               mulx                    mult   
/               divx                    accel_divx              divid   
/   
/
/   
/...september 16, 1983...   
/   
#include   "e80287.h"
/   
/   
		.globl  accel_divx
		.globl  divx
		.globl  log_divx
		.globl  divid
		.globl  mulx
		.globl  mult

/   
quotient_length:         .byte      28,36,57,68  / incremented and changed to
log_quotient_length:     .byte      28,36,60,68  / a byte table on 12/02/82.
						/ for unknown reasons, divx   
						/ doesn't work with 53-bit   
						/ precision from log function   
low_quotient_byte:       .byte      [offset_result+6],[offset_result+4]
			.byte      [offset_result+2],[offset_result+1]

		.text

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/                       accel_divx:   
/                       """"""""""   
/       function:   
/                fractional divide. result_frac <-- frac1/frac2.   
/   
/       inputs:   
/               frac2 is assumed to be normalized and non-zero.
/   
/       outputs:   
/               the sticky bit is set for result_frac   
/               the remainder for ((frac1)/2)mod(frac2) is left in frac1   
/   
/       data accessed:   
/              -word_frac1            offset_operand2   
/              -word_frac2            result_word_frac   
/              -lsb_result   
/   
/       data changed:   
/              -word_frac1            result_word_frac   
/              -lsb_result   
/   
/: lcalled:   
/               sticky_right_shift              get_precision   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
divisor_words:       .byte   2, 3, 4, 4
quotient_words:      .byte   2, 3, 4, 5
   
accel_divx:   
clr_quo:   
		mov     $result_word_frac,%di
		call    clear_5w                /clears %ax as well.
		mov     $offset_operand1,%di    /shift frac1 (here, the dividend)
		movb    $1,%cl                  /right by $1 bit
		call    sticky_right_shift   
		call    get_precision   
		movb    %dl,%bl
		xorb    %bh,%bh
		mov     $quotient_words,%di     / (%di) has offset quotient_words' table
		movb    %cs:(%bx,%di),%cl       / q holds the number of words to be
		movb    %cl,q                   / produced for the quotient.
		xorb    %ch,%ch
		lea     [word_frac1+6](%bp),%si
		push    %ss:2(%si)
		mov     %sp,%dx                  / %dx holds the relative to %ss of the
						 / interim partial remainder's highest word.
		dec     %cx
push_dvdnd:   
		push    %ss:(%si)
		sub     $2,%si
		loop    push_dvdnd
		mov     %cx,%ax                 / %cx is 0 here.
		mov     $divisor_words,%di
		movb    %cs:(%bx,%di),%cl       / bit_ct  holds the number of words
		movb    %cl,bit_ct              / in the divisor.
push_zeroes:   
		push    %ax
		loop    push_zeroes
		mov     %dx,%bx                 / %bx holds the relative to %ss of the
						/ interim partial remainder's highest word.
		mov     %ax,%di                 / %di is index to words of quotient.
		movb    q,%cl
main_loop:   
		push    %cx
		mov     %ss:(%bx),%dx
		cmp     [word_frac2+8](%bp),%dx / is dvsr(1) = prem(j)_
		jne     do_divide
		mov     $0x0ffff,%ax            / %ax <-- apprx_q(j) = 2**16-1
		mov     %ss:-2(%bx),%dx         / %dx <-- prem(j+1)
		jmp     get_r_apprx_j
do_divide:   
		mov     %ss:-2(%bx),%ax         / (%dx,%ax) <--  prem(j)*2**16 + prem(j+1)
		div     [word_frac2+8](%bp)
		mov     %ax,q_apprx_j
		mov     %dx,r_apprx_j
		jmp     test_q_apprx_j
dec_q_apprx_j:   
		mov     q_apprx_j,%ax
		dec     %ax
		mov     r_apprx_j,%dx
get_r_apprx_j:   
		mov     %ax,q_apprx_j
		add     [word_frac2+8](%bp),%dx / r_apprx_j <-- %dx + dvsr(1)
		jc      adjst_prem
		mov     %dx,r_apprx_j
test_q_apprx_j:   
		mov     q_apprx_j,%ax
		mul     [word_frac2+6](%bp)      / (%dx,%ax) <-- dvsr(2)*q_apprx_j
		cmp     r_apprx_j,%dx
		jb      adjst_prem
		ja      dec_q_apprx_j
		cmp     %ss:-4(%bx),%ax          / is %ax > prem(j+2)_
		ja      dec_q_apprx_j
adjst_prem:   
		xor     %ax,%ax
		mov     %ax,carry
		movb    bit_ct,%al
		mov     %ax,%cx
		shl     $1,%ax
		neg     %ax
		mov     %ax,%si
prem_loop:   
		mov     sign2(%bp,%si),%ax
		mul     q_apprx_j
		sub     %ax,%ss:(%bx,%si)
		jnc     sbtrct_carry
		inc     %dx
sbtrct_carry:   
		mov     carry,%ax
		sub     %ax,%ss:(%bx,%si)
		jnc     next_carry
		inc     %dx
next_carry:   
		mov     %dx,carry
		add     $2,%si
		loop    prem_loop
   
		sub     %dx,%ss:(%bx,%si)   / here, %si = 0.
		jnc     next_j
		dec     q_apprx_j
		movb    bit_ct,%cl
		mov     %cx,%ax
		shl     $1,%ax
		neg     %ax
		mov     %ax,%si
		clc   
fix_prem_loop:   
		mov     sign2(%bp,%si),%ax
		adc     %ax,%ss:(%bx,%si)
		add     $2,%si
		loop    fix_prem_loop
		adc     %cx,%ss:(%bx,%si)
   
next_j:   
		mov     q_apprx_j,%ax
		mov     %ax,[result_word_frac+8](%bp,%di)
		pop     %cx
		dec     %cx
		jz      get_sticky_bit
		sub     $2,%bx
		sub     $2,%di
		jmp     main_loop
   
get_sticky_bit:   
		mov     %cx,%ax
		mov     %ax,%si
		movb    bit_ct,%cl
sticky_loop:   
		or      %ss:-2(%bx,%si),%ax
		sub     $2,%si
		loop    sticky_loop
		orb     %al,%ah
		movb    %ah,lsb_result(%bp)
		mov     %cx,%si
		movb    bit_ct,%cl
   
stor_rmndr:   
		mov     %ss:-2(%bx,%si),%ax
		mov     %ax,[word_frac1+8](%bp,%si)
		sub     $2,%si
		loop    stor_rmndr
   
		movb    q,%cl
		addb    bit_ct,%cl
		shl     $1,%cx
		add     %cx,%sp                  /   restore stack
   
		ret   

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
		push    $quotient_length               / NEED OFFset normal entry ptr
		jmp	clear_quotient   
log_divx:   
		push    $log_quotient_length            / NEED OFFset log entry ptr
clear_quotient:   
		mov     $result_word_frac,%di           / NEED OFF
		call	clear_5w   
		mov	$offset_operand2,%di		/shift frac2 (divisor)  
		movb	$1,%cl				/right by 1 bit   
		call	sticky_right_shift   
		call	get_precision   
		movb	%dl,%bl   
		xorb	%bh,%bh   
		pop     %di                             / retrieve quotient length table
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
		call    accel_divx
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

		mov     expon1(%bp),%ax         /form doubly-biased exponent
		add     expon2(%bp),%ax         /if high bit set, underflow
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
		mov     4(%bp,%si),%ax          /if low 3 words = zero
		or      2(%bp,%si),%ax          /then single precision
		or	(%bp,%si),%ax   
		jnz	do_frac_multiply   
		add	%cx,%si			/frac1 is s. p.
		add	%cx,%bx			/so adjust pointers   
do_frac_multiply:   
		push	%di			/ save frac2   
		mov     (%bp,%di),%di          / load multiplier
		xor	%cx,%cx			/clear %cx   
		mov     (%bp,%si),%ax          / multiply first word
		mul	%di   
		add	%ax,(%bx)		/add to partial product 
		adc	%dx,%cx			/ %cx initially 0   
		mov     2(%bp,%si),%ax          / multiply second word
		mul	%di   
		add	%cx,%ax   
		adc	$0,%dx   
		xor	%cx,%cx   
		add	%ax,2(%bx)		/add to partial product   
		adc	%dx,%cx   
		cmp	$offset_operand1,%si   
		je	mult_third   
		mov	%cx,4(%bx)		/multiplicand is s. p.
		jmp     end_of_mul_loop         /so add in final word
mult_third:   
		mov     4(%bp,%si),%ax          / multiply third word
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
		mov     8(%bp,%si),%ax          / multiply fifth word
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
