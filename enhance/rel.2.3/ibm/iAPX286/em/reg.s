	.file	"reg.s"

	.ident	"@(#)reg.s	1.2"



/
/   
/			r e g . m o d   
/			=============   
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
/		operation cluster for the 80287 register stack.   
/   
/	.globl:   
/		put_result	stack_full_	float_int16   
/		save_regs	restore_regs	set_up_indefinite   
/		getx		fetch_an_op	exchange	exam   
/		fxtrac		abs_value	chsign		put_op1_result   
/		put_si_result	do_exchange   
/   
/	internal:   
/		extend_single	extend_double	get_bcd		fetch_an_op   
/		float16		float32		float64   
/   
#include   "e80287.h"   

		.text

		.globl	getx
		.globl	fetch_an_op
		.globl	put_result
		.globl	stack_full_
		.globl	fxtrac   
		.globl	float_int16
		.globl	float16
		.globl	save_um
		.globl	save_regs
		.globl	restore_regs
		.globl	restore   
		.globl	set_up_indefinite
		.globl	exchange
		.globl	exam
		.globl	decompose
		.globl	load   
		.globl	abs_value
		.globl	chsign
		.globl	put_op1_result
		.globl	do_exchange   
		.globl	put_si_result   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""  
/			extend_single:   
/			""""""""""""""   
/	function:   
/		retrieves a single real operand from memory
/		extends its parts, and puts them in the operand   
/   
/	inputs:   
/		assumes mem_operand_pointer set up   
/		di points to operand1 or operand2   
/   
/	outputs:   
/		extended operand in operand   
/   
/	data accessed:   
/		- mem_operand_pointer   
/   
/	data changed:   
/		- operands   
/   
/	procedures:   
/		set_d_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

extend_single:
		les	mem_operand_pointer(%bp),%bx   
		xor	%ax,%ax   
		mov	%ax,frac64(%bp,%di)	/clear low 5 bytes of   
		mov	%ax,[frac64+2](%bp,%di)	/fraction to 0   
		movb	%al,[frac64+4](%bp,%di)
		mov	%es:(%bx),%ax 			/ move 3 fraction   
		mov	%ax,[frac64+5](%bp,%di)    	/ bytes from memory   
		mov	%es:2(%bx),%ax   
		movb	%al,[frac64+7](%bp,%di)
		cwd					/ transmit sign to %dx  
		movb	%dl,sign(%bp,%di)		/ transfer to operand   
		and	$0x7f80,%ax   
		shr	$7,%ax				/get single real expon  
		jnz	single_invalid_   
		mov	[frac64+5](%bp,%di),%ax		/zero or denormalized_  
		orb	[frac64+7](%bp,%di),%al
		jnz	single_denormalized   
		movb	special,tag(%bp,%di)		/set tag to special
		mov	%ax,expon(%bp,%di)		/expon to 0   
		ret   
single_denormalized:   
		movb	valid,tag(%bp,%di)       	/set tag to valid   
		mov	single_exp_offset+1,expon(%bp,%di)
		jmp	set_d_error			/set d-error   
single_invalid_:   
		cmpb	true,%al   
		jne	single_operand_is_valid   
		mov	$0x7fff,expon(%bp,%di)		/operand is inv/inf   
		orb	$0x80,[frac64+7](%bp,%di)	/exp= $0x7fff, msb=1   
		cmp	$0,[frac64+5](%bp,%di)		/check for infinity   
		jne	single_invalid   
		cmpb	$0x80,[frac64+7](%bp,%di)
		jne	single_invalid   
		movb	infinty,tag(%bp,%di)		/operand is infinity   
		ret   
single_invalid:   
		movb	inv,tag(%bp,%di)		/invalid operand:   
		ret					/set tag to invalid   
single_operand_is_valid:   
		movb	valid,tag(%bp,%di)		/set tag to valid
		add	single_exp_offset,%ax		/single_exp_offset
		mov	%ax,expon(%bp,%di)    		/implicit bit to 1   
		orb	$0x80,[frac64+7](%bp,%di)
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			extend_double:   
/			""""""""""""""   
/	function:   
/		retrieves a double real operand from memory
/		extends its parts, and puts them in the operand   
/   
/	inputs:   
/		assumes mem_operand_pointer is set up   
/		di points to operand1 or operand2   
/   
/	outputs:   
/		expanded operand in operand   
/   
/	data accessed:   
/		- mem_operand_pointer   
/   
/	data changed:   
/		- operand   
/   
/	procedures:   
/		set_d_error   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

