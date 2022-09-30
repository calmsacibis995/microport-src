/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/
#include "dis.h"
#define OP(a,b,c){a,b,c}

struct optab optab[256] = {
#include "instrs.h"
};
struct optab optabba[] = {
#include "instrba.h"
};
struct optab optabb1a[] = {
#include "instrb1a.h"
};
struct optab optabbb[] = {
#include "instrbb.h"
};
struct optab optabbc[] = {
#include "instrbc.h"
};
struct optab optabbd[] = {
#include "instrbd.h"
};
struct optab optabbr[] = {
#include "instrbr.h"
};
struct optab optabfa[] = {
#include "instrfa.h"
};
struct optab optabfb[] = {
#include "instrfb.h"
};
struct optab optabfc[] = {
#include "instrfc.h"
};
struct optab optabws[] = {
#include "instrfd.h"
};
char mneu[80];
long loc;
long dispaddr;
char decodetype;
int override;
int argument[5];
char *curpos;
char *segovtbl[] = {
	"%es:",
	"%cs:",
	"%ss:",
	"%ds:",
	0
	};
char *reglistd[] = {
	"%ax",
	"%cx",
	"%dx",
	"%bx",
	"%sp",
	"%bp",
	"%si",
	"%di",
	"%al",
	"%cl",
	"%dl",
	"%bl",
	"%ah",
	"%ch",
	"%dh",
	"%bh",
	"(%bx,%si)",
	"(%bx,%di)",
	"(%bp,%si)",
	"(%bp,%di)",
	"(%si)",
	"(%di)",
	"(%bp)",
	"(%bx)",
	"",
	"",
	"",
	"",
	"%es",
	"%cs",
	"%ss",
	"%ds",
	"",
	"$1",
	"(%dx)",
	"(%dx)",
	0
	};
