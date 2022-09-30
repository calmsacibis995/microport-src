		.file	"dcode.s"


/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/ 			d c o d e .m o d    
/			================   
/   
/	================================================================   
/               intel corporation proprietary information   
/       this software  is  supplied  under  the  terms  of  a  license
/       agreement  or non-disclosure agreement  with intel corporation
/       and  may not be copied nor disclosed except in accordance with
/       the terms of that agreement.
/	================================================================   
/
/         @(#)dcode.s	1.1 - 85/09/06
/   
/	function:   
/		converts the 80287 instruction into information   
/               about the first operand, information about the   
/               second operand, information about the result 
/               and the operation type.  defines mem_operand_ptr.   
/   
/	.globl:   
/		e80287   
/   
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

#include   "e80287.h"   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	address table for instruction group decoding handlers   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/ 10-9-8  (mod = 3)::1   

group_handler:	.value	arith_short_real	/  0 0 0  0   
		.value	arith_top_reg		/  0 0 0  1   
		.value	load_short_real		/  0 0 1  0   
		.value	transcendentals		/  0 0 1  1   
		.value	arith_short_int		/  0 1 0  0   
		.value	reserved		/  0 1 0  1   
		.value	load_short_int		/  0 1 1  0   
		.value	administrative		/  0 1 1  1   
		.value	arith_long_real		/  1 0 0  0   
		.value	arith_reg_top		/  1 0 0  1   
		.value	load_long_real		/  1 0 1  0   
		.value	store_reg		/  1 0 1  1   
		.value	arith_word_int		/  1 1 0  0   
		.value	arith_reg_pop		/  1 1 0  1   
		.value	load_word_int		/  1 1 1  0   
		.value	transfer_status		/  1 1 1  1   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	address table for r/m decoding handlers   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
rm_handler:	.value	rm_0			/ ea = (%bx)+(%si) + disp   
		.value	rm_1			/ ea = (%bx)+(%di) + disp   
		.value	rm_2			/ ea = (%bp)+(%si) + disp   
		.value	rm_3			/ ea = (%bp)+(%di) + disp   
		.value	rm_4			/ ea = (%si)+disp   
		.value	rm_5			/ ea = (%di)+disp   
		.value	rm_6			/ ea = (%bp)+disp*   
		.value	rm_7			/ ea = (%bx)+disp   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	address table for instruction operations   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
op_table:	.value	load
		.value	store
		.value	chsign
		.value	compar
		.value	pop_free
		.value	arith
		.value	arith
		.value	arith   
		.value	arith
		.value	ldcw
		.value	save_um
		.value	restore
		.value	pop_free
		.value	abs_value
		.value	init   
		.value	fxtrac
		.value	log
		.value	splog
		.value 	exp
		.value	tan
		.value	arctan
		.value	sqrt
		.value	remr
		.value	intpt
		.value	scale   
		.value	exchange
		.value	pop_free
		.value	restore_status
		.value	stenv
		.value	stcw
		.value	stsw   
		.value	load_con
		.value	load_con
		.value	load_con
		.value	load_con
		.value	load_con
		.value	load_con   
		.value	load_con
		.value	decr_top
		.value	incr_top
		.value	clex
		.value	compar
		.value	exam   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	arithmetic operation type decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_0a_0b_2a_4a_4b_6a_6b_op:	.byte	add_op	/ fadd     0 0 0   
		.byte	mul_op			/ fmul     0 0 1   
		.byte	compar_op 		/ fcom     0 1 0   
		.byte	compar_pop		/ fcomp    0 1 1   
		.byte	sub_op			/ fsub     1 0 0   
		.byte	subr_op			/ fsubr    1 0 1   
		.byte	div_op			/ fdiv     1 1 0   
		.byte	divr_op			/ fdivr    1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	control word/environment operation type decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_1a_op:	.byte	load_op			/ fld      0 0 0   
		.byte	error_op		/ reserved 0 0 1   
		.byte	store_op		/ fst      0 1 0   
		.byte	store_pop		/ fstp     0 1 1   
		.byte	ldenv_op		/ fldenv   1 0 0   
		.byte	ldcw_op			/ fldcw    1 0 1   
		.byte	stenv_op		/ fstenv   1 1 0   
		.byte	stcw_op			/ fstsw    1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	load/exchange/nop operation decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3-2-1-0   
group_1ba_op:	.byte	load_op			/ fld      0 0 0 r e g   
		.byte	exchange_op		/ fxch     0 0 1 r e g   
		.byte	null_op			/ fnop     0 1 1 r e g   
		.byte	store_pop		/ fstp*  0 1 1 r e g   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	transcendental operation decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3-2-1-0   
group_1bb_op:	.byte	chsign_op		/ fchs     1 0 0 0 0 0   
		.byte	abs_op			/ fabs     1 0 0 0 0 1   
		.byte	error_op		/ reserved 1 0 0 0 1 0   
		.byte	error_op		/ reserved 1 0 0 0 1 1   
		.byte	test_op			/ ftst     1 0 0 1 0 0   
		.byte	exam_op			/ fxam     1 0 0 1 0 1   
		.byte	error_op		/ reserved 1 0 0 1 1 0   
		.byte	error_op		/ reserved 1 0 0 1 1 1   
		.byte	load_1_op		/ fld1     1 0 1 0 0 0   
		.byte	load_l2t_op		/ fldl2t   1 0 1 0 0 1   
		.byte	load_l2e_op		/ fldl2e   1 0 1 0 1 0   
		.byte	load_pi_op		/ fldpi    1 0 1 0 1 1   
		.byte	load_lg2_op		/ fldlg2   1 0 1 1 0 0   
		.byte	load_ln2_op		/ fldln2   1 0 1 1 0 1   
		.byte	load_0_op		/ fldz     1 0 1 1 1 0   
		.byte	error_op		/ reserved 1 0 1 1 1 1   
		.byte	exp_op			/ f2xm1    1 1 0 0 0 0   
		.byte	log_op			/ fyl2x    1 1 0 0 0 1   
		.byte	tan_op			/ fptan    1 1 0 0 1 0   
		.byte	arctan_op		/ fpatan   1 1 0 0 1 1   
		.byte	fxtrac_op		/ fxtract  1 1 0 1 0 0   
		.byte	error_op		/ reserved 1 1 0 1 0 1   
		.byte	decstp_op		/ fdecstp  1 1 0 1 1 0   
		.byte	incstp_op		/ fincstp  1 1 0 1 1 1   
		.byte	remr_op			/ fprem    1 1 1 0 0 0   
		.byte	splog_op		/ fyl2xp1  1 1 1 0 0 1   
		.byte	sqrt_op			/ fsqrt    1 1 1 0 1 0   
		.byte	error_op		/ reserved 1 1 1 0 1 1   
		.byte	intpt_op		/ frndint  1 1 1 1 0 0   
		.byte	scale_op		/ fscale   1 1 1 1 0 1   
		.byte	error_op		/ reserved 1 1 1 1 1 0   
		.byte	error_op		/ reserved 1 1 1 1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	tempreal/bcd/short integer/long integer operation decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_3a_7a_op:	.byte	load_op			/ fild     0 0 0   
		.byte	error_op		/ reserved 0 0 1   
		.byte	store_op		/ fist     0 1 0   
		.byte	store_pop		/ fistp    0 1 1   
		.byte	load_op			/ (fbld)   1 0 0   
		.byte	load_op			/ f(i)ld   1 0 1   
		.byte	store_pop		/ (fbstp)  1 1 0   
		.byte	store_pop		/ f(i)stp  1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	clear exception/initialize operation decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3-2-1-0   
group_3b_op:	.byte	null_op			/ feni     1 0 0 0 0 0   
		.byte	null_op			/ fdisi    1 0 0 0 0 1   
		.byte	cler_op			/ fclex    1 0 0 0 1 0   
		.byte	init_op			/ finit    1 0 0 0 1 1   
		.byte	null_op			/ fsetpm   1 0 0 1 0 0   
		.byte	error_op		/ reserved 1 0 0 1 0 1   
		.byte	error_op		/ reserved 1 0 0 1 1 0   
		.byte	error_op		/ reserved 1 0 0 1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	restore/save operation decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_5a_op:	.byte	load_op			/ fld      0 0 0   
		.byte	error_op		/ reserved 0 0 1   
		.byte	store_op		/ fst      0 1 0   
		.byte	store_pop		/ fstp     0 1 1   
		.byte	restore_op		/ frstor   1 0 0   
		.byte	error_op		/ reserved 1 0 1   
		.byte	save_op			/ fsave    1 1 0   
		.byte	stsw_op			/ fstsw    1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	free register/store operation decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_5b_op:	.byte	free_op			/ ffree    0 0 0   
		.byte	exchange_op		/ fxch*  0 0 1   
		.byte	store_op		/ fst      0 1 0   
		.byte	store_pop		/ fstp     0 1 1   
		.byte	error_op		/ reserved 1 0 0   
		.byte	error_op		/ reserved 1 0 1   
		.byte	error_op		/ reserved 1 1 0   
		.byte	error_op		/ reserved 1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	transfer status operation decode table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_7b_op:	.byte	free_op			/ ffree* 0 0 0   
		.byte	exchange_op		/ fxch*  0 0 1   
		.byte	store_pop		/ fstp*  0 1 0   
		.byte	store_pop		/ fstp*  0 1 1   
		.byte	stsw_op			/ fstsw    1 0 0   
		.byte	error_op		/ reserved 1 0 1   
		.byte	error_op		/ reserved 1 1 0   
		.byte	error_op		/ reserved 1 1 1   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	load/store environment/control word operand 1 loc/for table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						         /          5-4-3   
group_1a_op1_lf: .value	[[memory_opnd\*0x100]+single_real] / fld      0 0 0   
		 .value	[[null\*0x100]+null]	         / reserved 0 0 1   
		 .value	[[stack_top\*0x100]+extended_fp]   / fst      0 1 0   
		 .value	[[stack_top\*0x100]+extended_fp]   / fstp     0 1 1   
		 .value	[[null\*0x100]+null]               / fldenv   1 0 0   
		 .value	[[null\*0x100]+null]               / fldcw    1 0 1   
		 .value	[[null\*0x100]+null]		 / fstenv   1 1 0   
		 .value	[[null\*0x100]+null]	         / fstcw    1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/       transcendental operand/result format codes   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
   
#define	null_fmt	0x00			/ null format   
#define top_fmt		0x01
#define topm1_fmt	0x02
#define topp1_fmt	0x03
#define op1		0x40
#define op2		0x10
#define res1		0x04
#define res2		0x01

/	transcendental operand/result format types   

#define	fprem_type	[[op1\*top_fmt]+[op2\*topm1_fmt]+[res1\*top_fmt]]
#define	fyl2x_type	[[op1\*top_fmt]+[op2\*topm1_fmt]+[res1\*topm1_fmt]]  
#define	fdecstp_type	null_fmt
#define	fchs_type	[[op1\*top_fmt]+[res1\*top_fmt]]
#define	fld1_type	[res1\*topp1_fmt]
#define	ftst_type	[op1\*top_fmt]
#define	fptan_type	[[op1\*top_fmt]+[res1\*top_fmt]+[res2\*topp1_fmt]]
#define	fpatan_type	[[op1\*topm1_fmt]+[op2\*top_fmt]+[res1\*topm1_fmt]]   
#define	fscale_type	[[op1\*topm1_fmt]+[op2\*top_fmt]+[res1\*top_fmt]]
   
/	operand/result format/location table   
   
group_1bb_lf:	.value	null					/ null_fmt   
		.value	[[stack_top\*0x100]+extended_fp]	/ top_fmt   
		.value	[[stack_top_minus_1\*0x100]+extended_fp] / topm1_fmt   
		.value	[[stack_top_plus_1\*0x100]+extended_fp]	/ topp1_fmt   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	transcendental operand/result format/location type table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3-2-1-0   
group_1bb_type:	.byte	fchs_type		/ fchs     1 0 0 0 0 0   
		.byte	fchs_type		/ fabs     1 0 0 0 0 1   
		.byte	fdecstp_type		/ reserved 1 0 0 0 1 0   
		.byte	fdecstp_type		/ reserved 1 0 0 0 1 1   
		.byte	ftst_type		/ ftst     1 0 0 1 0 0   
		.byte	ftst_type		/ fxam     1 0 0 1 0 1   
		.byte	fdecstp_type		/ reserved 1 0 0 1 1 0   
		.byte	fdecstp_type		/ reserved 1 0 0 1 1 1   
		.byte	fld1_type		/ fld1     1 0 1 0 0 0   
		.byte	fld1_type		/ fldl2t   1 0 1 0 0 1   
		.byte	fld1_type		/ fldl2e   1 0 1 0 1 0   
		.byte	fld1_type		/ fldpi    1 0 1 0 1 1   
		.byte	fld1_type		/ fldlg2   1 0 1 1 0 0   
		.byte	fld1_type		/ fldln2   1 0 1 1 0 1   
		.byte	fld1_type		/ fldz     1 0 1 1 1 0   
		.byte	fdecstp_type		/ reserved 1 0 1 1 1 1   
		.byte	fchs_type		/ f2xm1    1 1 0 0 0 0   
		.byte	fyl2x_type		/ fyl2x    1 1 0 0 0 1   
		.byte	fptan_type		/ fptan    1 1 0 0 1 0   
		.byte	fpatan_type		/ fpatan   1 1 0 0 1 1   
		.byte	fptan_type		/ fxtract  1 1 0 1 0 0   
		.byte	fdecstp_type		/ reserved 1 1 0 1 0 1   
		.byte	fdecstp_type		/ fdecstp  1 1 0 1 1 0   
		.byte	fdecstp_type		/ fincstp  1 1 0 1 1 1   
		.byte	fprem_type		/ fprem    1 1 1 0 0 0   
		.byte	fyl2x_type		/ fyl2xp1  1 1 1 0 0 1   
		.byte	fchs_type		/ fsqrt    1 1 1 0 1 0   
		.byte	fdecstp_type		/ reserved 1 1 1 0 1 1   
		.byte	fchs_type		/ frndint  1 1 1 1 0 0   
		.byte	fscale_type		/ fscale   1 1 1 1 0 1   
		.byte	fdecstp_type		/ reserved 1 1 1 1 1 0   
		.byte	fdecstp_type		/ reserved 1 1 1 1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	load/store short integer/tempreal operand 1 format table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_3a_op1_fmt: .byte	int32			/ fild     0 0 0   
		.byte	null			/ reserved 0 0 1   
		.byte	extended_fp		/ fist     0 1 0   
		.byte	extended_fp		/ fistp    0 1 1   
		.byte	null			/ reserved 1 0 0   
		.byte	extended_fp		/ fld      1 0 1   
		.byte	null			/ reserved 1 1 0   
		.byte	extended_fp		/ fstp     1 1 1   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	load/store integer/tempreal/bcd operand 1 location table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_3a_7a_op1_loc: .byte	memory_opnd	/ fild     0 0 0   
		.byte	null			/ reserved 0 0 1   
		.byte	stack_top		/ fist     0 1 0   
		.byte	stack_top		/ fistp    0 1 1   
		.byte	memory_opnd		/ (fbld)   1 0 0   
		.byte	memory_opnd		/ f(i)ld   1 0 1   
		.byte	stack_top		/ (fbstp)  1 1 0   
		.byte	stack_top		/ f(i)stp  1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	load/store short integer/tempreal result format table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_3a_res_fmt: .byte	extended_fp		/ fild     0 0 0   
		.byte	null			/ reserved 0 0 1   
		.byte	int32			/ fist     0 1 0   
		.byte	int32			/ fistp    0 1 1   
		.byte	null			/ reserved 1 0 0   
		.byte	extended_fp		/ fld      1 0 1   
		.byte	null			/ reserved 1 1 0   
		.byte	extended_fp		/ fstp     1 1 1   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	load/store integer/tempreal/bcd result location table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_3a_7a_res_loc: .byte	stack_top	/ fild     0 0 0   
		.byte	null			/ reserved 0 0 1   
		.byte	memory_opnd		/ fist     0 1 0   
		.byte	memory_opnd		/ fistp    0 1 1   
		.byte	stack_top_plus_1	/ (fbld)   1 0 0   
		.byte	stack_top_plus_1	/ f(i)ld   1 0 1   
		.byte	memory_opnd		/ (fbstp)  1 1 0   
		.byte	memory_opnd		/ f(i)stp  1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	load/store state/status word operand 1 loc/for table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						         /          5-4-3   
group_5a_op1_lf: .value	[[memory_opnd\*0x100]+double_real] / fld      0 0 0   
		 .value	[[null\*0x100]+null]		 / reserved 0 0 1   
		 .value	[[stack_top\*0x100]+extended_fp]   / fst      0 1 0   
		 .value	[[stack_top\*0x100]+extended_fp]   / fstp     0 1 1   
		 .value	[[null\*0x100]+null]               / frstor   1 0 0   
		 .value	[[null\*0x100]+null]		 / reserved 1 0 1   
		 .value	[[null\*0x100]+null]		 / fsave    1 1 0   
		 .value	[[null\*0x100]+null]		 / fstsw    1 1 1   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	bcd/long integer operand 1 format table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_7a_op1_fmt: .byte	int16			/ fild     0 0 0   
		 .byte	null			/ reserved 0 0 1   
		 .byte	extended_fp		/ fist     0 1 0   
		 .byte	extended_fp		/ fistp    0 1 1   
		 .byte	bcd			/ fbld     1 0 0   
		 .byte	int64			/ fild     1 0 1   
		 .byte	extended_fp		/ fbstp    1 1 0   
		 .byte	extended_fp		/ fistp    1 1 1   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	bcd/long integer result format table   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
						/          5-4-3   
group_7a_res_fmt: .byte	extended_fp		/ fild     0 0 0   
		 .byte	null			/ reserved 0 0 1   
		 .byte	int16			/ fist     0 1 0   
		 .byte	int16			/ fistp    0 1 1   
		 .byte	extended_fp		/ fbld     1 0 0   
		 .byte	extended_fp		/ fild     1 0 1   
		 .byte	bcd			/ fbstp    1 1 0   
		 .byte	int64			/ fistp    1 1 1   

		.text

		.globl	load
		.globl	store
		.globl	chsign
		.globl	compar
		.globl	arith
		.globl	ldcw	
		.globl	save_um
		.globl	restore
		.globl	pop_free
		.globl	abs_value
		.globl	init
		.globl	log
		.globl	fxtrac
		.globl	splog
		.globl	exp
		.globl	tan
		.globl	arctan
		.globl	sqrt
		.globl	remr
		.globl	intpt
		.globl	scale
		.globl	exchange
		.globl	restore_status
		.globl	stenv
		.globl	stcw
		.globl	stsw
		.globl	stsw
		.globl	load_con
		.globl	decr_top
		.globl	incr_top
		.globl	clex
		.globl	exam
		.globl	fetch_an_op
	
		.globl	emul

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			e80287:   
/   
/	function:   
/		executive program for 80287 emulator.   
/		decode instruction, i.e. separate operation,operand   
/		and result info; get memory operand pointer.   
/   
/	inputs:   
/		instruction (in memory).   
/   
/	outputs:   
/		op1, op2, operation, result, result2, and memory operand   
/		pointer info   
/   
/	data accessed:   
/		- op1_format			op1_location   
/		- op1_use_up			op2_format   
/		- op2_location			op2_use_up   
/		- operation_type		result_location   
/		- result_format			result2_format   
/		- result2_location   
/   
/	data changed:   
/		- mem_operand_pointer		op1_location   
/		- op1_format			op1_use_up   
/		- op2_format			op2_location   
/		- result_format			result2_format   
/		- result2_location   
/   

emul:						/ EMUL Start
		push	%bp			/ save orig base pointer
		mov	%sp,%bp			/
		mov	8(%bp),%si		/ get ptr to saved user regs
		mov	6(%bp),%ax		/ emulator data segment
		push	%ds			/ save environment
		push	%es
		pusha
		push	%si			/ remember it
		sub	global_reent_seg_length,%sp / reserve stack space   
		mov	%sp,%bp			/set bp to global_reentrant_seg 
		mov	%ax,saved_em_ds(%bp)	/ save emulator data segment
		mov	%ss:42(%si),%ax		/ get user flags
		mov	%ax,saved_flags(%bp)	/
		mov	%ss:40(%si),%ax		/ get user CS
		mov	%ax,saved_cs(%bp)	/
		mov	%ss:38(%si),%ax		/ get user IP
		mov	%ax,saved_ip(%bp)	/
		mov	%ss:2(%si),%ax		/ get usr DS
		mov	%ax,saved_ds(%bp)	/
		mov	%ss:0(%si),%ax		/ get usr ES
		mov	%ax,saved_es(%bp)	/
		mov	%ss:18(%si),%ax		/ get usr AX
		mov	%ax,saved_ax(%bp)	/
		mov	%ss:16(%si),%ax		/ get usr CX
		mov	%ax,saved_cx(%bp)	/
		mov	%ss:14(%si),%ax		/ get usr DX
		mov	%ax,saved_dx(%bp)	/
		mov	%ss:12(%si),%ax		/ get usr BX
		mov	%ax,saved_bx(%bp)	/
		mov	%ss:46(%si),%ax		/ get usr SS
		mov	%ax,saved_sp(%bp)	/
		mov	%ss:8(%si),%ax		/ get usr BP
		mov	%ax,saved_bp(%bp)	/
		mov	%ss:6(%si),%ax		/ get usr SI
		mov	%ax,saved_si(%bp)	/
		mov	%ss:4(%si),%ax		/ get usr DI
		mov	%ax,saved_di(%bp)	/
		cld				/ d-flag clear for all e80287   
		xor	%dx,%dx
		mov	%dx,%cx
		movb	$null,%dl		/ initialize op1 and op2   
		movb	%dl,%cl			/  format registers to null   
		mov	$do_nothing\*0x100+do_nothing,%bx /initialize useups
	 	lds	saved_ip(%bp),%si
get_opcode:   
		mov	(%si),%ax		/ load first two bytes   
		inc	%si			/ increment the pointer   
		andb	$0x87,%al		/ is the first byte a prefix_   
		jns	get_opcode		/ yes, skip for now   
		addb	$0x40,%ah		/ no, add 1 to mod   
		rclb	$1,%al			/ cf set if mod = 3   
		rolb	$1,%al			/ form handler table index   
		mov	$0x3800,%si		/ form bits 5-4-3 index in %si  
		and	%ax,%si   
		shr	$11,%si   
		mov	$0x001e,%di		/ form group handler table   
		and	%ax,%di			/  index in %di   
		push	%cs			/ copy cs to %ds for addressing 
		pop	%ds			/  all the tables   
		call	*%cs:group_handler(%di)	/ call the proper group handler 
		andb	$0x07,%ah		/ store register number (r/m)   
		mov	%ax,operation_type(%bp) / and operation type   
		mov	%dx,op1_format(%bp) / store operand and result   
		mov	%cx,op2_format(%bp) 	/ formats and locations   
		mov	%si,result_format(%bp)   
		mov	%di,result2_format(%bp)   
		movb	%bh,op1_use_up(%bp)	/ store op1 and op2 useups   
		movb	%bl,op2_use_up(%bp)   
		mov	saved_cs(%bp),%ax
		mov	%ax,%ds
		mov	saved_ip(%bp),%si  	/ load instruction pointer   
		mov	%si,instruction_pointer(%bp) / save old pointer   
		mov	%ds,[instruction_pointer+2](%bp)   
		xor	%cx,%cx			/ extended addressing = 'false' 
segment_ds:   
		mov	saved_ds(%bp),%dx
get_mod_rm_byte:   
		mov	(%si),%ax		/ load first two bytes again   
		inc	%si			/ bump instruction pointer   
 		andb	%al,%al			/ is there a segment override?  
		js	got_mod_rm		/ no, %ah contains mod r/m byte
		dec	%cx			/ yes, boolean = 'true'   
		mov	saved_es(%bp),%dx	/ load %es: override   
		cmpb	$0x26,%al		/ is it %es: ?   
		je	get_mod_rm_byte		/ yes, scan past prefix byte   
		mov	saved_cs(%bp),%dx	/ no, load %cs: override   
		cmpb	$0x2e,%al		/ is it %cs: ?   
		je	get_mod_rm_byte		/ yes, scan past prefix byte   
		mov	saved_sp(%bp),%dx	/ no, load %ss: override   
		cmpb	$0x36,%al		/ is it ss: ?   
		je	get_mod_rm_byte		/ yes, scan past prefix byte   
 		jmp	segment_ds		/ no, assume ds: override   
got_mod_rm:   
		inc	%si			/ bump instruction pointer   
		movb	%ah,%bl			/ save mod r/m in %bl   
		mov	(%si),%ax		/ load possible displacement   
		push	%ax			/ save momentarily
		mov	saved_em_ds(%bp),%ax	/ emulator segment
		push	%ax
		pop	%ds
		pop	%ax			/ restore ax

		cmpb	$0xc0,%bl		/ is mod = 3?   
		jnc	store_return		/ yes, no memory operand   
		shlb	$1,%bl			/ no, is mod = 2_   
		jc	mod_2			/ yes, disp = disp-hi: disp-lo 
		js	mod_1			/ branch if mod = 1   
		andb	$0x0e,%bl		/ mod = $0   
		cmpb	$0x0c,%bl		/ does r/m = 6?   
		je	store_ea		/ yes, ea = disp-hi: disp-lo   
		xor	%ax,%ax			/ no, disp = 0   
		dec	%si			/ subtract one for return   
mod_1:   
		dec	%si			/ subtract one for return   
		cbw				/ extend disp to 16 bits   
mod_2:   
		and	$0x000e,%bx		/ %ax now = real displacement   
		call	*%cs:rm_handler(%bx)	/ call r/m handler (%cs <> %ds) 
store_ea:   
		inc	%si			/ add two for return address   
		inc	%si   
		mov	%dx,[mem_operand_pointer+2](%bp) / store base   
		mov	%ax,mem_operand_pointer(%bp) / store   

		testb	$0x80,operation_type(%bp) / update sr_memop pointer?   
		jnz	store_return		/ no, must be administrative   
		mov	%dx,sr_mem_base		/ yes, store base in a_msr   
		mov	%ax,sr_mem_offset	/ store in a_msr   
store_return:   
		mov	%si,saved_ip(%bp)	/ update return address   
		mov	offset_op1_rec,%si	/set up for fetch_an_op   
		mov	$offset_operand1,%di   
		call	fetch_an_op		/ fetch op1   
		push	%ax			/save stack error flag   
		mov	offset_op2_rec,%si	/set up for fetch_an_op   
		mov	$offset_operand2,%di   
		call	fetch_an_op		/ fetch op2   
		pop	%bx			/ combine stack error flags   
		or	%bx,%ax			/ set/reset zf, clear cf   
		movb	operation_type(%bp),%bl / load operation type   
		rclb	$1,%bl			/ shift save ptrs flag to cf   
        	jc	do_operation		/ branch if administrative ins  
		les	instruction_pointer(%bp),%ax / load instruction ptr   
		mov	%ax,sr_instr_offset	/ store in status reg   
		mov	%es,sr_instr_base	/ store base in status reg   
do_operation:   
		call	*%cs:op_table(%bx)	/ call instruction operator   
		movb	sr_masks,%al		/set zf to false if any   
		notb	%al			/ unmasked errors
		andb	sr_errors,%al		/set %al to 0 if none   
		jz	release_stack   
		movb	$0xff,%al		/ load nonzero ie mask   
release_stack:   
		mov	saved_ax(%bp),%cx
		mov	saved_ip(%bp),%bx
		/ movb	[saved_flags+1](%bp),%dl
		add	global_reent_seg_length,%sp / release global area   
		pop	%si
		mov	%bx,%ss:38(%si)		/ restore IP
		mov	%cx,%ss:18(%si)		/ restore %ax
 		/ andb	%dl,%al 		/ interrupt enabled?
		cmpb	$0x00,%al
		je	no_exception		/ branch if no exception   
 		lcall	emul_signal_fpe
no_exception:   
		popa
		pop	%es
		pop	%ds

		leave
		lret				/ exit 80287 emulator   
rm_0:   
		add	saved_bx(%bp),%ax	/ ea = (%bx)+(%si) + disp   
rm_4:   
		add	saved_si(%bp),%ax	/ ea = (%si)+disp   
		ret   
rm_5:   
		add	saved_di(%bp),%ax	/ ea = (%di)+disp   
		ret   
rm_1:   
		add	saved_di(%bp),%ax	/ ea = (%bx)+(%di) + disp   
rm_7:   
		add	saved_bx(%bp),%ax	/ ea = (%bx)+disp   
		ret   
rm_2:   
		add	saved_si(%bp),%ax	/ ea = (%bp)+(%si) + disp   
rm_6:   
		add	saved_bp(%bp),%ax	/ ea = (%bp)+disp   
		jcxz	segment_ss		/ branch if no override given   
		ret   
rm_3:   
		add	saved_di(%bp),%ax	/ ea = (%bp)+(%di) + disp   
		jmp	rm_6			/ merge with r/m = 6   
segment_ss:   
		mov	saved_sp(%bp),%dx	/ for r/m = 2, 3, or 6, the   
		ret				/ standard base = %ss   


/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	instruction group decoding handlers   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	these handlers load all necessary operation, operand, and result   
/	information into the following registers for storage into the   
/	global data records upon return:   
/   
/	 %dh => op1_location		dl => op1_format (null)   
/	 %ch => op2_location		cl => op2_format (null)   
/	 %si => result_location/result_format   
/	 %di => result2_location/result2_format   
/	 %bh => op1_use_up (do_nothing)	bl => op2_use_up (do_nothing)   
/	 %ah => (modb r/m, reg byte)	al => operation_type   
/   
/	parentheses indicate initial values upon entry.   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/   
/	group 0a-standard arithmetic instructions with short-real operands   
/   
arith_short_real:   
		movb	$single_real,%cl	/ op2 format is real   
op1_top_op2_mem_0a:   
		movb	$memory_opnd,%ch	/ op2 loc is memory operand   
op1_top_stand_arith_op_0a:   
		movb	%cs:group_0a_0b_2a_4a_4b_6a_6b_op(%si),%al / arithmetic op   
		mov	%dx,%di			/ result2 format is null   
		mov	$[[stack_top\*0x100]+extended_fp],%dx /op1 extended_fp   
		mov	%dx,%si			/ result is same as op1   
arith_op_0a:   
		cmpb	$subr_op,%al		/ is it fsubr _   
		je	fsubr_0a		/ reverse operands   
		cmpb	$divr_op,%al		/ is it fdivr _   
		je	fdivr_0a		/ reverse operands   
		cmpb	$compar_pop,%al		/ is it fcomp _   
		jne	exit_0a			/ no, return   
		movb	$compar_op,%al		/ yes, op type is compare   
		movb	pop_stack,%bh		/ op1 useup is pop stack   
exit_0a:   
		ret   
fsubr_0a:   
		movb	$sub_op,%al		/ operation type is subtract   
		jmp	swap_0a		/ switch operand locations   
fdivr_0a:   
		movb	$div_op,%al		/ operation type is division   
swap_0a:   
		xchgb	%bl,%bh			/ switch operand useups   
		xchg	%cx,%dx			/ switch operand fmt/loc   
		ret   
/   
/	group 2a-standard arithmetic instructions with integer operands   
/   
arith_short_int:   
		movb	$int32,%cl		/ op2 format is integer   
		jmp	op1_top_op2_mem_0a	/ finish loading information   
/   
/	group 4a-standard arithmetic instructions with long-real operands   
/   
arith_long_real:   
		movb	$double_real,%cl	/ op2 format is long real   
		jmp	op1_top_op2_mem_0a	/ load rest of information   
/   
/	group 6a-standard arithmetic instructions with word-integer operands   
/   
arith_word_int:   
		movb	$int16,%cl		/ op2 format is word integer   
		jmp	op1_top_op2_mem_0a	/ load rest of information   
/   
/	group 0b-standard arithmetic instructions with inner-stack operands   
/   
arith_top_reg:   
		mov	$[[reg\*0x100]+extended_fp],%cx / op2 is reg extended_fp   
		jmp	op1_top_stand_arith_op_0a / load rest of information   
/   
/	group 6b-reversed arithmetic with register operand and pop useup   
/   
arith_reg_pop:   
		movb	pop_stack,%bl		/ op2 useup is pop stack   
/   
/	group 4b-reversed arithmetic instructions with inner-stack operands   
/   
arith_reg_top:   
		movb	%cs:group_0a_0b_2a_4a_4b_6a_6b_op(%si),%al / arithmetic op's
		mov	%dx,%di			/ result2 format is null   
		mov	$[[stack_top\*0x100]+extended_fp],%dx /op1extended_fp   
		mov	$[[reg\*0x100]+extended_fp],%cx /op2 is reg extended_fp 
		mov	%cx,%si			/ result is same as op2   
		jmp	arith_op_0a		/ load rest of information   
/   
/	group 1a-load/store instructions with short-real operand   
/		   load/store environment/control word with string operand   
/   
load_short_real:   
		movb	%cs:group_1a_op(%si),%al	/ load operation type   
		shl	$1,%si			/ change byte index to word   
		mov	%cs:group_1a_op1_lf(%si),%dx	/ load op1 loc/for   
		mov	$[[memory_opnd\*0x100]+single_real],%si / result format   
load_store_1a:   
		mov	%cx,%di			/ result2 format is null   
		cmpb	$load_op,%al		/ is it fld_   
		je	fld_1a			/ result goes to stack top   
		cmpb	$store_pop,%al		/ is it fstp_   
		je	fstp_1a			/ op1 useup is pop stack   
		cmpb	$store_op,%al		/ is it fst_   
		je	fst_1a			/ result goes to memory   
		mov	%di,%si			/ else, result format is null   
		ret   
fld_1a:   
		mov	$[[stack_top_plus_1\*0x100]+extended_fp],%si /result top 
		ret   
fstp_1a:   
		movb	pop_stack,%bh		/ op1 useup is pop stack   
		movb	$store_op,%al		/ operation type is store   
fst_1a:   
		ret   
/   
/	group 3a-integer/tempreal load/store instructions   
/   
load_short_int:   
		movb	%cs:group_3a_7a_op(%si),%al	/ load operation type   
		movb	%cs:group_3a_res_fmt(%si),%dl	/ load result format   
		movb	%cs:group_3a_7a_res_loc(%si),%dh / load result location   
		push	%dx			/ push result fmt/loc   
		movb	%cs:group_3a_op1_fmt(%si),%dl	/ load op1 format   
		movb	%cs:group_3a_7a_op1_loc(%si),%dh / load op1 location   
		pop	%si			/ pop result fmt/loc   
		jmp	load_store_1a		/ finish loading information   
/   
/	group 5a-load/store instructions with long-real operand   
/                  load/store state/status word instructions   
/   
load_long_real:   
		movb	%cs:group_5a_op(%si),%al	/ load operation type   
		shl	$1,%si			/ change byte index to word   
		mov	%cs:group_5a_op1_lf(%si),%dx	/ load op1 loc/for   
		mov	$[[memory_opnd\*0x100]+double_real],%si / result format   
		jmp	load_store_1a		/ load rest of information   
/   
/	group 7a-load/store word-integer/bcd/long-integer instructions   
/   
load_word_int:   
		movb	%cs:group_3a_7a_op(%si),%al	/ load operation type   
		movb	%cs:group_7a_res_fmt(%si),%dl	/ load result format   
		movb	%cs:group_3a_7a_res_loc(%si),%dh / load result location 
		push	%dx			/ push result fmt/loc   
		movb	%cs:group_7a_op1_fmt(%si),%dl	/ load op1 format   
		movb	%cs:group_3a_7a_op1_loc(%si),%dh / load op1 location   
		pop	%si			/ pop result fmt/loc   
		jmp	load_store_1a		/ finish loading information   
/   
/	group 1b-load/store/transcendental instructions with inner   
/		   stack operands   
transcendentals:   

		testb	$0x20,%ah		/ does bit 5 = 0?   
		jz	group_1ba		/ yes, it's a load/store op   
		mov	$0x1f00,%si		/ no, it's a transcendental   
		and	%ax,%si			/ calculate new table index   
		shr	$8,%si			/ shift right one byte   
		movb	%cs:group_1bb_type(%si),%al / load format/location type 
		rolb	$1,%al			/ rotate one left   
		mov	$0x0004,%cx		/ load loop count   
load_lf_1bb:   
		rolb	$2,%al			/ shift next field into %di   
		mov	$0x0006,%di		/ form table index   
		and	%ax,%di   
		push	%cs:group_1bb_lf(%di)	/ stack next format/location   
		loop	load_lf_1bb		/ decode four fields   
		movb	%cs:group_1bb_op(%si),%al	/ load operation type   
		cmpb	$log_op,%al		/ is it fyl2x_   
		je	pop_op1_1bb		/ yes, operand 1 gets popped   
		cmpb	$splog_op,%al		/ is it fyl2xp1_   
		je	pop_op1_1bb		/ yes, operand 1 gets popped   
		cmpb	$arctan_op,%al		/ is it fpatan_   
		jne	return_1bb		/ no, load return information   
pop_op1_1bb:   
		movb	pop_stack,%bh		/ pop operand 1   
return_1bb:   
		pop	%di			/ load operand 1 information   
		pop	%si			/ load operand 2 information   
		pop	%cx			/ load result 1 information   
		pop	%dx			/ load result 2 information   
		ret   
group_1ba:   
 		mov	%cx,%di			/ assume result2 always null   
		movb	%cs:group_1ba_op(%si),%al	/ load operation type   
load_store_1ba:   
		cmpb	$load_op,%al		/ is it fld st(i) _   
		je	fld_1ba			/ op1 is a register   
		cmpb	$exchange_op,%al	/ is it fxch st(i) _   
		je	fxch_1ba		/ op1 is stack top   
		cmpb	$store_pop,%al		/ is it fstp st(i)_ *   
		je	fstp_1ba		/ pop op1   
		cmpb	$store_op,%al		/ is it fst st(i)_   
		je	fst_1ba			/ don't pop op1   
		mov	%di,%si			/ result format is null   
		ret   
fld_1ba:   
		mov	$[[reg\*0x100]+extended_fp],%dx / op1 is a stack reg   
		mov	$[[stack_top_plus_1\*0x100]+extended_fp],%si /result
		ret   
fxch_1ba:   
		mov	$[[stack_top\*0x100]+extended_fp],%dx /op1 is stack
		mov	$[[reg\*0x100]+extended_fp],%cx / op2 is a stack reg   
		mov	%cx,%si			/ result is the stack reg   
		mov	%dx,%di			/ result2 is the stack top   
		ret   
fstp_1ba:   
		movb	$store_op,%al		/ operation type is store   
		movb	pop_stack,%bh		/ op1 useup is pop stack   
fst_1ba:   
		mov	$[[stack_top\*0x100]+extended_fp],%dx /op1 is stack
		mov	$[[reg\*0x100]+extended_fp],%si / result is stack reg   
		ret   
/   
/	group 5b-free register/store to register instructions   
/   
store_reg:   
		movb	%cs:group_5b_op(%si),%al	/ load operation type   
test_ffree_5b:   
		cmpb	$free_op,%al		/ is it ffree st(i)_   
		jne	load_store_1ba		/ no, must be fst(p)/fxch   
		movb	free,%bh		/ yes, op1 useup is free   
		jmp	load_store_1ba		/ load rest of information   
/   
/	group 7b-transfer status instruction   
/   
transfer_status:   
		movb	%cs:group_7b_op(%si),%al	/ load operation type   
		cmpb	$free_op,%al		/ is it ffreep st(i) _   
		jne	not_ffreep_7b		/ no, useup is do nothing   
		movb	pop_stack,%bh		/ yes, op1 useup is pop   
not_ffreep_7b:   
		cmpb	$stsw_op,%al		/ is it fstsw %ax _   
		jne	test_ffree_5b		/ no, handle like group 5b   
		movb	$reg,%ch		/ yes, op2 loc is 'reg'   
		jmp	test_ffree_5b		/ finish loading information   
/   
/	group 2b-reserved   
/   
reserved:   
		movb	$error_op,%al		/ illegal operation   
		lcall	emul_signal_ill
		pop	%cx			/ discard return word
		jmp	release_stack		/ bad goto
null_results_2b:   
		mov	%cx,%si			/ result is null   
		mov	%cx,%di			/ result2 is null   
		ret   
/   
/	group 3b-administrative instructions   
/   
administrative:   
		mov	$0x0700,%si		/ calculate new table index   
		and	%ax,%si   
		shr	$8,%si			/ shift right one byte   
		movb	%cs:group_3b_op(%si),%al	/ load operation type   
		jmp	null_results_2b		/ result and result2 are null   