extend_double:
		push	%ds				/ save a_msr   
		lds	mem_operand_pointer(%bp),%bx   
		movb	$0,frac64(%bp,%di)		/clear low byte of   
		mov	(%bx),%ax			/fraction to 0 
		mov	%ax,[frac64+1](%bp,%di)    	/move 7 fraction bytes 
		mov	2(%bx),%ax			/from memory   
		mov	%ax,[frac64+3](%bp,%di)
		mov	4(%bx),%ax   
		mov	%ax,[frac64+5](%bp,%di)
		movb	6(%bx),%al   
		andb	$0x0f,%al			/clear leading bit now  
		movb	%al,[frac64+7](%bp,%di)
		push	%bx
		push	%di
		movb	$3,%cl				/bits to align for   
		call	left_shift			/x-format   
		pop	%di   
		pop	%bx   
		mov	6(%bx),%ax			/ load msw of memop   
		pop	%ds				/ reload a_msr   
		cwd					/ transmit sign to %dx  
		movb	%dl,sign(%bp,%di)		/ transfer to operand   
		and	$0x7ff0,%ax			/clear sign and frac   
		shr	$4,%ax				/align exponent bits   
		jnz	double_invalid_   
		mov	[frac64+1](%bp,%di),%ax		/exp=0 : is fraction 0_ 
		or	[frac64+3](%bp,%di),%ax 
		or	[frac64+5](%bp,%di),%ax 
		orb	[frac64+7](%bp,%di),%al   
		jnz	double_denormalized   
		movb	special,tag(%bp,%di)		/set tag to special
		mov	%ax,expon(%bp,%di)		/expon to zero   
		ret   
double_denormalized:   
		movb	valid,tag(%bp,%di)		/set tag to valid   
		mov	double_exp_offset+1,expon(%bp,%di)
		jmp	set_d_error			/set d-error   
double_invalid_:   
		cmp	$0x07ff,%ax   
		jne	double_operand_is_valid   
		mov	$0x7fff,expon(%bp,%di)		/operand is invalid or  
		mov	[frac64+1](%bp,%di),%ax		/infinity: set exponent 
		or	[frac64+3](%bp,%di),%ax		/to 0x7fff   
		or	[frac64+5](%bp,%di),%ax		/is it infinity_   
		orb	[frac64+7](%bp,%di),%al   
		movb	inv,%al   
		jnz	double_set_tag			/ if frac nonzero, inv  
		movb	infinty,%al			/ else, operand is inf  
		jmp	double_set_tag   
double_operand_is_valid:   
		add	double_exp_offset,%ax		/double_exp_  
		mov	%ax,expon(%bp,%di)
		movb	valid,%al			/ set tag to valid   
double_set_tag:   
		movb	%al,tag(%bp,%di)	  	/ set operand tag   
		orb	$0x80,[frac64+7](%bp,%di)	/set leading bit   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			getx:   
