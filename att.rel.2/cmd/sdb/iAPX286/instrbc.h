/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
OP("inc",NORMAL,RMTOR+SWORD),
OP("dec",NORMAL,RMTOR+SWORD),
OP("call",NORMAL,RMTOR+SWORD),
OP("lcall",NORMAL,RMTOR+SWORD),
OP("jmp",NORMAL,RMTOR+SWORD),
OP("jmp",NORMAL,MEMO+RMTOR+SWORD),
OP("push",NORMAL,MEMO+RMTOR+SWORD),
OP("",ILLEGAL,0),
OP("incb",NORMAL,RMTOR+SBYTE),
OP("decb",NORMAL,RMTOR+SBYTE),
OP("",ILLEGAL,0),
OP("",ILLEGAL,0),
OP("",ILLEGAL,0),
OP("",ILLEGAL,0),
OP("",ILLEGAL,0),
OP("",ILLEGAL,0),
