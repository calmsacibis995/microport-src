
# line 10 "yacc.y"
#include	"defs.h"

#define		ERROR	'e'

int flag;

int (*func)();
int destroy();
int block();
int unblock();

char *prs();

# line 24 "yacc.y"
typedef union  {			/* Yacc value stack */
	  int ival;
	  char str[256];
       } YYSTYPE;
# define LX_name 257
# define LX_defname 258
# define LX_create 259
# define LX_resume 260
# define LX_delete 261
# define LX_layers 262
# define LX_quit 263
# define LX_newline 264
# define LX_long_option 265
# define LX_help 266
# define LX_block 267
# define LX_unblock 268
# define LX_toggle 269
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 161 "yacc.y"


yyerror(s)			/* Yacc required error routine */
	char *s;
{
}


#include	"lex.c"
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 34
# define YYLAST 90
yytabelem yyact[]={

     4,    17,    18,     6,     7,     8,     9,    10,    16,    16,
    11,    13,    14,    12,    17,    18,    16,    27,    22,    17,
    18,    16,     5,    38,    42,    16,     2,    33,    19,    21,
    23,    32,    26,    29,    30,    31,    41,    28,    34,    25,
    39,    15,    15,    35,    20,    36,     3,    37,    24,     1,
    40,     0,     0,     0,     0,     0,    44,    45,     0,     0,
     0,    46,     0,     0,     0,    49,    48,    51,    52,    43,
     0,    53,     0,     0,     0,     0,     0,     0,     0,    47,
     0,     0,    43,    50,     0,    47,    47,     0,     0,    50 };
yytabelem yypact[]={

  -256,  -256, -1000, -1000, -1000, -1000,  -239,  -243, -1000,  -248,
  -255,  -255,  -255, -1000, -1000,  -255, -1000, -1000, -1000, -1000,
  -255, -1000,  -255, -1000,  -255,  -238, -1000,  -255,  -238, -1000,
 -1000, -1000,  -238,  -238, -1000, -1000, -1000, -1000,  -243, -1000,
 -1000,  -238,  -243, -1000,  -243,  -243, -1000, -1000,  -243, -1000,
 -1000, -1000, -1000, -1000 };
yytabelem yypgo[]={

     0,    40,    49,    26,    46,    44,    22,    39,    23,    37,
    24,    36,    31,    27 };
yytabelem yyr1[]={

     0,     2,     2,     3,     5,     3,     3,     4,     4,     4,
     4,     7,     4,     4,     4,     9,     4,    11,     4,     4,
     4,     4,    12,     4,    13,     4,     4,    10,    10,     8,
     8,     1,     1,     6 };
yytabelem yyr2[]={

     0,     2,     4,     3,     1,     7,     3,     5,     7,     5,
     7,     1,     8,     5,     7,     1,     8,     1,    10,     5,
     5,     5,     1,     8,     1,     8,     5,     3,     5,     3,
     5,     2,     2,     2 };
yytabelem yychk[]={

 -1000,    -2,    -3,    -4,   256,    -6,   259,   260,   261,   262,
   263,   266,   269,   267,   268,    -1,   264,   257,   258,    -3,
    -5,    -6,   257,    -6,    -1,    -7,    -6,   265,    -9,    -6,
    -6,    -6,   -12,   -13,    -6,    -6,    -6,    -6,    -8,    -1,
    -6,   -11,   -10,    -1,    -8,    -8,    -6,    -1,   -10,    -6,
    -1,    -6,    -6,    -6 };
yytabelem yydef[]={

     0,    -2,     1,     3,     4,     6,     0,     0,    11,    15,
     0,     0,     0,    22,    24,     0,    33,    31,    32,     2,
     0,     7,     0,     9,     0,     0,    13,    17,     0,    19,
    20,    21,     0,     0,    26,     5,     8,    10,     0,    29,
    14,     0,     0,    27,     0,     0,    12,    30,     0,    16,
    28,    23,    25,    18 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"LX_name",	257,
	"LX_defname",	258,
	"LX_create",	259,
	"LX_resume",	260,
	"LX_delete",	261,
	"LX_layers",	262,
	"LX_quit",	263,
	"LX_newline",	264,
	"LX_long_option",	265,
	"LX_help",	266,
	"LX_block",	267,
	"LX_unblock",	268,
	"LX_toggle",	269,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"input : input_line",
	"input : input input_line",
	"input_line : cmd",
	"input_line : error",
	"input_line : error eol",
	"input_line : eol",
	"cmd : LX_create eol",
	"cmd : LX_create LX_name eol",
	"cmd : LX_resume eol",
	"cmd : LX_resume name eol",
	"cmd : LX_delete",
	"cmd : LX_delete name_list eol",
	"cmd : LX_layers eol",
	"cmd : LX_layers LX_long_option eol",
	"cmd : LX_layers",
	"cmd : LX_layers layers_list eol",
	"cmd : LX_layers LX_long_option",
	"cmd : LX_layers LX_long_option layers_list eol",
	"cmd : LX_quit eol",
	"cmd : LX_help eol",
	"cmd : LX_toggle eol",
	"cmd : LX_block",
	"cmd : LX_block name_list eol",
	"cmd : LX_unblock",
	"cmd : LX_unblock name_list eol",
	"cmd : name eol",
	"layers_list : name",
	"layers_list : layers_list name",
	"name_list : name",
	"name_list : name_list name",
	"name : LX_name",
	"name : LX_defname",
	"eol : LX_newline",
};
#endif /* YYDEBUG */
/*	@(#)	1.1	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 3:
# line 47 "yacc.y"
{
													prompt();
												} break;
case 4:
# line 51 "yacc.y"
{
													fprintf(stderr, "syntax error\n");
												} break;
case 5:
# line 55 "yacc.y"
{
													yyerrok;
													prompt();
												} break;
case 6:
# line 60 "yacc.y"
{
													prompt();
												} break;
case 7:
# line 65 "yacc.y"
{
													create("");
												} break;
case 8:
# line 69 "yacc.y"
{
													create(yypvt[-1].str);
												} break;
case 9:
# line 73 "yacc.y"
{
													resume_current();
												} break;
case 10:
# line 77 "yacc.y"
{
													resume(yypvt[-1].str);
												} break;
case 11:
# line 81 "yacc.y"
{
													func = destroy;
												} break;
case 13:
# line 86 "yacc.y"
{
													all_layers(0);
												} break;
case 14:
# line 91 "yacc.y"
{
													all_layers(1);
												} break;
case 15:
# line 95 "yacc.y"
{	
													flag = 0;
												} break;
case 17:
# line 101 "yacc.y"
{
													flag = 1;
												} break;
case 19:
# line 106 "yacc.y"
{
													kill_all();
													YYACCEPT;
												} break;
case 20:
# line 111 "yacc.y"
{
													help();
												} break;
case 21:
# line 115 "yacc.y"
{
													toggle();
												} break;
case 22:
# line 119 "yacc.y"
{
													func = block;
												} break;
case 24:
# line 124 "yacc.y"
{
													func = unblock;
												} break;
case 26:
# line 129 "yacc.y"
{	
													resume(yypvt[-1].str);
												} break;
case 27:
# line 134 "yacc.y"
{
													one_layer(yypvt[-0].str, flag);
												} break;
case 28:
# line 138 "yacc.y"
{
													one_layer(yypvt[-0].str, flag);
												} break;
case 29:
# line 143 "yacc.y"
{
													(*func)(yypvt[-0].str);
												} break;
case 30:
# line 147 "yacc.y"
{
													(*func)(yypvt[-0].str);
												} break;
	}
	goto yystack;		/* reset registers in driver code */
}
