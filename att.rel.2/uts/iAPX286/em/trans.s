		.file "trans.s"

/   
/			t r a n s . m o d   
/                       =================   
/   
/	===============================================================   
/               intel corporation proprietary information   
/       this software  is  supplied  under  the  terms  of  a  license
/       agreement  or non-disclosure agreement  with intel corporation
/       and  may not be copied nor disclosed except in accordance with
/       the terms of that agreement.
/	===============================================================   
/   
/         @(#)trans.s	1.1 - 85/09/06
/

#include   "e80287.h"   

add_half_pattern:	.value	0x0080,0,0,0,0   
add_half_pattern_1:	.value	0x8000,0,0,0,0   
constant_ln2_ov_6:	.value	0,0,0,0x1ff5,0xec98   
constant_2_ov_ln2:	.value	0xbbc0,0x17f0,0x295c,0xaa3b,0x00b8   
constant_one:		.value	0,0,0,0,0x0100   
constant_six:    	.value	0,0,0,0,0xc000   
constant_sqrt_of_2:	.value	0x8400,0xde64,0x33f9,0x04f3,0x00b5   
constant_3_ov_ln2:	.value	0,0xf48d,0x0511,0xac5f,0x8a7f   
constant_one_1:		.value	0,0,0,0,0x8000   

/   
/ this constant has been decreased by 1 to compensate   
/ for the bits lost during right shift.   
/   

constant_c0h:	.value	0xff00
		.value	0xffff
		.value	0xffff
		.value	0xffff
		.value	0x00bf   

log_constant:	.value	0x9fc0,0xd687,0x39fb,0xc01a,0x0095	/1   
		.value	0xf260,0xdc57,0x5e68,0xd3c2,0x00a4	/2   
		.value	0xfd20,0xb43c,0xcfde,0x00d1,0x00ae	/3   
		.value	0xe680,0x9883,0xd648,0x1fb7,0x00b3	/4   
		.value	0x89c0,0xec39,0xac77,0xd69b,0x00b5	/5   
		.value	0x5380,0x914c,0x2e16,0x3cb4,0x00b7	/6   
		.value	0xfc80,0x428b,0xb778,0xf285,0x00b7	/7   
		.value	0x5700,0x63ba,0x6bd5,0x4e23,0x00b8	/8   
		.value	0x31e0,0xab26,0xf853,0x7c1f,0x00b8	/9   
		.value	0xfd80,0xa2a0,0xba1f,0x9329,0x00b8	/a   
		.value	0x57c0,0xbe18,0x7bca,0x9eb1,0x00b8	/b   
		.value	0x70a0,0xfe44,0x150d,0xa476,0x00b8	/c   
		.value	0xaa60,0x9b1b,0x8fd2,0xa758,0x00b8	/d   
		.value	0x9480,0x9ae9,0xd8be,0xa8c9,0x00b8	/e   
		.value	0x3380,0x2572,0x8017,0xa982,0x00b8	/f   
/   
tan_constant:	.value	0x34c0,0x68c2,0xa221,0x0fda,0x00c9	/0   
		.value	0x4580,0xda7b,0x2b0d,0x6338,0x00ed	/1   
		.value	0x1560,0x06eb,0xc964,0xdbaf,0x00fa	/2   
		.value	0x32c0,0x7b6e,0xd561,0xadd4,0x00fe	/3   
		.value	0x36c0,0xef4e,0xb967,0xaadd,0x00ff	/4   
		.value	0x4280,0xb125,0xdd4b,0xeaad,0x00ff	/5   
		.value	0xbbe0,0x94d5,0xdddb,0xfaaa,0x00ff	/6   
		.value	0x6800,0xd4b9,0xaddd,0xfeaa,0x00ff	/7   
		.value	0x4bc0,0xddb9,0xaadd,0xffaa,0x00ff	/8   
		.value	0x4ba0,0xdddd,0xaaad,0xffea,0x00ff	/9   
		.value	0xdba0,0xdddd,0xaaaa,0xfffa,0x00ff	/a   
		.value	0xdde0,0xaddd,0xaaaa,0xfffe,0x00ff	/b   
		.value	0xdde0,0xaadd,0xaaaa,0xffff,0x00ff	/c   
		.value	0xdde0,0xaaad,0xeaaa,0xffff,0x00ff	/d   
		.value	0xdde0,0xaaaa,0xfaaa,0xffff,0x00ff	/e   
		.value	0xade0,0xaaaa,0xfeaa,0xffff,0x00ff	/f   


	 	.text

 		.globl	arctan
		.globl	exp
		.globl	log
		.globl	splog
		.globl	move_10_bytes   
		.globl	tan
		.globl	move_constant
		.globl	test_5w
		.globl	test_4w
		.globl	test_3w   
		.globl	clear_5w
		.globl	set_5w
		.globl	set_4w
		.globl	set_3w
		.globl	left_shift_frac1_cl   
		.globl	left_shift_result_cl
		.globl	left_shift_frac2_cl   
		.globl	right_shift_frac1_cl
		.globl	right_shift_frac2_cl   
		.globl	right_shift_result_cl
		.globl	right_shift
		.globl	left_shift   
		.globl	gradual_underflow
		.globl	addition_normalize   
		.globl	sticky_right_shift
		.globl	one_left_normalize   
		.globl	subtraction_normalize
		.globl	left_shift_frac1_1   


/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			arctan:   
/			"""""""   
/   
/	function:   
/		calculates arc tangent of (y/x), where    
/		0 < y/x < 1 ;  op2 = x, op1 = y.   
/   
/	inputs:   
/		assumes that operand1 and operand2 are set up.   
/   
/	outputs:   
/		result in global record 12; possible underflow   
/		and inexact errors.   
/   
/	data accessed:   
/		- expon1		word_frac1   
/		- lsb_frac1		msb_frac1   
/		- offset_operand1		expon2   
/		- lsb_frac2		offset_operand2   
/		- offset_result		result_sign   
/		- result_tag		result_expon   
/		- result_word_frac	msb_result   
/		- offset_of_result_frac	offset_cop   
/		- offset_dop		siso   
/   
/	data changed:   
/		- expon1		frac1   
/		- frac2			result_sign   
/		- result_expon		result_frac   
/		- siso   
/   
/	procedures:   
/		put_si_result		right_shift		left_shift   
/		move_10_bytes		move_constant		addx   
/		subx			mulx			divx   
/		divid			one_left_normalize	round   
/		addition_normalize	underflow_response	pop_free   
/		test_5w			set_p_error		set_u_error   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
arctan:
		call	set_p_error		/always inexact   
		push	%di
		movb	sign1(%bp),%al		/check sign of Y operand
		cmpb	positive,%al		/is it positive?
		jne	bad1			/yes, goto abort
		movb	sign2(%bp),%al		/check sign of X operand
		cmpb	positive,%al		/is it positive?
		jne	bad1			/yes, goto arcont
		mov	expon2(%bp),%cx		/ exponent of X
		sub	expon1(%bp),%cx		/ exponent of Y
		js	bad1			/ Y > X
		mov	$offset_operand1,%di	/check for zero frac
		call	test_5w
		jz	bad1
		mov	$offset_operand2,%di	/check for zero frac
		call	test_5w
 		jnz	cont1
bad1:
		pop	%di
		call	set_i_error		/no, set error
		jmp	atn_put_atn		/abort request
cont1:
		pop	%di
		movb	positive,result_sign(%bp) /result is always positive  
		call	siso_0   
		mov	expon2(%bp),%ax   
		sub	expon1(%bp),%ax   
		mov	%ax,exp_tmp   
		cmp	$15,%ax   
		jle	atn_pseudo_div   
		cmp	$63,%ax   
/   
/tiny argument   
/   
		jg	atn_divide   
		jmp	atn_rat_appx   
atn_divide:   
		call	divid			/floating-point divide   
/   
/post operation rounding   
/   
		movb	prec64,%dl   
		call	post_op_round		/ d.e. precision rounding   
/   
/error check   
/   
		mov	result_expon(%bp),%ax   
		cmp	$0x7ffe,%ax   
		ja	atn_underflow   
		and	%ax,%ax   
		jz	atn_underflow   
		jmp	atn_put_atn_f   
atn_underflow:   
		call	underflow_response	/ underflow   
		mov	$offset_result,%di	/ for d-step compatibility   
		call	test_5w			/ don't raise underflow error   
		jz	no_u_error		/ when the result is zero   
		call	set_u_error		/ else, underflow error   
no_u_error:   
		jmp	atn_put_atn   
atn_pseudo_div:   
		call	right_shift_frac2_8   
		call	right_shift_frac1_8   
atn_loop_back_1:   
		call	subx   
		shl	$1,siso   
		cmpb 	$0,msb_result(%bp)
		jnz	atn_shift_left   
		call	move_result_cop		/ cop is $67 bits temp   
		mov	$offset_operand1,%di   
		call	right_shift_exp_tmp   
		call	addx   
		call	move_result_frac1   
		mov	$add_half_pattern,%si   
		call	add_frac2			/ add 1/2 round   
		call	move_result_frac2   
		movb	$0,lsb_frac2(%bp)		/frac2 = 64 bits x   
		call	move_cop_frac1			/frac1 = $67 bits y   
		inc	siso   
		cmpb 	$0,msb_result(%bp)
		je	atn_shift_left   
		movb	$1,%cl   
		call	right_shift_frac2_cl   
		movb	$0,lsb_frac2(%bp)
		jmp	atn_merge_0   
atn_shift_left:   
		call	left_shift_frac1_1   
atn_merge_0:   
		inc	exp_tmp   
		cmp	$15,exp_tmp   
		jna	atn_loop_back_1   
		movb	$7,%cl   
		call	left_shift_frac1_cl   
		and	$0xe000,word_frac1(%bp)
		mov	$15,exp_tmp   
		call	left_shift_frac2_8   
atn_rat_appx:   
		call	divx					/y/x -> z (67)  
		call	move_result_frac1   
		movb	$9,%cl   
		call	right_shift_frac1_cl   
		andb	$0xe0,lsb_frac1(%bp)
		call	move_result_frac2   
		call	right_shift_frac2_8   
		andb	$0xe0,lsb_frac2(%bp)
		call	result_and_0x0e000   
		call	move_result_cop   
		call	addx   
		call	move_result_dop		/dop = new y (67)   
		call	move_cop_frac1   
		call	move_cop_frac2   
		movb	$30,%cl   
		call	right_shift_frac1_cl	/ perform multiply   
		mov	$0,word_frac1(%bp)
		movb	$30,%cl   
		call	left_shift_frac1_cl   
		call	mulx   
		mov	$offset_result,%di   
		call	right_shift_exp_tmp   
		movb	$0xc0,msb_result(%bp)
		call	move_result_frac2	/new x (64)   
		and	$0xe000,word_frac2(%bp)
		cmpb 	$0,msb_dop(%bp)
		movb	$1,%al   
		movb	$7,%cl   
		jne	atn_merge_1   
		movb	$0,%al   
		movb	$8,%cl   
atn_merge_1:   
		push	%ax				/save %al   
		mov	$offset_dop,%di   
		call	left_shift   
		call	move_dop_frac1   
		call 	divx   
		call	result_and_0x0e000		/theta is here   
		pop	%ax   
		shrb	$1,%al   
		movb	$7,%cl   
		jnb	atn_merge_2   
		movb	$6,%cl   
atn_merge_2:   
		call	right_shift_result_cl   
		call	move_result_frac1   
atn_loop_back_2:   
		shr	$1,siso   
		jnb	atn_q_bit_not_set   
		mov	$10,%ax   
		mul	exp_tmp   
		add	$tan_constant,%ax   
		mov	%ax,%si   
		call	add_frac2   
		call	move_result_frac1   
atn_q_bit_not_set:   
		cmp 	$0,siso
		jz	atn_end_loop   
		movb	$1,%cl   
		call	right_shift_frac1_cl   
		andb	$0xe0,lsb_frac1(%bp)
		dec	exp_tmp   
		jmp	atn_loop_back_2   
atn_end_loop:   
		cmpb 	$0,msb_frac1(%bp)
		movb	$8,%cl   
		je	atn_merge_3   
		movb	$7,%cl   
		dec	exp_tmp   
atn_merge_3:   
		call	left_shift_frac1_cl   
		mov	$0x3ffe,expon1(%bp)
		mov	exp_tmp,%ax   
		sub	%ax,expon1(%bp)
		mov	$offset_operand1,%di   
		call	one_left_normalize   
		mov	expon1(%bp),%ax   
		mov	%ax,result_expon(%bp)
		call	add_half1_frac2   
atn_put_atn_f:   
		movb	valid,result_tag(%bp)	/valid result   
atn_put_atn:   
		mov	$offset_result,%di   
atn_put_result:   
		call	put_si_result   

		jmp	pop_free

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			exp:   
/			"""   
/	function:   
/		calculates 2**x-1, where 0 <= x < 1/2.   
/	inputs:   
/		assumes that operand1 is set up.   
/   
/	outputs:   
/		result in global record 12; possible underflow    
/		and inexact errors.   
/   
/	data accessed:   
/		- offset_operand1		tag1   
/		- expon1			word_frac1   
/		- lsb_frac1			offset_operand1   
/		- sign2				expon2   
/		- offset_operand2		offset_result   
/		- result_tag			result_expon   
/		- lsb_result			msb_result   
/		- offset_result		siso   
/		- offset_cop   
/   
/	data changed:   
/		- expon1			lsb_frac1   
/		- sign2				expon2   
/		- result_tag			msb_result   
/		- siso   
/   
/	procedures:   
/		put_si_result		right_shift		left_shift   
/		move_10_bytes		move_constant		addx   
/		subx			mulx			divx   
/		addition_normalize	round			divid   
/		log_constant		underflow_response	set_p_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

exp:
		cmpb	$0x01,tag1(%bp)		/operand = 0 _   
		jne	exp_non_zero   
		jmp	put_op1_result   
/   
/pseudo_division   
/   
exp_non_zero:   
		call	siso_0   
		mov	$0x3fff,%ax   
		sub	expon1(%bp),%ax   
		cmp	$63,%ax   
		ja	exp_tiny_argument   
		call	set_p_error			/ set error flag   
		mov	%ax,expon1(%bp)
		cmp	$15,%ax   
		ja	exp_rat_appx   
		call	right_shift_frac1_8   
exp_loop_back_1:   
		mov	$10,%ax   
		mul	expon1(%bp)
		add	$[log_constant-10],%ax  / NEED OFF  
		mov	%ax,%si   
		call	sub_frac2   
		shl	$1,siso   
		cmpb	$0,msb_result(%bp)
		jne	exp_merge_0   
		call	move_result_frac1   
		andb	$0xe0,lsb_frac1(%bp)
		inc	siso   
exp_merge_0:   
		inc	expon1(%bp)
		cmp	$15,expon1(%bp)
		ja	exp_branch_1   
		call	left_shift_frac1_1   
		jmp	exp_loop_back_1   
exp_tiny_argument:   
		movb	$0,sign2(%bp)
		mov	$0x3fff,expon2(%bp)
		mov	$constant_2_ov_ln2,%si   
		call	move_constant_frac2   
		call	left_shift_frac2_8	/ tiny argument   
		call	divid			/floating-point divide   
		movb	prec64,%dl   
		call	post_op_round   
/   
/error checking   
/   
		mov	result_expon(%bp),%cx   
		cmp	$0x7ffe,%cx   
		ja	exp_underflow   
		and	%cx,%cx   
		mov	valid,result_tag(%bp)	/valid result   
		jnz	exp_put_exp_jmp   
exp_underflow:   
		call	underflow_response   
exp_put_exp_jmp:   
		jmp	exp_put_exp_f   
/   
/pre_rat_appx   
/   
exp_branch_1:   
		mov	$15,expon1(%bp)
		call	left_shift_frac1_8   
exp_rat_appx:   
		call	move_frac1_cop   
/   
/80287 multiply (does 67-bit by 34-bit multiplication only).   
/   
		movb	$30,%cl   
		call	right_shift_frac1_cl   
		and	$0,word_frac1(%bp)
		movb	$30,%cl   
		call	left_shift_frac1_cl   
		call	mulx   
		call	move_result_frac1   
		and	$0xe000,word_frac1(%bp)
		mov	$constant_ln2_ov_6,%si   
		call	mul_frac2   
		mov	expon1(%bp),%cx   
		addb	$10,%cl   
		call	right_shift_result_cl   
		andb	$0xe0,lsb_result(%bp)
		call	move_cop_frac1   
		call	right_shift_frac1_8   
		call	move_result_frac2   
		call	subx   
		mov	expon1(%bp),%cx   
		inc	%cx   
		call	right_shift_result_cl   
		andb	$0xe0,lsb_result(%bp)
		call	move_result_frac2   
		mov	$constant_2_ov_ln2,%si   
		call	sub_frac1   
		call	move_cop_frac1   
		call	move_result_frac2   
		call	left_shift_frac2_8   
		call	divx   
		movb	$8,%cl   
		call	right_shift_result_cl   
		andb	$0xe0,lsb_result(%bp)
/   
/pseudo_multiply   
/   
exp_loop_back_2:   
		shr	$1,siso   
		jnb	exp_merge_2   
		call	move_result_frac1   
		call	move_result_frac2   
		mov	expon1(%bp),%cx   
		call	right_shift_frac2_cl   
		andb	$0xe0,lsb_frac2(%bp)
		or	$0x80,[word_frac2+8](%bp)
		call	addx   
exp_merge_2:   
		cmp	$0,siso   
		je	exp_almost_end   
		call	right_shift_result_1   
		andb	$0xe0,lsb_result(%bp)
		dec	expon1(%bp)
		jmp	exp_loop_back_2   
exp_almost_end:   
		movb	$8,%cl   
		cmpb	$0,msb_result(%bp)
		je	exp_merge_3   
		movb	$7,%cl   
		dec	expon1(%bp)
exp_merge_3:   
		call	left_shift_result_cl   
		mov	$0x3fff,%ax   
		sub	expon1(%bp),%ax   
		mov	%ax,result_expon(%bp)
		mov	$[POSITIVE+[VALID\*0x100]],result_sign(%bp)
		testb	$0x80,msb_result(%bp)
		jne	exp_add_half_round   
/   
/normalize x   
/   
		movb	$1,%cl   
		call	left_shift_result_cl   
		dec	result_expon(%bp)
exp_add_half_round:   
		call	move_result_frac1   
		call	add_half1_frac2   
exp_put_exp_f:   
		mov	$offset_result,%di   
		jmp	put_si_result   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			log:   
/			"""   
/	function:   
/		calculates y times log(x) or y times log(1+x).   
/   
/	inputs:   
/		assumes that operand1 and operand2 are set up;   
/		op1 = x, op2 = y.   
/   
/	outputs:   
/		possible underflow, overflow, or inexact errors   
/   
/	data accessed:   
/		- operation_type		offset_result_rec   
/		- sign1				expon1   
/		- offset_operand1		offset_operand2   
/		- tag2				sign2   
/		- expon2			word_frac2   
/		- msb_frac2			offset_operand2   
/		- offset_result			result_tag   
/		- result_sign			result_expon   
/		- result_word_frac		msb_result   
/		- msb_result			offset_result   
/		- result2_tag			result2_sign   
/		- msb_result2			loop_ct   
/		- offset_cop			offset_dop   
/		- siso   
/   
/	data changed:   
/		- sign1				sign2   
/		- msb_frac2			word_frac2   
/		- result_tag			result_sign   
/		- result_frac			offset_result   
/		- result2_tag			result2_sign   
/		- msb_result2			loop_ct   
/		- siso   
/   
/	procedures:   
/		put_result		right_shift		left_shift   
/		move_10_bytes		move_constant		addx   
/		subx			mulx			log_divx   
/		one_left_normalize	addition_normalize	log_constant   
/		test_5w			subtraction_normalize	test_4w   
/		underflow_response	overflow_response	decompose   
/		pop_free		sp_subadd		mult   
/		round			get_precision		set_p_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

log:
		movb	valid,result2_tag(%bp)
		jmp	log_entry   
splog:   
		movb	special,result2_tag(%bp)	/ flag for special log  
		push	%di
		movb	sign1(%bp),%al		/check sign of X operand
		cmpb	positive,%al		/is it positive?
		jne	bad2			/yes, goto abort
		mov	$offset_operand1,%di	/check for zero frac
		call	test_5w
 		jnz	cont2
bad2:
		pop	%di
		call	set_i_error		/no, set error
		jmp	atn_put_atn		/abort request
cont2:
		pop	%di
log_entry:   
		mov	$offset_cop,%di   
		call	move_frac2		/temp store y, cop <- frac(y)   
		movb	tag2(%bp),%al   
		movb	%al,lsb_cop(%bp)
		mov	expon2(%bp),%ax		/loop_ct <- expo(y)   
		mov	%ax,loop_ct   
		movb	sign2(%bp),%al		/lsb_result2 <- sign2   
		movb	%al,lsb_result2(%bp)
		movb	positive,result2_sign(%bp) / used for sign of result  
		cmpb	valid,result2_tag(%bp)	/ after log core   
		jnz	log_special_log		/ result sign same as result2   
		call	right_shift_frac1_8   
		mov	$constant_sqrt_of_2,%si   
		call	sub_frac2   
		cmpb 	$0,msb_result(%bp)
		jne	log_small_lg   
		jmp	log_large_lg   
log_sp_negative_x:   
		call	move_frac1_result   
		movb	negative,result2_sign(%bp)
		jmp	log_core   
log_special_log:   
		mov	$offset_operand1,%di	/ weed out the special case   
		call	test_5w			/ where x = 0.  result should   
		jz	log_splog_zero		/ be exact signed zero 6/17/82. 
		mov	expon1(%bp),%ax		/ log x not 0   
		mov	%ax,result_expon(%bp)
		cmpb	$0,sign1(%bp)
		jnz	log_sp_negative_x   
		call	move_frac1_frac2   
		mov	$0x3fff,%cx   
		sub	expon1(%bp),%cx   
		cmp	$72,%cx   
		jbe	log_sp_label_1   
		mov	$72,%cx   
log_sp_label_1:   
		call	right_shift_frac2_cl   
		mov	word_frac2(%bp),%cx   
		and     $0x1fff,%cx   
		or      %cx,%dx   
		jz	log_non_sticky   
		or	$0x2000,word_frac2(%bp)
log_non_sticky:   
		orb	$0x80,msb_frac2(%bp)
		and	$0xe000,word_frac2(%bp)
		jmp	log_div   
/   
/ frac(x) < sqrt(2)   
/   
log_small_lg:   
		call	left_shift_frac1_8   
		call	move_frac1_result   
		movb	$1,%cl   
		call	left_shift_result_cl   
		mov	$offset_result,%di   
		call	test_5w   
		jz	log_power_of_two   
		mov	$0x3ffe,result_expon(%bp)
		mov	$offset_result,%di   
		call	subtraction_normalize   
		call	move_frac1_frac2   
		call	move_result_frac1   
log_div:   
		call	log_divx   
		call	result_and_0x0e000   
		jmp	log_merge_lg   
/   
/ x = $2 ** n   
/   
log_power_of_two:   
		call	decompose   
log_mul:   
		mov	$offset_operand1,%di   
		call	test_5w   
		jnz	log_x_not_0   
		call	log_round		/ round the result   
log_splog_zero:   
		movb	$1,tag1(%bp)		/zero tag   
		movb	sign1(%bp),%al   
		xorb 	lsb_result2(%bp),%al   
		movb	%al,sign1(%bp)
		mov	$offset_operand1,%di   
		jmp	atn_put_result		/to preserve sign, even y = $0   
log_x_not_0:   
		cmpb	$1,lsb_cop(%bp)
		jne	log_y_not_zero   
/   
/y=0   
/   
		xor	%ax,%ax   
		mov	$result_word_frac+2,%di   
		call	set_4w   
		mov	%ax,result_expon(%bp)
		movb	sign1(%bp),%al   
		xorb	lsb_result2(%bp),%al   
		movb	%al,result_sign(%bp)
		movb	special,result_tag(%bp)
		jmp	log_finish_up   
log_y_not_zero:   
		movb	$0,lsb_cop(%bp)
		call	move_cop_frac2		/restore y into op2   
		mov	loop_ct,%ax   
		mov	%ax,expon2(%bp)
		movb	lsb_result2(%bp),%al   
		movb	%al,sign2(%bp)
		call	mult   
		push	%ax   
		call	log_round		/ round the result   
/   
/error checking   
/   
		mov	result_expon(%bp),%ax   
		cmp	$0x7ffe,%ax   
		ja	log_over_underflow   
		and	%ax,%ax   
		jnz	log_set_tag_valid   
		mov	$result_word_frac,%di   
		call	test_4w   
		jnz	log_over_underflow   
/   
/zero result   
/   
		movb	special,result_tag(%bp)
		jmp	log_pop_finish   
log_set_tag_valid:   
		movb	valid,result_tag(%bp)
log_pop_finish:   
		pop	%ax			/ discard underflow flag   
log_finish_up:   
		jmp	atn_put_atn   
log_over_underflow:   
		pop	%ax		/ retrieve underflow possible flag   
		cmp	$0x7fff,result_expon(%bp)
		je	log_overflow_case   
		cmpb	$0,%al   
		jne	log_underflow_case   
log_overflow_case:   
		call	overflow_response   
		jmp	log_finish_up   
log_underflow_case:   
		call	underflow_response   
		jmp	log_finish_up   
/   
/frac(x) > sqrt(2)   
/   
log_large_lg:   
		inc	expon1(%bp)
		call	move_frac1_frac2   
		mov	$constant_one,%si   
		call	sub_frac1   
		movb	$7,%cl   
		call	left_shift_result_cl   
		movb	negative,result2_sign(%bp) /set flag for large log 
		mov	$0x3fff,result_expon(%bp)
log_merge_lg:   
		mov	$offset_result,%di	/ don't normalize a zero   
		call	test_5w			/ is the result zero_   
		jz	log_core		/ yes, no need to normalize   
		mov	$offset_result,%di   
		call	subtraction_normalize   
log_core:   
		call	move_result_frac2   
		call	siso_0   
		mov	$0x3fff,%ax   
		sub	result_expon(%bp),%ax   
		mov	%ax,exp_tmp   
		cmp	$15,%ax   
		ja	log_rat_apx   
		call	move_result_frac1	/ log pseudo divide   
		mov	$7,%cx   
		add	exp_tmp,%cx   
		call	right_shift_frac1_cl   
		call	right_shift_frac2_8   
		call	addx   
		cmpb 	$0,msb_result(%bp)
		je	log_loop_back_1   
		andb	$0xe0,lsb_result(%bp)
		movb	$0,msb_result(%bp)
		call	move_result_frac2   
		mov	$1,siso   
log_loop_back_1:   
		shl	$1,siso   
		call	move_frac2_frac1   
		mov	exp_tmp,%cx   
		call	right_shift_frac1_cl   
 		orb	$0x80,[word_frac1+8](%bp)
		call	addx   
		cmpb 	$0,msb_result(%bp)
		je	log_merge_2   
		andb	$0xe0,lsb_result(%bp)
		movb	$0,msb_result(%bp)
		call	move_result_frac2   
		inc	siso   
log_merge_2:   
		movb	$1,%cl   
		call	left_shift_frac2_cl   
		inc	exp_tmp   
		cmp	$15,exp_tmp   
		jbe	log_loop_back_1   
		call	left_shift_frac2_8   
/   
/rational approximation   
/   
log_rat_apx:   
		call	set_p_error			/set p flag   
		call	move_frac2_frac1   
		call	mulx   
		mov	exp_tmp,%cx   
		cmp	$72,%cx   
		jbe	log_label_4   
		mov	$72,%cx   
log_label_4:   
		call	right_shift_result_cl   
		call	result_and_0x0e000   
		call	move_result_frac2   
		call	subx   
		call	move_result_dop   
		mov	exp_tmp,%cx   
		cmp	$72,%cx   
		jbe	log_label_5   
		mov	$72,%cx   
log_label_5:   
		call	right_shift_frac1_cl   
		and	$0xe000,word_frac1(%bp)
		call	move_frac1_frac2   
		mov	$constant_six,%si   
		call	sub_frac1   
		call	move_result_frac1   
		call	move_dop_frac2   
		mov	exp_tmp,%cx   
		inc	%cx   
		cmp	$72,%cx   
		jbe	log_label_6   
		mov	$72,%cx   
log_label_6:   
		call	right_shift_frac2_cl   
		and	$0xf000,word_frac2(%bp)
		call	subx   
		call	result_and_0x0e000   
		call	move_dop_frac1   
		call	move_result_frac2   
		call	log_divx   
		call	result_and_0x0e000   
		call	move_result_frac1   
		mov	$constant_3_ov_ln2,%si   
		call	mul_frac2   
		movb	$7,%cl   
		call	right_shift_result_cl   
		dec	exp_tmp   
log_loop_back_2:   
		shr	$1,siso   
		jnb	log_no_carry   
		mov	exp_tmp,%si   
		sub	$1,%si   
		mov	$10,%ax   
		mul	%si   
		mov	%ax,%si   
		add	$log_constant,%si   
		call	move_constant_frac2   
		call	move_result_frac1   
		call	addx   
log_no_carry:   
		cmp 	$0,siso
		jz	log_end_loop   
		call	right_shift_result_1   
		dec	exp_tmp   
		jmp	log_loop_back_2   
log_end_loop:   
		mov	$0x4007,%ax			/4007 = 3fff+8   
		sub	exp_tmp,%ax   
		mov	%ax,result_expon(%bp)
		andb	$0xe0,lsb_result(%bp)
		mov	$offset_result,%di	/ don't normalize a zero   
		call	test_5w			/ is the result zero_   
		jz	log_end_log_core	/ yes, no need to normalize   
		mov	$offset_result,%di   
		call	subtraction_normalize   
log_end_log_core:   
		cmpb	valid,result2_tag(%bp)
		jnz	log_special_log_end   
		call	decompose   
		call	move_result_frac2   
		mov	result_expon(%bp),%ax   
		mov	%ax,expon2(%bp)
		movb	$add_op,operation_type(%bp)
		movb	result2_sign(%bp),%al   
		movb	%al,sign2(%bp)
		call	sp_subadd   
log_merge_4:   
		call	move_result_frac1   
		movb	result_sign(%bp),%al   
		movb	%al,sign1(%bp)
		mov	result_expon(%bp),%ax   
		mov	%ax,expon1(%bp)
		jmp	log_mul   
log_special_log_end:   
		movb	result2_sign(%bp),%al   
		movb	%al,result_sign(%bp)
		jmp	log_merge_4   
/   
/do round				; made into common subroutine for all   
/   
log_round:   
		movb	$1,lsb_result(%bp) /fixed up such that   
		call	get_precision   
post_op_round:   
		mov	$offset_result,%di		/ s = $1 always   
		movb	false,%al			/ not second rounding   
		call	round   
		mov	$offset_result,%di   
		movb	$4,%ah				/ fall through   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/		addition_normalize:   
/   
/	function:   
/		performs normalization after addition if necessary.   
/   
/	inputs:   
/		carry-out from addition in %al upon entry.   
/		fraction in di.   
/   
/	outputs:   
/		increments exponent and shifts 1 bit in from left if al=1.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/	procedures:   
/		sticky_right_shift   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

addition_normalize:
		testb	$0x01,%al			/if low bit of %al = 1
		jnz	right_shift_1			/ then shift right $1   
		ret					/ else, exit   
right_shift_1:   
		inc	expon(%bp,%di)			/increment exponent   
		movb	$1,%cl				/shift right $1 bit   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			sticky_right_shift:   
/			""""""""""""""""""   
/	function:   
/		shifts a 10-byte number right by amount in   
/		cl register.  %di contains of number.   
/   
/	inputs:   
/		low bit of %al contains bit to inject from left.   
/		(for shifts of more than one, the result is undefined   
/		unless %al contains zero).   
/   
/	outputs:   
/		the low-order bit of the number is "sticky", ie, it is   
/		left as a one if any ones were shifted out.           
/   
/	data accessed:   
/   
/	data changed:   
/   
/	procedures:   
/		right_shift   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

sticky_right_shift:
		call	right_shift			/do right shift   
		and	%dx,%dx				/dx <> $0 if 1's lost   
		jz	sticky_right_shift_done		/done if no 1's lost   
 		orb	$0x01,(%bp,%di)		/else, set sticky bit   
sticky_right_shift_done:   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			gradual_underflow:   
/   
/	function:   
/		changes an "underflowed" number to a legal
/		denormalized number.   
/   
/	input:   
/		address of exponent and fraction in operands or result   
/		(ptr in %di register); minimum legal exponent(in   
/		ax register).   
/   
/	output:   
/		denormalized fraction and adjusted exponent (in input   
/		record)   
/   
/	data accessed:   
/		- expon			word_frac   
/   
/	data changed:   
/		- expon			word_frac   
/   
/	procedures:   
/		sticky_right_shift   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

gradual_underflow:
		mov	%ax,%cx			/set exponent to minimum   
		xchg	expon(%bp,%di),%ax
		sub	%ax,%cx			/form shift amount   
		cmp	$67,%cx			/(exponent-minimum exponent)   
		jb	do_right_shift   
		movb	$67,%cl			/shift amount must be <= $67   
do_right_shift:   
		xorb	%al,%al			/amount   
		jmp	sticky_right_shift   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			tan:   
/   
/	function:   
/		calculates y and x such that y/x = tan(theta)   
/   
/	inputs:   
/		assumes that operand1 is set up (theta).   
/   
/	outputs:   
/		results in result records;  y = result,  x = result2.   
/ 		no error checking except for possible stack overflow.   
/   
/	data accessed:   
/		- result_rec_offset		result2_rec_  
/		- offset_operand1		expon1   
/		- word_frac1			lsb_frac1   
/		- offset_operand1		offset_operand2   
/		- sign2				tag2   
/		- expon2			lsb_frac2   
/		- msb_frac2			offset_operand2   
/		- lsb_result			msb_result   
/		- offset_result			siso   
/		- offset_cop			lsb_cop   
/		- offset_dop			lsb_dop   
/		- msb_dop   
/   
/	data changed:   
/		- expon1			word_frac1   
/		- lsb_frac1			sign2   
/		- tag2				expon2   
/		- lsb_frac2			msb_result   
/		- lsb_cop			lsb_dop   
/		- siso   
/   
/	procedures:   
/		do_exchange		stack_full_		right_shift   
/		left_shift		move_10_bytes		move_constant   
/		addx			subx			mulx   
/		subtraction_normalize	test_5w			set_p_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

tan:
		call	stack_full_			/check stack overflow   
		jnz	sticky_right_shift_done		/ abort if stack full   
		mov	$offset_operand1,%di		/ load op1 ptr (theta)  
		call	test_5w				/ test for fptan(0)   
		jz	tan_pseudo_divide		/ inexact if theta <> $0
		call	set_p_error   
/   
/pseudo_division   
/   
tan_pseudo_divide:   
		call	siso_0   
		mov	$0x3fff,%ax   
		sub	expon1(%bp),%ax   
		cmp	$63,%ax   
		ja	tan_tiny_argument   
		mov	%ax,expon1(%bp)
		cmp	$16,%ax   
		ja	tan_rat_appx   
		dec	expon1(%bp)	/ decrement expon1   
		call	right_shift_frac1_8   
tan_loop_back_1:   
		mov	$10,%ax   
		mul	expon1(%bp)
		add	$tan_constant,%ax  	/ NEED OFFSET 
		mov	%ax,%si   
		call	sub_frac2   
		shl	$1,siso   
		cmpb	$0,msb_result(%bp)
		jne	tan_merge_0   
		call	move_result_frac1   
		andb	$0xe0,lsb_frac1(%bp)
		inc	siso   
tan_merge_0:   
		inc	expon1(%bp)
		cmp	$15,expon1(%bp)
		ja	tan_branch_1   
		call	left_shift_frac1_1   
		jmp	tan_loop_back_1   
tan_tiny_argument:   
		mov	$constant_one_1,%si   
		call	move_constant_frac2   
		mov	$0x3fff,expon2(%bp)
		mov	$POSITIVE+VALID\*0x100,sign2(%bp)
		jmp	do_exchange   
tan_branch_1:   
		call	left_shift_frac1_8   
		mov	$0,word_frac1(%bp)
tan_rat_appx:   
		call	move_frac1_cop			/ cop <- theta, cop tmp 
		call	mulx				/theta*theta   
		mov	expon1(%bp),%cx   
		shl	$1,%cx   
		add	$8,%cx   
		call	right_shift_result_cl   
		movb    $0,lsb_result(%bp)		/64 bits only   
		call	move_result_frac2   
		mov	$constant_c0h,%si   
		call	sub_frac1			/x <- 1.1-theta*theta   
		call	move_cop_frac2   
		call	right_shift_frac2_8   
		call	move_cop_frac1   
		movb	$9,%cl   
		call	right_shift_frac1_cl   
		call	move_result_cop			/cop <- x   
		call	addx				/y <- y'+theta   
		call	right_shift_result_1   
		call	move_result_dop			/dop <- y, dop   
		dec	expon1(%bp)		/is a 67 bits temp   
/   
/pseudo_multiply   
/   
tan_loop_back_2:   
		shr	$1,siso   
		jnb	tan_merge_2   
		call	move_cop_frac1			/x   
		call	move_dop_frac2			/y   
		call	addx				/x+y   
		call	move_result_dop			/ -> dop   
		mov	expon1(%bp),%cx   
		shl	$1,%cx   
		call	right_shift_frac2_cl   
		andb	$0xe0,lsb_frac2(%bp)
		call	subx				/x-y*2(-2exp)   
		call	move_result_cop			/ -> cop   
		movb	$0,lsb_cop(%bp)
tan_merge_2:   
		cmp	$0,siso   
		je	tan_almost_end   
		movb	$1,%cl   
		mov	$offset_dop,%di   
		call	right_shift_al0   
		andb	$0xe0,lsb_dop(%bp)
		dec	expon1(%bp)
		jmp	tan_loop_back_2   
tan_almost_end:   
		movb	$8,%cl   
		cmpb	$0,msb_dop(%bp)
		je	tan_merge_3   
		movb	$7,%cl   
		dec	expon1(%bp)
tan_merge_3:   
		mov	$offset_dop,%di   
		call	left_shift   
		call	move_dop_frac1   
		mov	$0x3fff,%ax   
		sub	expon1(%bp),%ax   
		mov	%ax,expon1(%bp)
/   
/normalize y   
/   
		mov	$offset_operand1,%di   
		call	subtraction_normalize   
/   
/now x   
/   
		call	move_cop_frac2   
		call	left_shift_frac2_8   
		mov	$0x3fff,expon2(%bp)
		mov	$POSITIVE+VALID\*0x100,sign2(%bp)
/   
/normalize x   
/   
		mov	$offset_operand2,%di   
		call	subtraction_normalize   
		jmp	do_exchange			/ put both results   
siso_0:   
		mov	$0,siso			/ clear siso   
		ret   
result_and_0x0e000:   
		and	$0xe000,result_word_frac(%bp) / mask result frac   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			test_5w, test_4w, test_3w   
/   
/	function:   
/		test $3, $4, or $5 consecutive words for zero   
/   
/	inputs:   
/       	least significant word pointed to by %ss:(%bp+%di)   
/		ax must be clear upon entry for test_4w and test_3w   
/   
/	outputs:   
/		if variable = $0, %ax = 0x00000; zf = $1   
/		otherwise    %ax = 0x0ffff; zf = $0   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

test_5w:
		mov	8(%bp,%di),%ax	/ dump the data into %ax   
test_4w:   
		or	6(%bp,%di),%ax   
test_3w:   
		or	4(%bp,%di),%ax   
		or	2(%bp,%di),%ax   
		or	(%bp,%di),%ax   
		jnz	its_not_zero	/ branch if the number is nonzero   
		ret			/ return with %ax = $0x00000, zf = $1   
its_not_zero:   
		mov	$0xffff,%ax	/ return with %ax = $0x0ffff, zf = $0  
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			clear_5w, set_5w, set__4w, set_3w   
/   
/	function:   
/		sets $3, $4, or $5 consecutive words to zero or given value   
/   
/	inputs:   
/       	least significant word pointed to by %ss:(%bp+%di)   
/		ax contains value given to store for set routines   
/   
/	outputs:   
/		ax contains zero for the clear_5w routine   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

clear_5w:
		xor	%ax,%ax   
set_5w:   
		mov	%ax,8(%bp,%di)	/ store the value in %ax   
set_4w:   
		mov	%ax,6(%bp,%di)
set_3w:   
		mov	%ax,4(%bp,%di)   
		mov	%ax,2(%bp,%di)   
		mov	%ax,(%bp,%di)   
		ret   
/   
/ common entries to shift and move subroutines   
/   
/ right_shift group   
/   
right_shift_exp_tmp:   
		mov	exp_tmp,%cx   
		shlb	$1,%cl
		jmp	right_shift_al0   
right_shift_result_1:   
		movb	$1,%cl   
right_shift_result_cl:   
		mov	$offset_result,%di   
		jmp	right_shift_al0   
right_shift_frac2_8:   
		movb	$8,%cl   
right_shift_frac2_cl:   
		mov	$offset_operand2,%di   
		jmp	right_shift_al0   
right_shift_frac1_8:   
		movb	$8,%cl   
right_shift_frac1_cl:   
		mov	$offset_operand1,%di   
right_shift_al0:   
		movb	$0,%al   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			right_shift:   
/   
/	function:   
/		shifts a 10-byte number right by amount in %cl register.   
/   
/	inputs:   
/		low bit of %al contains bit to inject from left.   
/		di contains the of the least significant byte   
/		of the 10-byte number.   
/   
/	outputs:   
/		on  %dx,return is non-zero if any ones have been shifted out.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

right_shift:
		cmpb	$80,%cl				/ more than $80 bits_   
		jle	right_count			/ no, form right count  
		movb	$80,%cl				/ yes, $80 is the max   
right_count:   
		xor	%dx,%dx				/dx = the "sticky word" 
		rorb	$1,%al				/ move lsb to msb   
		cbw					/ transmit sign to %ah  
		movb	%cl,%al 			/ copy amount to %al   
		andb	$0x78,%al			/ byte count = bits 3-6 
		jz	rightover_bits			/ branch if count < $8  
		push	%cx				/ stack bit count   
		push	%di				/ stack frac   
		push	%ds				/ save a_msr   
		push	%ax				/ stack insert byte   
 		push	%ss				/ copy stack segment   
		pop	%ds				/ to %ds and %es   
		push	%ss   
		pop	%es   
		cbw					/ clear %ah   
		shr	$3,%ax				/ normalize byte count  
		mov	%ax,%cx				/ set number to check   
		add	%bp,%di   
		mov	%di,%si				/ source ptr = %ss:bp+si   
right_shift_check:   
		orb	(%di),%dl			/ accumulate lost bytes 
		inc	%di				/ increment frac index  
		loop	right_shift_check   
		movb	$0x0a,%cl				/ count = 10-ax 
		sub	%ax,%cx				/ set number to move   
		mov	%si,%di				/ restore dest ptr   
		add	%ax,%si				/ src ptr = dest+ax   
          	rep	     				/ shift right %cx bytes 
		smovb
		mov	%ax,%cx				/ set number to inject  
		pop	%ax				/ reload inject byte   
right_inject_bytes:   
		movb	%ah,(%di)			/ inject next byte   
		inc	%di				/ increment frac index  
		loop	right_inject_bytes   
		pop	%ds				/ reload a_msr   
		pop	%di				/ reload lsb   
		pop	%cx				/ reload bit count   
rightover_bits:   
		and	$0x0007,%cx			/ shift remaining bits  
		jz	right_shift_done		/ exit if all done   
right_shift_bits:   
		rorb	$1,%ah				/ inject leading bit   
		rcr	$1,8(%bp,%di)		/ into the 80-bit field 
		rcr	$1,6(%bp,%di)	/ rotate the all five   
		rcr	$1,4(%bp,%di)	/ words in the number   
		rcr	$1,2(%bp,%di)   
		rcr	$1,(%bp,%di)		/ shift out lsb   
		adc	%dx,%dx			/ accumulate lost bits   
		loop	right_shift_bits	/ loop required amount   
right_shift_done:   
		ret   
/   
/ left_shift group   
/   
left_shift_result_cl:   
		mov	$offset_result,%di   
		jmp	left_shift   
left_shift_frac2_8:   
		movb	$8,%cl   
left_shift_frac2_cl:   
		mov	$offset_operand2,%di   
		jmp	left_shift   
left_shift_frac1_8:   
		movb	$8,%cl   
		jmp	left_shift_frac1_cl   
left_shift_frac1_1:   
		movb	$1,%cl   
left_shift_frac1_cl:   
		mov	$offset_operand1,%di   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			left_shift:   
/   
/	function:   
/		shifts a 10-byte number left by amount in %cl reg.   
/   
/	input:   
/		di contains of low-byte.   
/   
/	outputs:   
/   
/	data accessed:   
/   
/	data changed:   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

left_shift:
		cmpb	$80,%cl				/ more than $80 bits_   
		jle	left_count			/ no, form left count   
		movb	$80,%cl				/ yes, $80 is the max   
left_count:   
		movb	%cl,%al				/ form byte count   
		andb	$0x78,%al			/ = bits 3-6 of amount 
		jz	leftover_bits			/ branch if count < $8  
		push	%cx				/ stack bit count   
		push	%di				/ stack frac   
		push	%ds				/ save a_msr   
		cbw					/ clear %ah   
		shr	$3,%ax				/ normalize byte count  
		dec	%ax   
		mov	$0x0009,%cx			/ count = 10-ax   
		add	%cx,%di   
		sub	%ax,%cx				/ dest = %ss:bp+si   
		add	%bp,%di   
		mov	%di,%si				/ dest = %ss:bp+di   
		sub	%ax,%si   
		dec	%si   
		push	%ss				/copy %ss to %ds and %es
		pop	%ds   
		push	%ss   
		pop	%es   
		std					/ set direction flag   
		rep	     				/ shift bytes left   
		smovb
		movb	%cl,(%di)			/ clear next byte   
		mov	%ax,%cx				/ form zero count   
		mov	%di,%si				/ form string offsets   
		dec	%di   
		rep	     				/ clear rest of string  
		smovb
		cld					/ clear direction flag  
		pop	%ds				/ reload a_msr   
		pop	%di				/ reload frac   
		pop	%cx				/ reload bit count   
leftover_bits:   
		and	$0x0007,%cx			/ bits 0-2 = bit count  
		jz	left_shift_done			/ exit if all done   
left_shift_bits:   
		shl	$1,(%bp,%di)		/ shift five words   
		rcl	$1,2(%bp,%di)	/ one bit left, using   
		rcl	$1,4(%bp,%di)	/ the carry flag out   
		rcl	$1,6(%bp,%di)   
		rcl	$1,8(%bp,%di)   
		loop	left_shift_bits			/ loop bit count times   
left_shift_done:   

		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			one_left_normalize:   
/   
/	function:   
/		normalizes following a multiplication.   
/   
/	inputs:   
/		assumes fraction is not zero.  %di = frac offset.   
/   
/	outputs:   
/		shifts fraction left and decrements exponent until   
/		fraction normalized, but no more than one left shift.   
/               zf is reset if the fraction is normalized upon return.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

one_left_normalize:

		testb	$0x80,[frac80+9](%bp,%di)	/normalized_   
		jnz	one_left_norm_done		/quit if normalized.   
		push	%di				/ save frac   
		mov	$0x0001,%cx			/ load loop count   
		call	left_shift_bits			/shift $5 words left $1 
		pop	%di				/ reload frac  
		dec	expon(%bp,%di)	/decrement exponent   
		testb	$0x80,[frac80+9](%bp,%di)		/normalized_   
one_left_norm_done:   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			subtraction_normalize:   
/   
/	function:   
/		normalizes following a subtraction.   
/   
/	inputs:   
/		assumes fraction is not zero.  %di = frac   
/   
/	outputs:   
/		shifts fraction left and decrements exponent   
/		until fraction normalized.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

subtraction_normalize:
		call	one_left_normalize		/ normalize one bit   
		jz	subtraction_normalize		/quit when normalized.  
		ret   
/   
/ move_10_bytes group   
/   
move_frac2_frac1:   
		mov	$offset_operand1,%di   
move_frac2:   
		mov	$offset_operand2,%si   
		jmp	move_10_bytes   
move_frac1_result:   
		mov	$offset_result,%di   
		jmp	move_frac1   
move_frac1_cop:   
		mov	$offset_cop,%di   
		call	move_frac1   
move_frac1_frac2:   
		mov	$offset_operand2,%di   
move_frac1:   
		mov	$offset_operand1,%si   
		jmp	move_10_bytes   
move_dop_frac2:   
		mov	$offset_operand2,%di   
		jmp	move_dop   
move_dop_frac1:   
		mov	$offset_operand1,%di   
move_dop:   
		mov	$offset_dop,%si   
		jmp	move_10_bytes   
move_cop_frac2:   
		mov	$offset_operand2,%di   
		jmp	move_cop   
move_cop_frac1:   
		mov	$offset_operand1,%di   
move_cop:   
		mov	$offset_cop,%si   
		jmp	move_10_bytes   
move_result_cop:   
		mov	$offset_cop,%di   
		jmp	move_result   
move_result_dop:   
		mov	$offset_dop,%di   
		jmp	move_result   
move_result_frac2:   
		mov	$offset_operand2,%di   
		jmp	move_result   
move_result_frac1:   
		mov	$offset_operand1,%di   
move_result:   
		mov	$offset_result,%si   
move_10_bytes:   
		add	%bp,%si		/ add global record   
		push	%ds		/ save a_msr   
 		push	%ss		/ load source segment register   
set_destination:   
		pop	%ds   
 		push	%ss		/ load destination segment register   
		pop	%es		/ into %es   
		add	%bp,%di		/ add global record   
		mov	$5,%cx		/ move five words   
		rep	
		smov
		pop	%ds		/ reload a_msr   
		ret   
/   
/ add, sub, mul, move_constant group   
/   
add_half1_frac2:   
		mov	$add_half_pattern_1,%si   
		call	add_frac2   
		mov	$offset_result,%di   
		jmp	addition_normalize   
add_frac2:   
		call	move_constant_frac2   
		jmp	addx   
sub_frac1:   
		mov	$offset_operand1,%di   
sub_constant:   
		call	move_constant   
		jmp	subx   
sub_frac2:   
		mov	$offset_operand2,%di   
		jmp	sub_constant   
mul_frac2:   
		call	move_constant_frac2   
		jmp	mulx   
move_constant_frac2:   
		mov	$offset_operand2,%di   
move_constant:   
		push	%ds		/ save a_msr   
 		push	%cs		/ load code segment base into %ds   
		jmp	set_destination	/ move five words   