/			"""""   
/	function:   
/		fetches extended-format fp number from memory, unpacks it
/		and sets its tag   
/   
/	inputs:   
/		assumes mem_operand_pointer is set up   
/		di points to operand1 or operand2   
/   
/	outputs:   
/		if number is valid, then exponent and msb set to all ones   
/   
/	data accessed:   
/		- mem_operand_pointer		extra_word_reg   
/   
/	data changed:   
/		- operand			extra_word_reg   
/   
/	procedures:   
/		test_4w				test_3w   
/		set_d_error   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

getx:
		push	%ds				/ save a_msr   
		lds	mem_operand_pointer(%bp),%si	/ %ds:si ==> memop   
 		movb	true,extra_word_reg(%bp)	 / ok to set d-error   
getx_stackreg:   
		push	%ss   
		pop	%es				/ %es:di ==> operand   
		inc	%di				/ bump %di to frac64   
		inc	%di   
		push	%di				/ save operand   
		lea	(%bp,%di),%di   
		mov	$0x0004,%cx			/ move first four words 
		rep	     				/  to frac64   
		smov
		mov	(%si),%ax			/ get sign/expon word   
		pop	%di				/ reload operand   
		pop	%ds				/ reload a_msr   
		cwd					/ transmit sign to %dx  
		movb	%dl,[sign-frac64](%bp,%di)	/  and to operand   
		and	$0x7fff,%ax			/ strip off sign bit   
		mov	%ax,[expon-frac64](%bp,%di)	/ store to operand   
		jz	expon_zero			/ branch if exponent 0 
		cmp	$0x7fff,%ax			/ check for invalid   
		je	not_validx			/  or infinity   
		movb	valid,%dl		 	/ operand is valid   
getx_set_tag:   
		movb	%dl,[tag-frac64](%bp,%di)	/set operand tag   
		xor	%ax,%ax				/ (for stackreg return)
		ret   
not_validx:   
		and	6(%bp,%di),%ax		/  = x000000000000000b_ 
		movb	inv,%dl			/if msw and 0x7fff<>0 
		jnz	getx_set_tag			/  set tag to invalid   
		call	test_3w				/ if 3 lsw's <> 0   
		jnz	getx_set_tag			/ set tag to invalid   
		movb	infinty,%dl 			/ else, tag infinity   
		jmp	getx_set_tag   
expon_zero:   
		call	test_4w   
		movb	special,%dl			/ set tag to special   
		jz	getx_set_tag			/ if number is +/- 0   
		movb	denormd,%dl			/  set tag to denormd   
 		testb	%al,extra_word_reg(%bp)	/ ok to set d-error_   
		jz	getx_set_tag			/ no, must be stack op  
		call	set_d_error			/ yes, raise d-error   
		jmp	getx_set_tag			/ set tag to denormd   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			fetch_an_op:   
/			"""""""""""   
/	function:   
/		fetches one operand(op1 or op2) and puts its parts   
/ 		in operand1 or operand2   
/   
/	inputs:   
/		location of op info in %si   
/		location of the operand in %di   
/   
/	outputs:   
/		operand parts in operand1 or operand2   
/   
/	procedures:   
/		mem_fetch_routine		set_up_indefinite   
/		get_top				set_i_error   
/		getx_stackreg   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

fetch_location:		.value from_memory
			.value from_stack_top
			.value from_stack_minus_1
   			.value from_stack_plus_1
			.value from_reg   
mem_fetch_routine:	.value extend_single
			.value extend_double
			.value float16
			.value float32
   			.value float64
			.value getx
			.value float16
			.value float16
			.value get_bcd   

fetch_an_op:

		cmpb	$null,format(%bp,%si)		/if format = null
		je	no_stack_error_on_fetch		/then no operand to get 
		xor	%bx,%bx   
		mov	%bx,frac80(%bp,%di)		/set low order word of  
		movb	location(%bp,%si),%bl		/fraction to zero   
		shl	$1,%bx				/handle each operand   
		jmp	*%cs:fetch_location(%bx)	/location case separate 
from_memory:   
		movb	format(%bp,%si),%bl		/form table   
		shl	$1,%bx   
		call	*%cs:mem_fetch_routine(%bx)	/call routine to fetch  
no_stack_error_on_fetch:   
		xor	%ax,%ax				/ %ax = 0x0000   
		ret   
from_stack_top:   
		call	get_top				/ get reg num in %al   
unpack_reg:   
		call	reg_full_			/ zf=1 (empty) cx=top   
		jz	from_stack_plus_1		/ error if reg empty   
		mov	$sr_regstack,%si / NEED OFFSET set pointer to stack  
		add	%cx,%si				/ = (10*regnum)   
		shl	$2,%cx				/ +regstack   
		add	%cx,%si				/ %ds:si ==> regstack   
 		movb	%ch,extra_word_reg(%bp)	/ don't raise d-error   
		push	%ds				/ stack a copy of a_msr 
		jmp	getx_stackreg			/ unpack register   
from_stack_minus_1:   
		call	get_top   
		inc	%ax   
		jmp	unpack_reg   
from_reg:   
		call	get_top   
		addb	reg_num(%bp),%al   
		jmp	unpack_reg   
from_stack_plus_1:   
		call	set_up_indefinite		/ store indefinite opr  
		inc	%ax				/ stack error on return
		jmp	set_i_error			/ set invalid error   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			get_bcd:   
