		.file "store.s"

/
/   
/			s t o r e . m o d    
/			==================   
/   
/	=============================================================   
/               intel corporation proprietary information   
/       this software  is  supplied  under  the  terms  of  a  license
/       agreement  or non-disclosure agreement  with intel corporation
/       and  may not be copied nor disclosed except in accordance with
/       the terms of that agreement.
/	===============================================================   
/   
/         @(#)store.s	1.1 - 85/09/06
/
/	function:   
/		implements 80287 store, fix, and storex instructions.   
/   
/	.globl:   
/		store			fix16		move_op_to_op   
/   
/	internal:   
/		extended_store		bcd_store   
/		single_real_store	double_real_store   
/		store_valid		int64_store   
/		int16_store		int32_store   
/   
#include   "e80287.h"   

		.text

		.globl	store
		.globl	fix16
		.globl	move_op_to_op   

store_routine:	.value	single_real_store
		.value	double_real_store
		.value	int16_store   
		.value	int32_store
		.value	int64_store
		.value	extended_store   
		.value	int16_store
		.value	int16_store
		.value	bcd_store   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			store:   
/			""""""   
/	function:   
/		implements all store instructions .   
/   
/	inputs:   
/		operand1; format of result.   
/   
/	outputs:   
/		operand in "result_format" stored in memory   
/		destination pointed to by %es:di.   
/   
/	data accessed:   
/		- result_rec_offset	result_format   
/		- offset_operand1   
/   
/	data changed:   
/		- result record   
/   
/	procedures:   
/		set_up_indefinite	store routine (result format)   
/		i_masked_   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

store:
		jz	no_unmasked_error		/ branch if no error   
		call	i_masked_			/ is the error masked_  
		jz	return				/ no, abort the store   
no_unmasked_error:   
		cmpb	$reg,result_location(%bp)	/ yes, store the nan   
		jne	store_to_memory			/ branch if memop   
		call	put_op1_result			/ give opnd1 as result  
		jmp	pop_free   
store_to_memory:   
		movb	result_format(%bp),%bl		/call a different rtn  
		shl	$1,%bx				/ for each of the   
		jmp	*%cs:store_routine(%bx)		/ output formats   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			move_op_to_op:   
/   
/	function:   
/		moves operand record (op1,op2,result_op,result2_op) to   
/		another op.   
/   
/	inputs:   
/		ss:si points to the source, and   
/		ss:di points to the destination   
/	outputs:   
/		operand record in destination   
/   
/	data accessed:   
/		operand or result variables   
/   
/	data changed:   
/		operand or result variables   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
move_op_to_op:
		push	%ds			/ save a_msr   
		push	%ss   
		pop	%ds   
		push	%ss   
		pop	%es   
		mov	$[[sign2-sign1]\/2],%cx / NEED OFFset record length   
		rep	
		smov
		pop	%ds			/ reload a_msr   
return:   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			extended_store:   
/			"""""""""""""""   
/	function:   
/		stores into double extended format.   
/   
/	inputs:   
/		operand1   
/   
/	outputs:   
/		double extended operand in memory.   
/		stack popped if indicated   
/   
/	data accessed:   
/		- mem_operand_pointer		sign1   
/		- expon1			word_frac1   
/   
/	data changed:   
/   
/	procedures:   
/		pop_free   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
extended_store:	
		les	mem_operand_pointer(%bp),%di	/move frac to memory   
		push	%ds				/ save a_msr   
		push	%ss   
		pop	%ds				/ set up source pointer 
		lea	[word_frac1+2](%bp),%si   
		mov	$0x0004,%cx   
		rep	
		smov
		pop	%ds				/ reload a_msr   
		movb	sign1(%bp),%ah			/merge sign  exponent   
		and	$0x8000,%ax			/ and move to memory   
		or	expon1(%bp),%ax
		mov	%ax,%es:(%di)   
		jmp	pop_free			/ pop stack, if needed   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			bcd_store:   
/			""""""""""   
/	function:   
/		stores into bcd format   
/   
/	inputs:   
/		operand1   
/   
/	outputs:   
/		bcd operand in memory; stack popped   
/   
/	data accessed:   
/		- mem_operand_pointer		operation_type   
/		- offset_operand1		sign1   
/		- tag1				word_frac1   
/		- msb_frac1			offset_operand1   
/		- offset_operand2		sign2   
/		- expon2			word_frac2   
/		- offset_operand2		offset_result   
/		- result_expon			result_word_frac   
/		- msb_result			offset_result   
/   
/	data changed:   
/		- operation_type		word_frac1   
/		- msb_frac1			sign2   
/		- expon2			word_frac2   
/		- result_word_frac		msb_result   
/   
/	procedures:   
/		move_op_to_op			right_shift_frac1_cl   
/		right_shift_frac2_cl		right_shift_result_cl   
/		left_shift_frac1_cl		left_shift_result_cl   
/		divid				clear_5w   
/		pop_free			get_precision   
/		store_precision			set_i_masked_   
/		add_to_frac			move_constant   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

ten_18:		.value	0x0000
		.value 	0x0000
		.value	0x7640
		.value	0x6b3a
		.value	0xde0b 		/ frac for 10**18   

bcd_store:
		call	get_precision		/save old precision   
		push	%dx   
		movb	prec64,%al   
		call	store_precision		/and set to 64-bit   
		movb	tag1(%bp),%al   
		cmpb	special,%al		/ separate bcd cases   
		je	store_bcd_zero   
		testb	$0x80,msb_frac1(%bp)	/unnormalized operand   
		jz	invalid_bcd			/is invalid   
		cmpb	valid,%al   
		je	store_bcd_valid   
invalid_bcd:   
		call	set_i_masked_			/invalid error:  nan
		jz	go_to_bcd_store_done		/ unnorm, inf, denorm   
		movb	$16,%cl 			/invalid masked, shift  
		call	right_shift_frac1_cl		/frac1 right $16 bits   
		dec	%cx				/ %cx = $0x0ffff   
		mov	%cx,[word_frac1+8](%bp)		/set msw to all ones
		jmp	store_bcd_result		/and give as answer   
go_to_bcd_store_done:   
		jmp	bcd_store_done   
store_bcd_zero:   
		jmp	store_signed_bcd		/give +/- zero result   
store_bcd_valid:   
		movb	positive,sign2(%bp)		/set frac2 to 0.5   
		mov	$0x3ffe,expon2(%bp)
		mov	$word_frac2,%di   / NEED OFFSET
		call	clear_5w   
		mov	$0x8000,[word_frac2+8](%bp)
		movb	$add_op,%ah			/if op1 positive then   
		cmpb	%al,sign1(%bp)			/ result <- op1+0.5   
		je	add_point_5			/if op1 negative then   
		movb	$sub_op,%ah			/ result <-- op1-0.5   
add_point_5:   
		movb	%ah,operation_type(%bp)		/ set operation type   
		call	subadd   
		lea	result_word_frac(%bp),%si	/op1 <-- result   
		lea	word_frac1(%bp),%di   
		call	move_op_to_op   
		movb	positive,sign2(%bp)		/set frac2 to 10**18   
		mov	$0x403a,expon2(%bp)
		mov	$ten_18,%si   	/ NEED OFFSET
		mov	$offset_operand2,%di   
		call	move_constant   
		call	divid				/result <- op1 / 10**18 
		mov	$0x3fff,%cx			/shift_count <--    
		sub	result_expon(%bp),%cx		/-(unbiased exponent)   
		jle	invalid_bcd			/if count <= $0, invalid
		andb	%ch,%ch				/if shift_count > 0x0ff
		jz	shift_frac_for_bcd		/then set to $80   
		movb	$80,%cl   
shift_frac_for_bcd:   
		call	right_shift_result_cl		/shift result_frac   
		movb	$18,%cl				/loop_count <-- $18   
bcd_loop:   
		push	%cx   
		lea	result_word_frac(%bp),%si	/move result to op2   
		lea	word_frac2(%bp),%di   
		call	move_op_to_op   
		movb	$2,%cl   
		call	right_shift_frac2_cl		/shift frac2 rt $2 bits 
		mov	$offset_operand2,%si		/result_frac <--   
		mov	$offset_result,%di		/result_frac+frac2   
		call	add_to_frac   
		movb	$4,%cl   
		call	left_shift_frac1_cl		/shift frac1 left $4   
		movb	msb_result(%bp),%al		/low 4 bits of frac1 <- 
		shrb	$4,%al				/top 4 bits of result  
		orb	lsb_frac1(%bp),%al   
		movb	$3,%cl   
		call	left_shift_result_cl		/ shift result left $3  
		andb	$0x7f,msb_result(%bp)		/ and clear msb 
		pop	%cx   
		loop	bcd_loop   
store_signed_bcd:   
		movb	sign1(%bp),%al			/set sign in bcd number 
		andb	$0x80,%al   
		movb	%al,msb_frac1(%bp)
store_bcd_result:   
		les	mem_operand_pointer(%bp),%di	/move frac1 to memory   
		push	%ds				/ save a_msr   
		push	%ss   
		pop	%ds   
		lea	word_frac1(%bp),%si   
		mov	$5,%cx   
		rep	
		smov
		pop	%ds				/ reload a_msr   
		call	pop_free   
bcd_store_done:   
		pop	%ax   
		jmp	store_precision			/restore old setting   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			single_real_store:   
/			""""""""""""""""""   
/	function:   
/		implements store to single-precision real format   
/   
/	inputs:   
/		operand1   
/   
/	outputs:   
/		single_precision real in memory;   
/		stack popped if indicated.   
/   
/	data accessed:   
/		- mem_operand_pointer		offset_operand1   
/		- sign1				tag1   
/		- expon1			word_frac   
/		- msb_frac1   
/   
/	data changed:   
/   
/	procedures:   
/		round				store_valid   
/		addition_normalized		special_round_test   
/		directed_round_test		gradual_underflow   
/		pop_free			test_3w   
/		u_masked_			set_o_error   
/		o_masked_			set_p_error   
/		set_u_error			set_i_masked_   
/		store_denormd   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

single_real_store:
		push	$pop_free			/ return to pop_free   
		les	mem_operand_pointer(%bp),%di	/set %es:di to memop   
		mov	low_extended_expon_for_single,%si / 24-bit precision   
		movb	prec24,%dl   
		movb	tag1(%bp),%al			/ load op1 tag   
		cmpb	valid,%al   
		je	single_valid			/detect valid,infinity
		cmpb	infinty,%al			/denormd,invalid,and   
		je	single_invalid			/special cases   
		cmpb	denormd,%al   
		je	single_denormd   
		cmpb	inv,%al   
		jne	single_zero   
single_invalid:   
		mov	msb_frac1(%bp),%ax	/ %ah = sign1 al= msb1   
		or	$0x7f80,%ax 
		mov	%ax,%es:2(%di)   
		mov	[word_frac1+7](%bp),%ax   
		mov	%ax,%es:(%di)			/ don't set i-error   
		ret					/just store inv or inf  
single_valid:   
		mov	high_extended_expon_for_single,%di / load parameter   
		call	store_valid   
		les	mem_operand_pointer(%bp),%di	/ set %es:di to memop   
		jc	single_indefinite		/ c => store indefinite 
		jz	single_infinity			/ z => store infinity   
		js	single_max			/ s => store max   
single_store:   
		mov	[word_frac1+8](%bp),%ax		/ else store single   
		orb	[lsb_frac1+7](%bp),%al
		orb	%ah,%al   
		jz	single_zero			/ store signed zero   
		mov	$0x8001,%ax   
		andb	sign1(%bp),%al			/assemble single-prec   
 		addb	expon1(%bp),%ah   
		ror	$1,%ax   
		movb	$0x7f,%cl   
		andb	msb_frac1(%bp),%cl   
		orb	%cl,%al   
		mov	%ax,%es:2(%di)   
		mov	[word_frac1+7](%bp),%ax   
		mov	%ax,%es:(%di)   
		ret					/ pop stack and go home 
single_denormd:   
		call	store_denormd			/: denormal   
		les	mem_operand_pointer(%bp),%di	/ %es:di points to memop
		jmp	single_store			/  as a valid number   
single_zero:   
		xor	%ax,%ax				/ %es:di points to memop
		mov	%ax,%es:(%di)   
		movb	sign1(%bp),%ah   
		andb	$0x80,%ah   
		mov	%ax,%es:2(%di)   
		ret					/do needed pop or free  
single_indefinite:   
		mov	$0xffc0,%es:2(%di)		/ %es:di = memop ptr   
		mov	$0x0000,%es:(%di)   
		ret   
single_max:   
		mov	$0xffff,%es:(%di)		/ %es:di points to memop
		mov	$0x7f7f,%ax   
		orb	sign1(%bp),%ah   
		mov	%ax,%es:2(%di)   
		ret   
single_infinity:   
		call	single_zero   
		or	$0x7f80,%es:2(%di)		/ %es:di points to memop
		ret   
store_valid:   
		push	%di				/ stack high_expon   
		push	%si				/ stack low_expon   
		push	%dx				/ stack prec control   
		movb	false,%al			/ not second rounding   
		mov	$offset_operand1,%di		/ round to prec (%dl)   
		push	%di   
		call	round   
		pop	%di   
		call	addition_normalize		/ and renormalize   
		pop	%dx				/ unstack prec control  
		pop	%si				/ unstack low_expon   
		pop	%di				/ unstack high_expon   
		cmp	%si,expon1(%bp)			/ underflow_   
		jb	store_underflow			/ yes, < low_expon   
		je	decrement_exponent		/ no, = low_expon_   
		testb	$0x80,msb_frac1(%bp)		/ no, unnormalized_   
		jz	store_unnormal			/ yes, handle it   
		cmp	%di,expon1(%bp)			/ no, > high_expon_   
		jbe	do_store_valid			/ no, store number   
		call	o_masked_			/ is overflow masked_   
		jz	overflow_abort			/ no, error and abort   
		movb	true,%bl			/ yes, flag o-error   
		call	directed_round_test		/ directed rounding_   
		xorb	%al,%bl				/ no o-error, if true   
		call	set_p_error			/ set inexact error   
		movb	sign1(%bp),%al			/ get sign of op1   
		call	special_round_test		/ after round test, c=0 
		pushf					/  (z=0 s=1)/(z=1 s=0) 
		pushf					/ save twice for $2 pops
		andb	%bl,%bl				/ is o-error to be set_ 
		jz	pop_store_valid_done		/ no, exit store real   
overflow_abort:   
		call	set_o_error			/ overflow error   
pop_store_valid_done:   
		popf					/ reload flags (abort)  
		popf					/  (discard pop_free)   
store_valid_done:   
		ret					/ exit   
store_denormd:   
		mov	[word_frac1+8](%bp),%ax		/ load msw of fraction  
		and	$0x7fff,%ax			/ mask i-bit   
		mov	$[word_frac1+2],%di   
		call	test_3w			/ find +/-000080000000000000000 
		jz	store_underflow			/ if zero, exact result 
		call	set_p_error			/ else, set p-error   
store_underflow:   
		movb	true,%bl			/ flag possible u-error 
		call	u_masked_			/ is underflow masked_ 
		jz	underflow_abort			/ no, error and abort   
		call	directed_round_test		/ directed rounding_   
		xorb	%al,%bl				/ no u-error, if true   
		call	fix_u_error			/ fix the status word   
		mov	$offset_operand1,%di		/ load op1   
		mov	%si,%ax				/ load low exponent   
		push	%di				/ save op1   
		push	%dx				/ save prec control   
		call	gradual_underflow		/ do gradual underflow  
		pop	%dx				/ reload prec control   
		pop	%di				/ reload op1   
		movb	true,%al			/ perform second round  
		call	round   
decrement_exponent:   
		testb	$0x80,msb_frac1(%bp)		/ test msb of fraction  
		jnz	do_store_valid			/ don't decrement 80..0 
		dec	expon1(%bp)		/ else, decrement expon 
do_store_valid:   
		xor	%ax,%ax				/ c=0 s=0 z=0 for   
		inc	%ax				/  store valid exit   
		ret   
store_unnormal:   
		call	set_i_masked_			/ if invalid unmasked
		jz	pop_store_valid_done		/  abort the store   
		xor	%ax,%ax				/ else, c=1 s=0 z=0 for 
		stc					/  store indef exit   
		ret   
underflow_abort:   
		popf					/ discard first return 
		popf					/ dump pop_free   
fix_u_error:   
		andb	%bl,%bl				/ is u-error to be set_ 
		jz	store_valid_done		/ no, go back one level 
		jmp	set_u_error			/ yes, set u and return   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			double_real_store:   
/			""""""""""""""""""   
/	function:   
/		implements store to double-precision real format   
/   
/	inputs:   
/		operand1   
/   
/	outputs:   
/		double precision real in memory;   
/		stack popped if indicated.   
/   
/	data accessed:   
/		- mem_operand_pointer		offset_operand1   
/		- sign1				tag1   
/		- expon1			word_frac1   
/		- msb_frac1			offset_operand1   
/   
/	data changed:   
/		- sign1				word_frac1   
/   
/	procedures:   
/		test_3w				store_valid   
/		right_shift_frac1_cl		pop_free   
/		double_zero			double_fill   
/		store_denormd			double_frac_mem   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

double_real_store:

		push	$pop_free	/ NEED OFFSET return to pop_free   
		mov	low_extended_expon_for_double,%si / 53-bit precision   
		movb	prec53,%dl   
		movb	tag1(%bp),%al			/separate special cases 
		cmpb	valid,%al   
		je	double_valid   
		cmpb	denormd,%al   
		je	double_denormd   
		cmpb	infinty,%al   
		je	double_invalid			/ infinity is invalid   
		cmpb	inv,%al   
		jne	double_zero   
double_invalid:   
		call	double_frac_mem			/ store frac to memory  
		mov	msb_frac1(%bp),%ax	/store high word, sign
		or	$0x7ff0,%ax			/expon(all ones), and   
		mov	%ax,%es:(%di)			/high 4 bits of frac   
		ret					/ %es:di ==> exponent   
double_valid:   

		mov	high_extended_expon_for_double,%di   
		call	store_valid   
		jc	double_indefinite		/ c => store indef   
		jz	double_infinity			/ z => store infinity   
		js	double_max			/ s => store max   
double_store:   
		movb	[lsb_frac1+3](%bp),%al		/ else store double   
		cbw   
		mov	$[word_frac1+4],%di   
		call	test_3w   
		jz	double_zero			/ store signed zero   
double_frac_mem:   
		movb	$3,%cl				/start by shifting   
		call	right_shift_frac1_cl		/ fraction right $3 bits
		les	mem_operand_pointer(%bp),%di	/ %es:di points to memop
		push	%ds
		push	%ss
		pop	%ds				/ set up source pointer 
		lea	[word_frac1+3](%bp),%si		/move frac to memory   
		movb	$3,%cl   
		rep	
		smov
		pop	%ds				/ reload a_msr   
		mov	expon1(%bp),%ax		/fetch high-order word
		sub	$0x3c00,%ax			/with expon, sign, and  
		shl	$4,%ax				/top $4 bits of fraction
		andb	$0x0f,msb_frac1(%bp)
		orb	msb_frac1(%bp),%al
		andb	$0x80,sign1(%bp)
		orb	sign1(%bp),%ah   
		mov	%ax,%es:(%di)			/ %es:di points to expon
		ret	
double_denormd:
		call	store_denormd
		jmp	double_store
double_zero:
		xor	%ax,%ax
double_fill:
		les	mem_operand_pointer(%bp),%di
		mov	%ax,%es:(%di)
		mov	%ax,%es:2(%di)   
		mov	%ax,%es:4(%di)   
		movb	sign1(%bp),%ah   
		andb	$0x80,%ah   
double_expon:   
		mov	%ax,%es:6(%di)			/ store the exponent   
		ret   
double_indefinite:   
		call	double_zero			/ zero the memop   
		mov	$0xfff8,%ax			/ now fix the exponent 
		jmp	double_expon   
double_max:   
		mov	$0xffff,%ax			/ set fraction to ff..f 
		call	double_fill   
		mov	$0x7fef,%ax			/ set exponent to $7fef 
		orb	sign1(%bp),%ah			/ add sign bit   
		jmp	double_expon   
double_infinity:   
		call	double_zero			/ first clear the memop 
		or	$0x7ff0,%es:6(%di)		/ %es:di -> memop   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/		fix16:   
/   
/	function:   
/		 fixes valid number to a 16-bit integer.   
/   
/	inputs:   
/		 operand1   
/   
/	outputs:   
/		16-bit integer in word_frac1+8;   
/		overflow indication in zf (zf:1 => no overflow)   
/   
/	data accessed:   
/		- offset_operand1		sign1   
/		- expon1			word_frac1   
/		- offset_operand1   
/   
/	data changed:   
/		- word_frac1			extra_word_reg   
/   
/	procedures:   
/		sticky_right_shift		round   
/		careful_round			p_error_   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

fix16:   
		mov	high_int16_exponent,%cx		/calc shift amount   
		sub	expon1(%bp),%cx			/if expon1 > high expon 
		js	no_fix16_ovf			/ then overflow (zf=0) 
		cmp	max_int16_shift,%cx		/don't let shift amount 
		jbe	do_shift_16			/exceed max_int16_shift 
		movb	max_int16_shift,%cl   
do_shift_16:   
		movb	prec16,%dl   
		call	careful_round			/ careful p_error round 
		jz	no_fix16_ovf			/ round cy out (zf=1)   
		cmp	$0x8000,[word_frac1+8](%bp)   
		jb	check_sign_16			/no overflow if<$0x8000
		ja	no_fix16_ovf			/ else, overflow (zf=0) 
		cmpb	negative,sign1(%bp)		/oflow if = 0x8000 and  
		jnz	no_fix16_ovf			/ sign1 = pos (zf=0)   
check_sign_16:   
		movb	positive,%al			/ %al is overflow flag 
		cmpb	sign1(%bp),%al			/negate integer if   
		jz	no_fix16_ovf			/sign is neg (zf=1)   
		neg	[word_frac1+8](%bp)
		xorb	%al,%al				/ no overflow (zf=1)   
no_fix16_ovf:   
		ret   
careful_round:   
		push	%dx				/ save precision param  
		mov	$offset_operand1,%di		/shift frac1 by %cl   
		xorb	%al,%al   
		call	sticky_right_shift   
		pop	%dx				/ reload precision   
		mov	$offset_operand1,%di		/round to 16-bit prec   
		movb	false,%al   
 		movb	%al,extra_word_reg(%bp)		/ fix old p_error flag  
		call	p_error_			/ to reflect state of   
		jz	round_int			/ p_error before round  
		notb	extra_word_reg(%bp)		/ old p_error = true   
round_int:   
		call	round   
		cmpb	true,%al			/overflow if al=true   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			int16_store:   
/   
/	function:   
/		stores into 16-bit integer format   
/   
/	inputs:   
/		operand1   
/   
/	outputs:   
/		16-bit integer in memory operand location   
/   
/	data accessed:   
/		- mem_operand_pointer		tag1   
/		- word_frac1			msb_frac1   
/		- offset_operand1		extra_word_reg   
/   
/	data changed:   
/   
/	procedures:   
/		fix16				pop_free   
/		invalid_or_special		get_rnd_control   
/		set_i_error			i_masked_   
/		set_p_error			clear_p_error   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

int16_store:
/   
/	this whole section is an 80287 d-step kludge so that store   
/	will generate i+p errors for some weird boundary cases.   
/   
		xor	%dx,%dx				/ clr bndry oflow flag  
		cmp	high_int16_exponent-1,expon1(%bp) / boundary exponent 
		jne	not_boundary_16			/ no, won't raise i+p   
		mov	[word_frac1+8](%bp),%dx		/ yes, check msb   
		cmp	$0xfffe,%dx			/ round up case_   
		jne	not_boundary_16			/ no, %dx ==> bndry cond
		mov	[word_frac1+6](%bp),%ax		/ yes, must check lsb   
		or	[word_frac1+4](%bp),%ax   
		or	[word_frac1+2](%bp),%ax   
		jnz	not_boundary_16			/ branch if frac <> $0 
		mov	%ax,%dx			/ $0 lsb ==> no p-error 
not_boundary_16:   
		cmpb	valid,tag1(%bp)
		jne	invalid_or_special_16   
		testb	$0x80,msb_frac1(%bp)/if unnormalized
		jz	invalid_or_special_16		/then invalid error   
		push	%dx				/ save bndry oflow flag 
		call	fix16				/make op1 a 16-bit int  
		pop	%dx				/ load bndry oflow flag 
		jnz	invalid_or_special_16		/if zf=0, oflow on fix  
		mov	[word_frac1+8](%bp),%ax		/move integer to memory 
		les	mem_operand_pointer(%bp),%di	/set %es:di to location 
finish_16:   
		mov	%ax,%es:(%di)			/ store integer to mem 
		jmp	pop_free			/ pop stack and go home 
invalid_or_special_16:   
		call	invalid_or_special		/ fetch result in %ax   
		jmp	finish_16   
invalid_or_special:   
		xor	%bx,%bx   
		cmpb	special,tag1(%bp)	/ is operand tagged special_   
		je	exit_invalid_or_special / yes, load memop ptr and exit  
		call	set_i_error		/ no, set invalid error   
		mov	$0x8000,%bx		/ load inv constant into %bx   
		cmpb	%bl,sign1(%bp)		/ is sign positive_   
		jne	clear_inexact		/ no, must clear p-error   
		call	get_rnd_control		/ **************************   
		cmpb	rnd_down,%al		/ d-step kludge 06/28/81 *   
		je	clear_inexact		/ if i-error and round   *   
		cmpb	rnd_to_zero,%al		/ even or up and sign +, *   
		je	clear_inexact		/ let p-error stand      *   
		inc	%dx			/ an overflow boundary case_   
		jz	invalid_masked_		/ yes, p-error for rnd up/even  
		inc	%dx			/ special boundary case_   
		jnz	clear_inexact		/ no, clear the p-error   
		cmpb	rnd_up,%al		/ yes, round up_   
		je	invalid_masked_		/ yes, p-error for rnd up only  
clear_inexact:  
 		cmpb	true,extra_word_reg(%bp) / was p-error set_   
		je	invalid_masked_		/ yes, leave it set   
		call	clear_p_error		/ no, clear the new p-error   
invalid_masked_:   
		call	i_masked_		/ is invalid error masked_   
		jnz	exit_invalid_or_special	/ yes, store invalid constant   
		pop	%ax			/ no, throw away first address  
exit_invalid_or_special:   
		mov	%bx,%ax   
		les	mem_operand_pointer(%bp),%di	/ load pointer to memop 
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/		int32_store:   
/   
/	function:   
/		stores into 32-bit integer format   
/   
/	input:   
/		operand1   
/   
/	output:   
/		32-bit integer in location pointed to by %es:di.   
/   
/	data accessed:   
/		- mem_operand_pointer		offset_operand1   
/		- sign1				tag1   
/		- expon1			word_frac1   
/		- msb_frac1			offset_operand1   
/   
/	data changed:   
/		- word_frac1   
/   
/	procedures:   
/		pop_free			invalid_or_special   
/		careful_round   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

int32_store:
		xor	%dx,%dx		/ clear boundary overflow case flag   
		cmp	high_int32_exponent-1,expon1(%bp) /boundary exponent_ 
		jne	not_boundary_32			/ no, won't raise i+p   
		mov	[word_frac1+8](%bp),%ax		/ yes, load msw   
		inc	%ax				/ msw = 0x0ffff_   
		jnz	not_boundary_32			/ no, not a bndry case  
		mov	[word_frac1+6](%bp),%dx		/ yes, test second msw  
		cmp	$0xfffe,%dx			/ round up case_   
		jne	not_boundary_32			/ no, %dx ==> bndry cond
		or	[word_frac1+4](%bp),%ax		/ yes, must check lsb   
		or	[word_frac1+2](%bp),%ax   
		jnz	not_boundary_32			/ branch if frac <> $0  
		mov	%ax,%dx				/ $0 lsb ==> no p-error 
not_boundary_32:   
		cmpb	valid,tag1(%bp)
		jne	invalid_or_special_32   
		testb	$0x80,msb_frac1(%bp)	/if unnormalized, then  
		jz	invalid_or_special_32		/invalid error   
		mov	high_int32_exponent,%cx		/calc shift amount   
		sub	expon1(%bp),%cx		/if expon1 > high expon 
		js	invalid_or_special_32		/ then overflow   
		cmp	max_int32_shift,%cx		/shift amount shouldn't 
		jbe	do_shift_32			/exceed max_int32_shift 
		movb	max_int32_shift,%cl   
do_shift_32:   
		push	%dx   
		movb	prec32,%dl   
		call	careful_round   
		pop	%dx				/ overflow if zf=1 ->   
		jz	invalid_or_special_32		/carry-out from round   
		mov	[word_frac1+8](%bp),%ax		/ load ms word into %ax 
		xor	$0x8000,%ax   
		js	check_sign_32			/no overflow if integer 
		or	[word_frac1+6](%bp),%ax		/< 0x80000000   
		jnz	invalid_or_special_32   
		cmpb	%al,sign(%bp)			/overflow if integer = 
		je	invalid_or_special_32		/ $0x80000000, sign1 pos
check_sign_32:   
		cmpb	negative,sign1(%bp)	/negate integer if sign 
		jne	do_store_32			/negative   
		not	[word_frac1+6](%bp)
		add	$1,[word_frac1+6](%bp)
		not	[word_frac1+8](%bp)
		adc	$0,[word_frac1+8](%bp)
do_store_32:   
		les	mem_operand_pointer(%bp),%di	/set %es:di to location 
		mov	[word_frac1+6](%bp),%ax		/to store to   
		mov	%ax,%es:(%di)   
		mov	[word_frac1+8](%bp),%ax   
finish_32:   
		mov	%ax,%es:2(%di)   
		jmp	pop_free			/ pop stack and exit   
invalid_or_special_32:   
		call	invalid_or_special		/ get memop pointer   
		mov	$0,%es:(%di)		/  and ms word   
		jmp	finish_32		/ move $0 to memory   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""  
/			int64_store:   
/			""""""""""""   
/	function:   
/		stores into 64-bit integer format   
/   
/	input:   
/		operand1   
/   
/	output:   
/		64-bit integer in memory location pointed to by %es:di.   
/   
/	data accessed:   
/		- mem_operand_pointer		offset_operand1   
/		- sign1				tag1   
/		- expon1			word_frac1   
/		- msb_frac1			offset_operand1   
/   
/	data changed:   
/		- word_frac1   
/   
/	procedures:   
/		pop_free			invalid_or_special   
/		careful_round   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

int64_store:
		xor	%dx,%dx				/clr bndry oflow flag   
		cmp	high_int64_exponent-1,expon1(%bp) /boundary exponent_ 
		jne	not_boundary_64			/ no, won't raise i+p   
		mov	[word_frac1+8](%bp),%dx		/ yes, load msb   
		and	[word_frac1+6](%bp),%dx   
		and	[word_frac1+4](%bp),%dx   
		and	[word_frac1+2](%bp),%dx   
not_boundary_64:   
		cmpb	valid,tag1(%bp)
		jne	invalid_or_special_64   
		testb	$0x80,msb_frac1(%bp)		/if unnormalized
		jz	invalid_or_special_64		/then invalid error   
		mov	high_int64_exponent,%cx		/ shift amount = high   
		sub	expon1(%bp),%cx			/ exponent-expon1   
		js	invalid_or_special_64		/if expon1 high oflow   
		cmp	max_int64_shift,%cx		/shift amount shouldn't 
		jbe	do_shift_64			/exceed max_int64_shift 
		movb	max_int64_shift,%cl   
do_shift_64:   
		push	%dx   
		movb	prec64,%dl   
		call	careful_round   
		pop	%dx			/overflow if zf=true;   
		jnz	no_overflow		/-> carry-out from round   
invalid_or_special_64:   
		call	invalid_or_special	/ get ms word and pointer   
		xor	%bx,%bx   
		mov	%bx,%es:(%di)		/ store $0 into $4 ls words   
		mov	%bx,%es:2(%di)   
		mov	%bx,%es:4(%di)   
		jmp	finish_64	/ store ms word and exit   
no_overflow:   
		mov	[word_frac1+8](%bp),%ax		/load ms word   
		xor	$0x8000,%ax			/no oflow if integer   
		js	check_sign_64			/ < $0x8000000000000000
		or	[word_frac1+6](%bp),%ax		/overflow if integer > 
		or	[word_frac1+4](%bp),%ax		/ 0x8000000000000000   
		or	[word_frac1+2](%bp),%ax		/overflow if integer = 
		jnz	invalid_or_special_64		/ $0x8000000000000000 an
		cmpb	%al,sign1(%bp)
		je	invalid_or_special_64		/ and sign1 = positive   
check_sign_64:   
		cmpb	negative,sign1(%bp) /negate integer if sign 
		jne	do_store_64			/negative   
		xor	%ax,%ax   
		not	[word_frac1+2](%bp)
		add	$1,[word_frac1+2](%bp)
		not	[word_frac1+4](%bp)
		adc	%ax,[word_frac1+4](%bp)
		not	[word_frac1+6](%bp)
		adc	%ax,[word_frac1+6](%bp)
		not	[word_frac1+8](%bp)
		adc	%ax,[word_frac1+8](%bp)
do_store_64:   
		les	mem_operand_pointer(%bp),%di	/set %es:di to location 
		mov	[word_frac1+2](%bp),%ax		/to store to   
		mov	%ax,%es:(%di)   
		mov	[word_frac1+4](%bp),%ax   
		mov	%ax,%es:2(%di)   
		mov	[word_frac1+6](%bp),%ax   
		mov	%ax,%es:4(%di)   
		mov	[word_frac1+8](%bp),%ax   
finish_64:   
		mov	%ax,%es:6(%di)   
		jmp	pop_free			/ pop stack and go home   
