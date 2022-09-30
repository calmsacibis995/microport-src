/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
OP("fadds",FPOKM,0),
OP("fiaddl",FPOKM,0),
OP("faddl",FPOKM,0),
OP("fiadd",FPOKM,0),
OP("fadd",FPOK,0),
OP("reserved",RESERVED,0),		/*  add illegal combination */
OP("fadd",FPOK,0),
OP("faddp",FPOK,0),
OP("fmuls",FPOKM,0),
OP("fimull",FPOKM,0),
OP("fmull",FPOKM,0),
OP("fimul",FPOKM,0),
OP("fmul",FPOK,0),
OP("reserved",RESERVED,0),		/*  mul illegal combination */
OP("fmul",FPOK,0),
OP("fmulp",FPOK,0),
OP("fcoms",FPCOK,0),
OP("ficoml",FPCOK,0),
OP("fcoml",FPCOK,0),
OP("ficom",FPCOK,0),
OP("fcom",FPCOK,0),
OP("reserved",RESERVED,0),		/*  com illegal combination */
OP("(fcom)",FPCW,0),
OP("(fcomp)",FPCW,0),
OP("fcomps",FPCOK,0),
OP("ficompl",FPCOK,0),
OP("fcompl",FPCOK,0),
OP("ficomp",FPCOK,0),
OP("fcomp",FPCOK,0),
OP("reserved",RESERVED,0),		/*  comp illegal combination */
OP("(fcomp)",FPCW,0),
OP("fcompp",FPCNOARG,0),
OP("fsubs",FPOKM,0),
OP("fisubl",FPOKM,0),
OP("fsubl",FPOKM,0),
OP("fisub",FPOKM,0),
OP("fsub",FPOK,0),
OP("reserved",RESERVED,0),		/*  sub illegal combination */
OP("fsub",FPOK,0),
OP("fsubp",FPOK,0),
OP("fsubrs",FPOKM,0),
OP("fisubrl",FPOKM,0),
OP("fsubrl",FPOKM,0),
OP("fisubr",FPOKM,0),
OP("fsubr",FPOK,0),
OP("reserved",RESERVED,0),		/*  subr illegal combination */
OP("fsubr",FPOK,0),
OP("fsubrp",FPOK,0),
OP("fdivs",FPOKM,0),
OP("fidivl",FPOKM,0),
OP("fdivl",FPOKM,0),
OP("fidiv",FPOKM,0),
OP("fdiv",FPOK,0),
OP("reserved",RESERVED,0),		/*  div illegal combination */
OP("fdiv",FPOK,0),
OP("fdivp",FPOK,0),
OP("fdivrs",FPOKM,0),
OP("fidivrl",FPOKM,0),
OP("fdivrl",FPOKM,0),
OP("fidivr",FPOKM,0),
OP("fdivr",FPOK,0),
OP("reserved",RESERVED,0),		/*  divr illegal combination */
OP("fdivr",FPOK,0),
OP("fdivrp",FPOK,0),