/			""""""""   
/	function:   
/		fetches a bcd number and converts it to an unpacked    
/		tempreal in operand1.   
/	inputs:   
/		assumes mem_operand_pointer is set up.   
/		ignores %di and uses operand2 and result fields.   
/   
/	outputs:   
/		unpacked operand in operand1   
/   
/	data accessed:   
/		- mem_operand_pointer		expon1   
/		- offset_operand1		sign1   
/		- tag1				word_frac1   
/		- offset_operand1			word_frac2   
/		- lsb__frac2			offset_operand2   
/		- result_word_frac		offset_result   
/   
/	data changed:   
/		- sign1				tag1   
/		- expon1			word_frac1   
/		- word_frac2			result_word_frac   
/   
/	procedures:   
/		left_shift			move_op_to_result   
/		subtraction_normalize		test_5w   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
get_bcd:
		push	%ds			/ save a_msr   
		lds	mem_operand_pointer(%bp),%si   
		push	%ss			/ set up destination pointer   
		pop	%es   
		lea	word_frac2(%bp),%di	/ %es:di ==> word_frac2   
		mov	$0x0005,%cx   
		rep	      		/ move bcd fraction to frac2   
		smov
		pop	%ds			/ reload a_msr   
		movb	msb_frac2(%bp),%al	/ get most significant byte   
		cbw				/  and transmit sign to %ah   
		movb	%ah,sign1(%bp)		/ store sign into operand   
		mov	$offset_operand1,%di	/ set frac1 to 0   
		call	clear_5w   
		movb	$18,%cl			/set loop count to $18 digits   
convert_loop:   
		push	%cx   
		movb	$4,%cl			/shift frac2 left 4 bits to   
		call	left_shift_frac2_cl	/ the next digit   
		andb	$0x0f,msb_frac2(%bp)	/clear previous digit   
		call	left_shift_frac1_1	/ frac1 <= 10*frac1+frac2(9):   
		mov	$offset_operand1,%si	/ 1) frac1 <= frac1*2   
		call	move_op_to_result	/ 2) result_frac <= frac1*4   
		movb	$2,%cl   
		call	left_shift_result_cl   
		mov	result_word_frac(%bp),%ax / 3) frac1 <-- frac1+  
		add	%ax,word_frac1(%bp)	 /	result_frac   
		mov	[result_word_frac+2](%bp),%ax   
		adc	%ax,[word_frac1+2](%bp)   
		mov	[result_word_frac+4](%bp),%ax   
		adc	%ax,[word_frac1+4](%bp)   
		mov	[result_word_frac+6](%bp),%ax   
		adc	%ax,[word_frac1+6](%bp)   
		mov	[result_word_frac+8](%bp),%ax   
		adc	%ax,[word_frac1+8](%bp)   
		xorb	%ah,%ah			/ 4) frac1 <-- frac1+frac(9)   
		movb	msb_frac2(%bp),%al   
		add	%ax,word_frac1(%bp)   
		movb	$0,%al   
		adc	%ax,[word_frac1+2](%bp)   
		adc	%ax,[word_frac1+4](%bp)   
		adc	%ax,[word_frac1+6](%bp)   
		adc	%ax,[word_frac1+8](%bp)   
		pop	%cx			/and loop   
		loop	convert_loop   
		mov	$word_frac1,%di   	/its offset
		call	test_5w			/frac1 = 0_   
		jnz	make_floating_point   
		mov	%ax,expon1(%bp) 	/set expon and tag for zero   
		movb	special,tag1(%bp)   
		ret   
