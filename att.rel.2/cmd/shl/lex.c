# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define NOCMDS 10

static char *cmds[] = 
{
	"?",
	"create",
	"resume",
	"delete",
	"layers",
	"help",
	"quit",
	"block",
	"unblock",
	"toggle"
};

static int cmd_values[] =
{
	LX_help,
	LX_create,
	LX_resume,
	LX_delete,
	LX_layers,
	LX_help,
	LX_quit,
	LX_block,
	LX_unblock,
	LX_toggle,
};

yywrap()
{
	return(0);
}

reserved(str,length)
	char *str;
	int length;
{
	int i;

	if (length > 7)
		return(ERROR);
	else
	{
		for (i=0; i < NOCMDS; i++)
			if (strncmp(str,cmds[i], length) == 0)
				return(cmd_values[i]);

		return(ERROR);
	}
}

# define CMD 2
# define OTHER 4
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
int token;
char *strcpy();
char *strncpy();
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
		{
						BEGIN OTHER;
						if ((token = reserved(yytext,yyleng)) != ERROR)
							return(token);
						else
						{
							strncpy(yylval.str, yytext, NAMSIZ);
							yylval.str[NAMSIZ] = 0;
							return(LX_name);
						}
					}
break;
case 2:
{
						strcpy(yylval.str,yytext);
						return(LX_defname);
					}
break;
case 3:
	{
						return(LX_long_option);
					}
break;
case 4:
{
						strncpy(yylval.str, yytext, NAMSIZ);
						yylval.str[NAMSIZ] = 0;
						return(LX_name);
					}
break;
case 5:
	{
							BEGIN CMD;
							return(LX_newline);
					}
break;
case 6:
;
break;
case 7:
	{
						return(ERROR);
					}
break;
case 8:
			{
						BEGIN CMD;
						unput(yytext[0]);
					}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] = {
0,

8,
0,

4,
7,
8,
0,

6,
7,
8,
0,

5,
8,
0,

4,
7,
8,
0,

4,
7,
8,
0,

2,
4,
7,
8,
0,

1,
4,
7,
8,
0,

4,
0,

6,
0,

4,
0,

3,
4,
0,

1,
4,
0,

2,
4,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,7,	0,0,	
3,8,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,7,	1,7,	
3,9,	3,10,	8,15,	9,16,	
11,0,	11,0,	0,0,	0,0,	
0,0,	0,0,	8,0,	8,0,	
0,0,	0,0,	0,0,	0,0,	
12,0,	12,0,	0,0,	0,0,	
13,0,	13,0,	14,0,	14,0,	
15,0,	15,0,	9,16,	11,0,	
0,0,	1,7,	1,7,	3,11,	
3,8,	8,0,	4,12,	5,12,	
3,12,	6,12,	1,7,	12,0,	
3,13,	8,15,	8,15,	13,0,	
11,17,	14,0,	0,0,	15,0,	
17,0,	17,0,	8,15,	0,0,	
1,7,	5,8,	3,14,	6,8,	
18,0,	18,0,	19,0,	19,0,	
20,0,	20,0,	0,0,	0,0,	
8,15,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	17,0,	
0,0,	0,0,	0,0,	0,0,	
14,19,	0,0,	0,0,	18,0,	
17,20,	19,0,	0,0,	20,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
19,19,	0,0,	0,0,	12,18,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+0,	yysvec+1,	0,	
yycrank+-3,	0,		0,	
yycrank+-1,	yysvec+3,	0,	
yycrank+-2,	yysvec+3,	0,	
yycrank+-4,	yysvec+3,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+-13,	0,		yyvstop+3,
yycrank+6,	0,		yyvstop+7,
yycrank+0,	0,		yyvstop+11,
yycrank+-7,	yysvec+8,	yyvstop+14,
yycrank+-19,	yysvec+8,	yyvstop+18,
yycrank+-23,	yysvec+8,	yyvstop+22,
yycrank+-25,	yysvec+8,	yyvstop+27,
yycrank+-27,	yysvec+8,	yyvstop+32,
yycrank+0,	yysvec+9,	yyvstop+34,
yycrank+-51,	yysvec+8,	yyvstop+36,
yycrank+-59,	yysvec+8,	yyvstop+38,
yycrank+-61,	yysvec+8,	yyvstop+41,
yycrank+-63,	yysvec+8,	yyvstop+44,
0,	0,	0};
struct yywork *yytop = yycrank+127;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
'(' ,')' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'1' ,'1' ,'1' ,'1' ,'1' ,'1' ,'1' ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,'?' ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,
'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,
'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,'?' ,
'?' ,'?' ,'?' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	@(#)	1.1	*/
int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
