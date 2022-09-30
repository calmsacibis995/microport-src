	.file	"round.s"

	.ident	"@(#)round.s	1.2"



/
/   
/			r o u n d . m o d   
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
/         @(#)round.s	1.1 - 85/09/06
/   
/	function:   
/		implements all rounding modes for all precisions   
/   
/	.globl:   
/		round			special_round_test   
/		directed_round_test   
/   
/	internal:   
/		prec16_case		prec24_case   
/		prec32_case		prec53_case   
/		prec64_case   
/   
/
/   

#include   "e80287.h"   

		.text

		.globl	round
		.globl directed_round_test
		.globl special_round_test   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			prec16_case:   
/			""""""""""""   
/	function:   
/		 sets up for rounding to 16 bits.   
/   
/	inputs:   
/		assumes %di points to operand record.   
/   
/	outputs:   
/		sets: %dl if round or sticky bit set   
/		      %dh if guard bit set   
/		      %ah if lsb set   
/		      %cl to number of rounded bytes   
/		      %ch to pattern to add to low byte for rounding   
/		and clears the guard, round, and sticky bytes.   
/		doesn't affect al.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/	procecures:   
/		not_ah_clear_32   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

prec16_case:					/initially, all guard

		xchg	[frac80+6](%bp,%di),%cx / round, and sticky info zero
		and	%cx,%cx		/examine and clear grst word   
		jns	get_round_sticky_16   
		notb	%dh			/set guard byte   
get_round_sticky_16:   
		and	$0x7fff,%cx   
		or	[frac80+4](%bp,%di),%cx   
		or	[frac80+2](%bp,%di),%cx   
		or	frac80(%bp,%di),%cx   
		jz	get_lsb_16   
		notb	%dl			/set round-sticky byte   
get_lsb_16:   
 		xorb	%ah,%ah			/initially, assume lsb is zero  
		testb	$0x01,[frac80+8](%bp,%di)
		jz	set_rnd_info_16   
		call	not_ah_clear_32		/set lsb byte   
		mov	$0x0000,[frac80+4](%bp,%di)
set_rnd_info_16:   
		mov	$0x0102,%cx		/the rounded result has $2 bytes
		ret				/the pattern to add is $0x01   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			prec24_case:   
/			""""""""""""   
/	function:   
/		sets up for rounding to 24 bits.   
/   
/	inputs:   
/		assumes %di points to operand record.   
/   
/	outputs:   
/		sets: %dl if round or sticky bit set   
/		      %dh if guard bit set   
/		      %ah if lsb set   
/		      %cl to number of rounded bytes   
/		      %ch to pattern to add to low byte for rounding   
/		and clears the guard, round, and sticky bytes.   
/		doesn't affect al.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

prec24_case:					/initially all guard
		xchg	[frac80+5](%bp,%di),%cx / round, and sticky info zero  
		and	%cx,%cx			/ examine and clear grst word   
		jns	get_round_sticky_24   
		notb	%dh			/set guard byte   
get_round_sticky_24:   
		and	$0x7fff,%cx   
		or	[frac80+3](%bp,%di),%cx   
		or	[frac80+1](%bp,%di),%cx   
		orb	frac80(%bp,%di),%cl   
		and	%cx,%cx   
		jz	get_lsb_24   
		notb	%dl				/set round-sticky byte   
get_lsb_24:   
 		xorb	%ah,%ah				/initially, assume lsb  
		testb	$0x01,[frac80+7](%bp,%di)       /is zero   
		jz	clear_sticky_bytes_24   
		notb	%ah				/set lsb byte   
clear_sticky_bytes_24:   
		xor	%cx,%cx   
		mov	%cx,[frac80+3](%bp,%di)
		mov	%cx,[frac80+1](%bp,%di)
		movb	%cl,frac80(%bp,%di)
		mov	$0x0103,%cx		/the rounded result has $3 bytes
		ret				/the pattern to add is $0x01   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""  
/			prec32_case:   
/			""""""""""""   
/	function:   
/		sets up for rounding to 32 bits.   
/   
/	inputs:   
/		assumes %di points to operand record.   
/   
/	outputs:   
/		sets: %dl if round or sticky bit set   
/		      %dh if guard bit set   
/		      %ah if lsb set   
/		      %cl to number of rounded bytes   
/		      %ch to pattern to add to low byte for rounding   
/		and clears the guard, round, and sticky bytes.   
/		doesn't affect al.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

prec32_case:					 /initially all guard
		xchg	[frac80+4](%bp,%di),%cx / round, and sticky info zero
		and	%cx,%cx   
		jns	get_round_sticky_32   
		notb	%dh			/set guard byte   
get_round_sticky_32:   
		and	$0x7fff,%cx   
		or	[frac80+2](%bp,%di),%cx   
		or	[frac80](%bp,%di),%cx   
		jz	get_lsb_32   
		notb	%dl			/set round-sticky byte   
get_lsb_32:   
 		xorb	%ah,%ah			/initially, assume lsb is zero  
		testb	$0x01,[frac80+6](%bp,%di)
		jz	clear_sticky_bytes_32   
not_ah_clear_32:   
		notb	%ah			/set lsb byte   
clear_sticky_bytes_32:   
		xor	%cx,%cx   
		mov	%cx,[frac80+2](%bp,%di)
		mov	%cx,frac80(%bp,%di)
		mov	$0x0104,%cx		/the rounded result has $4 bytes
		ret				/the pattern to add is $0x01   

/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			prec53_case:   
/			"""""""""""   
/	function:   
/		sets up for rounding to 53 bits.   
/   
/	inputs:   
/		assumes %di points to operand record.   
/   
/	outputs:   
/		sets: %dl if round or sticky bit set   
/		      %dh if guard bit set   
/		      %ah if lsb set   
/		      %cl to number of rounded bytes   
/		      %ch to pattern to add to low byte for rounding   
/		and clears the guard, round, and sticky bytes.   
/		doesn't affect al.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
prec53_case:					/initially all guard
     		mov	[frac80+2](%bp,%di),%cx  / round, and sticky info zero 
		testb	$0x04,%ch     			/examine guard bit   
		jz	get_round_sticky_53   
		notb	%dh			/set guard byte   
get_round_sticky_53:   
		and	$0x03ff,%cx   
		or	frac80(%bp,%di),%cx
		jz	get_lsb_53   
		notb	%dl			/set round-sticky byte   
get_lsb_53:   
 		xorb	%ah,%ah			/initially, assume lsb is zero  
		testb	$0x08,[frac80+3](%bp,%di)
		jz	clear_grst_bits_53   
		notb	%ah			/set lsb byte   
clear_grst_bits_53:   
		mov	$0,frac80(%bp,%di)
		and	$0xf800,[frac80+2](%bp,%di)
		mov	$0x0807,%cx		/the rounded result has $7 bytes
		ret				/the pattern to add is $0x08   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			prec64_case:   
/			""""""""""""   
/	function:   
/		sets up for rounding to 64 bits.   
/   
/	inputs:   
/		assumes %di points to operand record.   
/   
/	outputs:   
/		sets: %dl if round or sticky bit set   
/		      %dh if guard bit set   
/		      %ah if lsb set   
/		      %cl to number of rounded bytes   
/		      %ch to pattern to add to low byte for rounding   
/		and clears the guard, round, and sticky bytes.   
/		doesn't affect al.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

prec64_case:					/initially all guard
		xchg	frac80(%bp,%di),%cx	/round, and sticky info zero   
		and	%cx,%cx			/ examine and clear grst word   
		jns	get_round_sticky_64   
		notb	%dh			/set guard byte   
get_round_sticky_64:   
		and	$0x7fff,%cx   
		jz	get_lsb_64   
		notb	%dl			/set round-sticky byte   
get_lsb_64:   
 		xorb	%ah,%ah			/initially, assume lsb is zero  
		testb	$0x01,[frac80+2](%bp,%di)
		jz	set_round_info_64   
		notb	%ah			/set lsb byte   
set_round_info_64:   
		mov	$0x0108,%cx		/rounded result has $8 bytes   
common_return:   
		ret				/the pattern to add is $0x01   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			round:   
/			"""""   
/	function:   
/		implements rounding.   
/   
/	inputs:   
/		assumes:di points to record to round 
/		        %dl contains rounding precision
/			al is true if second rounding, false if first.   
/   
/	outputs:   
/		on  %al,return is true if there was a carry-out   
/		from the rounding.   
/   
/	data accessed:   
/		added_one   
/   
/	data changed:   
/		added_one   
/   
/	procedures:   
/		prec16_case			prec24_case   
/		prec32_case			prec53_case   
/		prec64_case			clear_p_error   
/		get_rnd_control			p_error_   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

precn_case:	.value prec24_case
		.value prec32_case
		.value prec53_case
		.value prec64_case
		.value prec16_case   

rnd_case:	.value case_rnd_to_even
		.value case_rnd_down
		.value case_rnd_up
		.value case_rnd_to_zero   
/   
round:						/ unpacked status info   
		xorb	%dh,%dh			/ set to zero the following:   
		mov	%dx,%si			/ cl to num bytes in rnd frac   
		shl	$1,%si			/ %ch to pattern to add to low  
		xor	%dx,%dx			/  byte in rounding   
		xor	%cx,%cx			/ dl if round or sticky set   
		call	*%cs:precn_case(%si)	/ dh if grdb set,ah if lsb set 
		and	%dx,%dx			/if dl and %dh both zero, then 
		jnz	inexact_case		/result is exact   
		movb	false,%al		/ no carry out from rounding   
		movb	%al,added_one		/clear flag, no overflow   
		ret   
inexact_case:   
		push	%ax			/ save lsb/2nd rounding flags   
		call	get_rnd_control		/round different rnd_control   
		cbw				/ cases separately   
		mov	%ax,%bx   
		pop	%ax			/ reload lsb/2nd rounding flags 
		shr	$1,%bx   
		jmp	*%cs:rnd_case(%bx)   
case_rnd_up:   
		notb	%bh			/if+increment, else truncate   
case_rnd_down:   
		cmpb	%bh,sign(%bp,%di) 	/increment if neg
		jne	do_increment		/ truncate if pos   
		jmp	case_rnd_to_zero   
case_rnd_to_even:   
		movb	%dl,%bh			/save round-sticky info in %bh  
		orb	%ah,%dl			/if guard and (round or sticky  
		cmpb	true,%al		/ or lsb), set %dx to round up  
		jne	do_add_			/if 2nd round, if ((lsb=added)  
		movb	added_one,%bl		/ and not (round or sticky)   
		call	p_error_		/ and ((added=1) or last round  
		jz	p_error_0		/ inexact)), round in opposite  
		movb	true,%bl   
p_error_0:   
		cmp	$0x00ff,%bx		/ direction   
		jne	do_add_   
		cmpb	added_one,%ah   
		jne	do_add_			/ branch if %ah <> added_one   
		inc	%dx   
		jz	case_rnd_to_zero	/ truncate   
		jmp	do_increment   
do_add_:   
		inc	%dx   
		jz	do_increment   
case_rnd_to_zero:   
		movb	false,%al		/clear flag, indicate nothing   
		movb	%al,added_one		/ added in round (%al = false)  
		jmp	set_inexact   
do_increment:   
		movb	true,added_one		/set flag for $1 added in round 
		movb	%ch,%dl			/put pattern to add in %dl   
		xorb	%ch,%ch			/ %cx = num bytes in rounded res
		mov	$10,%bx   
		sub	%cx,%bx			/ %bx =  num of low byte   
		add	%bx,%di			/ %di = true   
		xorb	%al,%al   
		addb	%dl,frac80(%bp,%di)	/add pattern to low byte and   
		inc	%di			/ propogate carries   
		dec	%cx   
inc_loop:   
		adcb	%al,frac80(%bp,%di)
		inc	%di   
		loop	inc_loop   
		sbbb	%al,%al			/set %al if carry set   
set_inexact:   
		jmp	set_p_error   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			special_round_test:   
/			""""""""""""""""""   
/	function:   
/		test for rounding toward zero.   
/   
/	inputs:   
/		assumes sign to be in %al upon entry.   
/   
/	outputs:   
/		returns true (in %al) if:   
/		  (rnd_down and sign is positive) or   
/		  (rnd_up and sign is negative).   
/   
/	data accessed:   
/   
/	data changed:   
/   
/	procedures:   
/		get_rnd_control   
/   
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

special_round_test:
		movb	%al,%ah				/ save sign   
		call	get_rnd_control			/get rounding mode   
		cmpb	rnd_down,%al
		jne	round_up_	
		andb	%ah,%ah   
		jz	is_true				/ true if +rnd_down   
is_false:   
		xorb	%al,%al   
		ret   
round_up_:   
		cmpb	rnd_up,%al   
		jne	is_false			/ branch if not up   
		andb	%ah,%ah   
		jz	is_false			/ branch if +rnd_up   
is_true:   
		orb	true,%al   
exit_round_test:   
		ret   

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   
/			directed_round_test:   
/			"""""""""""""""""""   
/	function:   
/		test for round control (up or down).   
/   
/	inputs:   
/		none   
/   
/	outputs:   
/		returns true (in %al) if the round control    
/		is rnd_up or rnd_down.  returns false, otherwise.   
/   
/	data accessed:   
/   
/	data changed:   
/   
/	procedures:   
/		get_rnd_conrol   
/   
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""   

directed_round_test:
		call	get_rnd_control   
		cmpb	rnd_down,%al   
		je	is_true   
		cmpb	rnd_up,%al   
		je	is_true   
		jmp	is_false   