make_floating_point:   
		mov	$0x404e,expon1(%bp)	/set expon1 and normalize   
		mov	$offset_operand1,%di   
		call	subtraction_normalize   
		movb	valid,tag1(%bp)		/set tag to valid   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""" 
/			put_result:   
/   
/	function:   
/		puts result on 80287 stack or in 80287 register.   
/		implements 80287 load instruction.   
/   
/	inputs:   
/		address of data parts  (in %di register)   
/		address of result info (in %si).   
/   
/	outputs:   
/		stack error indication (in %al register).   
/   
/	data accessed:   
/   
/	data changed:   
/   
/	procedures:   
/		decr_top		stack_full_   
/		set_up_indefinite	store_reg_tag   
/		get_top			i_masked_   
/		set_i_masked_   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

location_case:	.value	push_onto_stack   
		.value	put_in_stack_top   
		.value	put_in_stack_top_minus_1   
		.value	push_onto_stack   
		.value	put_in_reg   
   
load:
		jz	put_op1_result		/ if zf not set, stack error   
		call	i_masked_   
		jnz	put_op1_result		/if error masked, continue   
		ret				/ else, return   
put_op1_result:   
		mov	$offset_operand1,%di	/di points to result record   
put_si_result:   
		mov	offset_result_rec,%si	/ merge with put_result   
put_result:   
		xorb	%bh,%bh			/handle each case separately   
		movb	location(%bp,%si),%bl   
		shl	$1,%bx   
		jmp	*%cs:location_case(%bx)   
put_in_stack_top_minus_1:   
		call	get_top			/put reg num of top-1 in %al   
		inc	%ax   
		jmp	mask_top   
push_onto_stack:   
		call	stack_full_		/ is top-1 full_   
		jz	push_value		/ no, so put on stack   
		call	set_i_masked_		/ yes, so stack error   
		jz	exit_load		/ unmasked, so done with load   
		call	set_up_indefinite	/push indefinite as the result   
push_value:   
		call	decr_top		/ decrement top for push   
put_in_stack_top:   
		call	get_top			/put reg num of stack top in %al
		jmp	mask_top   
put_in_reg:   
		call	get_top			/put reg num specified in   
		addb	reg_num(%bp),%al		/ instruction in %al   
mask_top:   
		and	$0x0007,%ax   
		mov	%ax,%cx			/ save reg num word in %cx   
		movb	tag(%bp,%di),%al	/ put new tag in %al   
		call	store_reg_tag   
		lea	frac64(%bp,%di),%si	/ %ds:si ==> operand frac64   
		mov	$sr_regstack,%di	/form offset within sr_regstack 
		add	%cx,%di			/for result (10*reg num)   
		shl	$2,%cx   
		add	%cx,%di			/ %es:di ==>  treal st(reg num) 
		movb	$0x04,%cl			/ count = 4 words   
 		push	%ds			/ set segment registers   
		pop	%es   
		push	%ss   
		pop	%ds   
		rep	     			/ fraction => sr_regstack   
		smov
		movb	(%si),%ah		/ %ah = sign of operand   
		and	$0x8000,%ax		/ form tempreal exponent   
		or	2(%si),%ax   
		push	%es			/ reload a_msr into %ds   
		pop	%ds   
		mov	%ax,(%di)		/ exponent => sr_regstack   
exit_load:   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""  
/			stack_full_:   
/   
/	function:   
/		checks stack to see if an attempted push will   
/		generate a stack error.   
/   
/	inputs:   
/		tag for register on top of stack   
/   
/	outputs:   
/		zf = true if stack is empty; zf = false if stack is full.   
/   
/	procedures:   
/		get_top			get_reg_tag   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
stack_full_:
		call	get_top			/determine reg no. of (top-1)   
		dec	%ax   
reg_full_:   
		andb	$0x07,%al   
		call	get_reg_tag   
		cmpb	empty,%al		/if (top-1) is not empty
return:   
		ret				/then stack is full   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			decompose:   
/   
/	function:   
/		decomposes op1 into two parts   
/   
/	inputs:   
/		assumes op1 is set up   
/   
/	outputs:   
/		puts the exponent, floated in op1; the fraction in op2   
/   
/	data accessed:   
/		- offset_operand		sign1   
/		- tag1				expon1   
/		- offset_operand2		sign2   
/		- tag2				expon2   
/   
/	data changed:   
/		- sign2				expon2   
/   
/	procedures:   
/		float_int16			move_op_to_op   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

decompose:
		lea	word_frac1(%bp),%si		/move op1 to op2   
		lea	word_frac2(%bp),%di   
		call	move_op_to_op   
		cmpb	special,tag2(%bp)		/if number=0, both   
		je	return				/ op1 and op2 are zero  
		mov	$exponent_bias,expon2(%bp)	/set op2 exponent=bias  
		mov	expon1(%bp),%ax		/float exp into op1   
		sub	$exponent_bias,%ax		/first unbias it   
		mov	$offset_operand1,%di   
		jmp	float_int16   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			float16:   
/   
/	function:   
/		retrieves a 16-bit, 2's complement integer from memory
/		and fills in the operand   
/   
/	inputs:   
/		assumes mem_operand_pointer is set up   
/		di points to operand1 or operand2   
/   
/	outputs:   
/		operand   
/   
/	data accessed:   
/		- mem_operand_pointer   
/   
/	data changed:   
/		- operand   
/   
/	procedures:   
/		clear_5w		subtraction_normalize   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

float16:
		les	mem_operand_pointer(%bp),%bx	/fetch the 16-bit int   
		mov	%es:(%bx),%ax   
float_int16:   
		mov	%ax,%dx				/ save integer in %dx   
		call	clear_5w			/ clear low 10 bytes   
		or	%dx,%ax				/ if integer is zero
		jnz	get_sign_and_mag_16		/  then load +0   
		mov	%ax,expon(%bp,%di)
		movb	%al,sign(%bp,%di)
		mov	%ax,[frac80+8](%bp,%di)
		movb	special,tag(%bp,%di)
		ret   
get_sign_and_mag_16:   
		cwd					/ transmit sign to %dx  
		movb	%dl,sign(%bp,%di)		/ and store to operand  
		jns	positive_int			/ branch if positive   
		neg	%ax				/ negate if negative   
positive_int:   
		mov	%ax,[frac80+8](%bp,%di)    	/ store abs(int)   
		mov	$0x400e,%ax			/ load valid exponent   
normalize_int:   
		mov	%ax,expon(%bp,%di)		/expon valid/unnormal   
		push	%di				/ save operand   
		call	subtraction_normalize		/ normalize   
		pop	%di				/ restore   
		movb	valid,tag(%bp,%di)
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			float32:   
/   
/	function:   
/		retrieves a 32-bit, 2's complement integer from memory   
/		and fills in the operand   
/   
/	inputs:   
/		assumes mem_operand_pointer is set up   
/		di points to operand1 or operand2   
/   
/	outputs:   
/		operand   
/   
/	data accessed:   
/		- mem_operand_pointer   
/   
/	data changed:   
/		- operand   
/   
/	precedures called:   
/		subtraction_normalize		clear_5w   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
float32:
		call	clear_5w			/clear low bytes frac   
		les	mem_operand_pointer(%bp),%bx   
		mov	%es:(%bx),%ax			/move from memory to   
		mov	%ax,[frac80+6](%bp,%di)    	/global record area   
		mov	%es:2(%bx),%ax   
		mov	%ax,[frac80+8](%bp,%di)
		cwd					/ transmit sign to %dx  
		movb	%dl,sign(%bp,%di)    		/ and store to operand  
		and	%ax,%ax				/ test for -fraction   
		jns	detect_zero_32   
		not	[frac80+6](%bp,%di)		/negative, so form 2's  
		add	$1,[frac80+6](%bp,%di)		/complement   
		not	[frac80+8](%bp,%di)   
		adc	$0,[frac80+8](%bp,%di)
detect_zero_32:   
		mov	[frac80+6](%bp,%di),%ax		/fraction = zero_   
		or	[frac80+8](%bp,%di),%ax 
		jnz	set_expon_32   
		mov	%ax,expon(%bp,%di)		/set exponent to zero   
		movb	special,tag(%bp,%di)
		ret   
set_expon_32:   
		mov	$0x401e,%ax			/ expon valid/unnormal  
		jmp	normalize_int			/ normalize   
set_expon_64:   
		mov	$0x403e,%ax			/expon valid/unnormal   
		jmp	normalize_int			/ normalize it   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""" 
