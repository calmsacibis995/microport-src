/ @(#)e80287.h	1.1 - 85/09/06 /

/      Copyright (c) 1985 AT&T /
/        All Rights Reserved   /

/      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     /
/      The copyright notice above does not evidence any        /
/      actual or intended publication of such source code.     /

/...definition of global reentrant data segment    

#define 	global_reent_seg_length		$134   


#define	mem_operand_pointer	0	
#define	instruction_pointer	4	
#define	operation_type		8
#define	reg_num			9

#define	op1_format		10
#define	op1_location		11
#define	op1_use_up		12

#define	op2_format		13
#define	op2_location		14
#define	op2_use_up		15

#define	result_format		16
#define	result_location		17

#define	result2_format		18
#define	result2_location	19
   
#define	word_frac1		20
#define	sign1			30
#define	tag1			31
#define	expon1			32
   
#define	word_frac2		34
#define	sign2			44
#define	tag2			45
#define	expon2			46
   
#define	extra_word_reg		48
#define	result_word_frac	58
#define	result_sign		68
#define	result_tag		69
#define	result_expon		70
   
#define	result2_word_frac	72
#define	result2_sign		82
#define	result2_tag		83
#define	result2_expon		84
   
#define	cop			86
#define	dop			96
   
#define	saved_di		106
#define	saved_si		108
#define	saved_bp		110
#define	saved_sp		112
#define	saved_bx		114
#define	saved_dx		116
#define	saved_cx		118
#define	saved_ax		120
#define	saved_es		122
#define	saved_ds		124
#define	saved_ip		126
#define	saved_cs		128
#define	saved_flags		130
#define saved_em_ds		132
   
   
/	convenience definitions   
   
#define 	offset_op1_rec			$op1_format   
#define 	offset_op2_rec			$op2_format   
#define 	offset_result_rec		$result_format   
#define 	offset_result2_rec		$result2_format   
#define 	offset_operand1			word_frac1   
#define 	lsb_frac1			word_frac1   
#define 	msb_frac1			word_frac1+9   
#define 	offset_operand2			word_frac2   
#define 	lsb_frac2			word_frac2   
#define 	msb_frac2			word_frac2+9   
#define 	offset_result			result_word_frac   
#define 	lsb_result			result_word_frac   
#define 	msb_result			result_word_frac+9   
#define 	offset_result2			result2_word_frac   
#define 	lsb_result2			result2_word_frac   
#define 	msb_result2			result2_word_frac+9   
#define 	offset_cop			cop   
#define 	lsb_cop				cop   
#define 	msb_cop				lsb_cop+9   
#define 	offset_dop			dop   
#define 	lsb_dop				dop   
#define 	msb_dop				lsb_dop+9   
#define 	q				mem_operand_pointer(%bp)
#define 	exp_tmp				mem_operand_pointer(%bp)
#define 	siso				[mem_operand_pointer+2](%bp)   
#define 	loop_ct				[mem_operand_pointer+4](%bp)  
#define 	bit_ct				[mem_operand_pointer+6](%bp)
#define 	added_one			[mem_operand_pointer+7](%bp)

/...define structure of operand info and result info structures...   
   
#define 	format				0   
#define 	location			1   
#define 	use_up				2   
   
/...define format codes...   
   
#define 	single_real			0   
#define 	double_real			1   
#define 	int16				2   
#define 	int32				3   
#define 	int64				4   
#define 	extended_fp			5   
#define		null				7
#define 	bcd				8   
#define 	op_no_result			9   
#define 	no_op_result			10   
#define 	op2_pop				11   
#define 	op2_no_pop			12   
#define 	result2				13   
#define 	op1_top_op2			14   
   
/...define operation type codes...   
   
#define 	save_ptrs			0x80   
#define 	load_op				0   
#define 	store_op			1   
#define 	chsign_op			2   
#define 	compar_op			3   
#define 	error_op			4+save_ptrs   
#define 	add_op				5   
#define 	sub_op				6   
#define 	mul_op				7   
#define 	div_op				8   
#define 	ldcw_op				9+save_ptrs   
#define 	save_op				10+save_ptrs   
#define 	restore_op			11+save_ptrs   
#define 	null_op				12+save_ptrs   
#define 	abs_op				13   
#define 	init_op				14+save_ptrs   
#define 	fxtrac_op			15   
#define 	log_op				16   
#define 	splog_op			17   
#define 	exp_op				18   
#define 	tan_op				19   
#define 	arctan_op			20   
#define 	sqrt_op				21   
#define 	remr_op				22   
#define 	intpt_op			23   
#define 	scale_op			24   
#define 	exchange_op			25   
#define 	free_op				26   
#define 	ldenv_op			27+save_ptrs   
#define 	stenv_op			28+save_ptrs   
#define 	stcw_op				29+save_ptrs   
#define 	stsw_op				30+save_ptrs   
#define 	load_1_op			31   
#define 	load_l2t_op			32   
#define 	load_l2e_op			33   
#define 	load_pi_op			34   
#define 	load_lg2_op			35   
#define 	load_ln2_op			36   
#define 	load_0_op			37   
#define 	decstp_op			38   
#define 	incstp_op			39   
#define 	cler_op				40+save_ptrs   
#define 	test_op				41   
#define 	exam_op				42   
#define 	compar_pop			43   
#define 	subr_op				44   
#define 	divr_op				45   
#define 	store_pop			46   
   
/...definition of register tag codes...   
   
#define 	valid				$0   
#define		VALID				 0
#define 	special				$1   
#define 	inv				$2   
#define 	empty				$3   
#define		nempty				$0xfffc		/* ~empty */
#define 	denormd				$6   
#define 	infinty				$0x0a   
   
/...infinity_control settings...   
   
#define 	projective			$0   
#define 	affine				$1   
   
/...define location codes...   
   
#define 	memory_opnd			0   
#define 	stack_top			1   
#define 	stack_top_minus_1		2   
#define 	stack_top_plus_1		3   
#define 	reg				4   
   
/...define use_up codes...   
   
#define 	free				$0   
#define 	pop_stack			$1   
#define 	do_nothing			2   
   
/...definition of operand and result records in global reent seg...   
   
#define 	frac80				0   
#define 	frac64				2   
#define 	sign				10   
#define 	tag				11   
#define 	expon				12   
   
/...definition of precision codes...   
   
#define 	prec24				$0   
#define 	prec32				$1   
#define 	prec53				$2   
#define 	prec64				$3   
#define 	prec16				$4   
   
/...define round control codes...   
   
#define 	rnd_to_even			$0x0000   
#define 	rnd_down			$0x0004   
#define 	rnd_up				$0x0008   
#define 	rnd_to_zero			$0x000c   
   
/...define positive and negative...   
   
#define 	positive			$0x0000   
#define		POSITIVE			 0x0000
#define 	negative			$0x00ff   
   
/...definition of true and false...   
   
#define 	true				$0x00ff   
#define 	false				$0x0000   
   
#define 	wrap_around_constant		$0x6000   
#define 	no_change			$0x00f0   
#define 	exponent_bias 			0x3fff   
   
#define 	single_exp_offset		$0x3f80   
#define 	double_exp_offset		$0x3c00   
   
#define 	zero_mask               	$0x40   
#define		nzero_mask			$0xbf	/* ~zero_mask	*/
#define 	sign_mask               	$0x01   
#define		nsign_mask			$0xfe   /* ~sign_mask  	*/
#define 	a_mask                  	$0x02   
#define 	na_mask                  	$0xfd	/* ~a_mask	*/   
#define 	c_mask                  	$0x04   
#define 	nc_mask                  	$0xfb	/* ~c_mask	*/   
#define 	inexact_mask            	$0x20   
#define 	ninexact_mask            	$0xdf	/* ~inexact_mask*/   
#define 	underflow_mask          	$0x10   
#define 	overflow_mask           	$0x08   
#define 	zero_divide_mask        	$0x04   
#define 	invalid_mask            	$0x01   
#define 	denorm_mask             	$0x02   
#define 	top_mask                	$0x38   
#define 	ntop_mask                	$0xc7	/* ~top_mask	*/   
#define 	precision_mask          	$0x03   
#define 	nprecision_mask          	$0xfc	/* ~precision_mask */   
#define 	rnd_control_mask        	$0x0c   
#define 	nrnd_control_mask        	$0xf3	/* ~rnd_control_mask*/  
#define 	infinity_control_mask   	 $0x10   
#define 	high_extended_expon_for_single	 $0x407e   
#define 	low_extended_expon_for_single	 $0x3f81   
#define 	high_extended_expon_for_double	 $0x43fe   
#define 	low_extended_expon_for_double	 $0x3c01   
#define 	high_int16_exponent		 $0x400e   
#define 	max_int16_shift			 $17   
#define 	high_int32_exponent		 $0x401e   
#define 	max_int32_shift			 $33   
#define 	high_int64_exponent		 $0x403e   
#define 	max_int64_shift			 $65   

/* ~[c_mask | zero_mask | sign_mask | a_mask] 				*/
#define		ncombo_mask			 $0xb8


#define	sr_masks	146
#define sr_controls	147
#define sr_errors	148
#define sr_flags	149
#define	sr_tags		150
#define	sr_instr_offset	152
#define	sr_instr_base	154
#define	sr_mem_offset	156
#define	sr_mem_base	158
#define	sr_regstack	160