/			float64:   
/   
/	function:   
/		retrieves a 64-bit, 2's complement integer from memory   
/		and fills in the operand   
/   
/	inputs:   
/		assumes mem_operand_pointer is set up   
/		di points to operand1 or operand2   
/   
/	outputs:   
/		operand   
/   
/	data accessed:   
/		- mem_operand_pointer   
/   
/	data changed:   
/		- operand   
/   
/	precedures called:   
/		subtraction_normalize		test_4w   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

float64:

		push	%ds				/ save a_msr   
		lds	mem_operand_pointer(%bp),%si   
		push	%ss   
		pop	%es				/ %es:di ==> operand   
		push	%di				/ save op_   
		lea	frac64(%bp,%di),%di		/ set operand   
		mov	$0x0004,%cx			/ move fraction from   
		rep	     	 	 		/  memory to operand   
		smov
		pop	%di				/ reload operand   
		pop	%ds				/ reload a_msr   
		mov	8(%bp,%di),%ax		/ get long integer sign 
		cwd					/ transmit sign to %dx  
		movb	%dl,sign(%bp,%di)		/  and store to operand 
		inc	%dx				/ test for '-' fract   
		jnz	detect_zero_64   
		not	2(%bp,%di)		/'-', so form 2's   
		add	$1,2(%bp,%di)   	/complement   
		not	4(%bp,%di)   
		adc	%dx,4(%bp,%di)
		not	6(%bp,%di)   
		adc	%dx,6(%bp,%di)
		not	8(%bp,%di)   
		adc	%dx,8(%bp,%di)
detect_zero_64:   
		xor	%ax,%ax   
		call	test_4w				/integer = zero_   
		jnz	set_expon_64   
		mov	%ax,expon(%bp,%di)		/set expon = zero   
		movb	special,tag(%bp,%di)
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			save_um:   
/   
/	function:   
/		implements 80287 save instruction   
/   
/	inputs:   
/   
/	outputs:   
/   
/	data accessed:   
/		- mem_operand_pointer		result_rec_  
/   
/	data changed:   
/   
/	procedures:   
/		save_status		save_regs	init   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

save_um:
		call	save_status   
		call	save_regs   
		jmp	init			/ initialize environment   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			save_regs:   
/   
/	function:   
/		saves all registers in contiguous memory locations.   
/   
/	inputs:   
/		memory operand pointer (in %es:(%di))   
/   
/	outputs:   
/   
/	data accessed:   
/   
/	data changed:   
/   
/	procedures:   
/		get_top   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

save_regs:
		movb	$5,%cl				/ 5 words per register 
		call	get_top				/ %ax = top*5   
		mulb	%cl   
		mov	$40,%cx				/ number words to move  
		sub	%ax,%cx				/ from top of stack =   
		mov	$sr_regstack,%si		/  NEED OFFSET 40-top*5 
		push	%si				/ starting address =   
		add	%ax,%si				/  sr_regstack+top*5   
		add	%ax,%si				/  *2 bytes/word   
		rep		       		/ move top of stack   
		smov
		mov	%ax,%cx				/ move (top*5) words   
		pop	%si				/ load starting address 
		rep	     				/ move rest of stack   
		smov
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""  
/			restore_regs:   
/   
/	function:   
/		loads all registers from contiguous memory locations   
/		implements 80287 restore instruction   
/   
/	inputs:   
/		mem_operand_pointer   
/   
/	outputs:   
/		loads the sr_regstack area of a_msr from memory   
/   
/	data accessed:   
/   
/	data changed:   
/		- operand   
/   
/	procedures:   
/		restore_status			get_top   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

restore:
		call	restore_status			/ this call defines %es   
restore_regs:   
		movb	$5,%cl				/ 5 words per register 
		call	get_top   
		push	%ds				/ save a_msr   
		mulb	%cl				/ %ax = top*5   
		mov	$40,%cx				/ number words to move  
		sub	%ax,%cx				/ from top of stack =   
		mov	$sr_regstack,%di		/  NEED OFFSET 40-top*5 
		push	%di				/ starting address =   
		add	%ax,%di				/  sr_regstack+top*5   
		add	%ax,%di				/  *2 bytes/word   
		lds	mem_operand_pointer(%bp),%si	/ source is in memory   
		add	$14,%si				/ skip the environment  
		rep	      	 		/ move top of stack   
		smov
		mov	%ax,%cx				/ move (top*5) words   
		pop	%di				/ load starting address 
		rep	     				/ move rest of stack   
		smov
		pop	%ds				/ reload a_msr   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""  
/			set_up_indefinite:   
/   
/	function:   
/		puts indefinite in the operand pointed to by %di   
/   
/	input:   
/		di points to operand   
/   
/	output:   
/		indefinite in operand   
/   
/	data accessed:   
/		- operand   
/   
/	data changed:   
/		- operand   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

set_up_indefinite:
		xor	%ax,%ax   
		mov	%ax,frac64(%bp,%di)     /set up indef fraction   
		mov	%ax,[frac64+2](%bp,%di)
		mov	%ax,[frac64+4](%bp,%di)
		mov	$0xc000,[frac64+6](%bp,%di)
		movb	negative,sign(%bp,%di) /set sign negative   
		mov	$0x7fff,expon(%bp,%di)  /set exponent to 0x7fff   
		movb	inv,tag(%bp,%di)	 /set tag to invalid   
common_return:   
		ret   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			exchange   
/   
/   
/	function:   
/		implements 80287 exchange instruction   
/   
/	inputs:   
/		operand(s) in operands   
/   
/	outputs:   
/		result(s) on stack   
/   
/	data accessed:   
/		- result_rec_offset		offset_operand1   
/   
/	data changed:   
/   
/	procedures:   
/		put_result			i_masked_   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

exchange:
		jz	do_exchange		/ if zf not 0, then stack error
		call	i_masked_   
		jz	common_return		/if unmasked error, do nothing  
do_exchange:   
		call	put_op1_result		/ put op1(top) in result(reg)   
put_second_result:   
		mov	$offset_operand2,%di	/ put op2(reg) in res2(top)   
		mov	offset_result2_rec,%si   
		jmp	put_result   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			exam:   
/   
/	function:   
/		implements 80287 fxam instruction   
/   
/	inputs:   
/		operand1   
/   
/	outputs:   
/		error indicators set   
/   
/	data accessed:   
/		- tag1				sign1   
/		- msb_frac1   
/   
/	data changed:   
/   
/	procedures:   
/		set_a_bit			set_c_bit   
/		set_z_bit			set_s_bit   
/		set_a_bit			clear_c_bit   
/		clear_cond_bits   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

exam:
		jz	something_on_stack	/ branch on no stack error   
		call	clear_c_bit		/ clear sticky c_bit   
		call	set_z_bit		/ set s and z to 1_1   
		jmp	set_s_bit   
something_on_stack:   
		call	clear_cond_bits		/ clear condition code bits   
		movb	tag1(%bp),%al   
		cmpb	inv,%al			/remove special cases   
		je	check_s_z_a_bits   
		cmpb	infinty,%al   
		je	set_c_to_one   
		cmpb	denormd,%al   
		jne	valid_or_zero   
		call	set_z_bit		/set s, z, c to 0, 1, 1   
		call	set_c_bit		/ for denormalized op   
		jmp	check_a_bit   
valid_or_zero:   
		testb	$0x80,msb_frac1(%bp)	/for valid or zero
		jz	check_s_z_a_bits	/ set c to msb of frac   
set_c_to_one:   
		call	set_c_bit   
check_s_z_a_bits:   
		rorb	$1,%al			/set z-bit to lsb of tag1   
		jnc	check_s_bit   
		call	set_z_bit   
check_s_bit:   
		rorb	$1,%al			/set s-bit to lsb-1 of tag1   
		jnc	check_a_bit   
		call	set_s_bit   
check_a_bit:   
		testb	$0x01,sign1(%bp)      		/set a-bit to sign bit  
		jz	common_return   
		jmp	set_a_bit   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			fxtrac:   
/   
/	function:   
/		implements the 80287 fxtrac instruction   
/   
/	inputs:   
/   
/	outputs:   
/		decomposed results in results   
/   
/	data accessed:   
/		- result_rec_offset		result2_rec_  
/		- offset_operand1		tag1   
/		- offset_operand2   
/   
/	data changed:   
/   
/	procedures:   
/		set_up_indefinite		decompose   
/		do_exchange			i_masked_   
/		set_i_masked_   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

fxtrac:
		jz	detect_i_error		/ branch if no stack error   
		call	i_masked_   
		jnz	put_indefs	/ branch if error masked   
		ret				/ return if unmasked error   
detect_i_error:   
		movb	tag1(%bp),%al		/ load operand tag   
		cmpb	special,%al   
		je	do_decompose   
		cmpb	valid,%al		/ if zero or valid, it's good   
		je	do_decompose   
		call	set_i_masked_		/invalid if nan, infinity   
		jz	common_exit		/ or denormd   
put_indefs:   
		mov	$offset_operand2,%di	/for masked invalid error
		call	set_up_indefinite	/set op2 to indefinite   
		jmp	put_second_result   
do_decompose:   
		call	decompose   
		jmp	do_exchange		/put both results   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			abs_value:   
/   
/	function:   
/		implements the 80287 absolute value instruction   
/   
/	inputs:   
/		assumes operand is set up   
/   
/	outputs:   
/		absolute value of operand in result   
/   
/	procedures:   
/		i_masked_   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

abs_value:
		jz	do_abs_value		/ branch if no stack error   
                call	i_masked_   
                jnz	abs_value_result	/masked error, so give indef   
common_exit:   
		ret				/ unmasked error   
do_abs_value:   
		movb	positive,sign1(%bp)	/set sign to positive   
abs_value_result:   
		jmp	put_op1_result		/put result on stack   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			chsign:   
/   
/	function:   
/		implement the 80287 chsign instruction   
/   
/	inputs:   
/		operand1   
/   
/	outputs:   
/		result <==  -(operand1)   
/   
/	procedures:   
/		i_masked_   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

chsign:
		jz	do_chsign		/ branch if no stack error   
                call	i_masked_   
                jz	common_exit   
                jmp     chsign_result   
do_chsign:   
		notb	sign1(%bp)		/complement sign   
chsign_result:   
		jmp	abs_value_result	/put op1 into result   
