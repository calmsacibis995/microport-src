/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)float.c	1.3 - 85/08/08 */
#include <ctype.h>
#include "systems.h"
extern double
	lclatof(),
	floor(),
	pow(),
	log2();

atob16f(line,lp)
char *line;
long *lp;
{
	double	dbl,fexp;
	long	exponent,sbit;
	long	mantissa;

/*
 *
 *	Some of the cast operations, namely
 *	double -> float and double -> long  don't necessarily
 *	have the same implementation on various machines
 *	that is why the following two variables are defined.
 *	Any time there are used in this file it is to get
 *	around the cast operator.
 *
 */
	double	dtmp;		/*  dummy var needed for casting */
	float	ftmp;		/*  dummy var needed for casting */

#ifdef	u370
	long	i, nf;	/* dummy var needed for maxi kludge
			it can be deleted when maxi libm is fixed */
#endif

	dbl = lclatof(line);
	if (dbl == 0.0) {
		*lp = 0x0L;
		return(0);
	}
	if (dbl > 0.0)
		sbit = 0L;
	else {
		sbit = 1L;
		dbl *= -1.0;
	}
	fexp = floor(log2(dbl));

/*
 *	The following ifdef is a maxi kludge.
 *	The error tolerence of the pow function
 *	is too small so it doesn't always
 *	return the correct value.
 *
 *	Note: 'dtmp' is NOT being used as a casting var in this ifdef.
 */
#ifdef	u370
	i = (long)fexp;
	if ( i >= 0 ) nf = 0;
	else {
		nf = 1;
		i = -i;
	}
	dtmp = (double)1.0;
	while (i-- > 0L) dtmp *= (double)2.0;
	if (nf == 0)
		dbl *= ((double)1.0 / dtmp);
	else
		dbl *= dtmp;
#else
	dbl *= pow((double)2.0,-fexp);
#endif

	ftmp = dbl;		/* this assgmt takes the place of a cast */
	if (ftmp >= 2.0) {
		dbl /= 2;
		++fexp;
	}
	else if (ftmp < 1.0) {
		dbl *= 2;
		--fexp;
	}
	exponent = (long)fexp + 127;
	dtmp = ((dbl - (double)1.0) * (double)0x00800000) ;  /* multiply by 2**23 */
	ftmp = dtmp;		/* this assgmt takes the place of a cast */
	mantissa = ftmp ;	/* this assgmt takes the place of a cast */
	*lp = (sbit << 31) | (exponent << 23) | mantissa;
	return(0);
}
double
lclatof(p)
register char *p;
{
	register char c;
	double fl, flexp, exp5;
	double big = 72057594037927936.;  /*2^56*/
	double ldexp();
	register int eexp, exp, bexp;
	register short	neg, negexp ;

	neg = 1;
	while((c = *p++) == ' ')
		;
	if (c == '-')
		neg = -1;
	else if (c=='+')
		;
	else
		--p;

	exp = 0;
	fl = 0;
	while ((c = *p++), isdigit(c)) {
		if (fl<big)
			fl = 10*fl + (c-'0');
		else
			exp++;
	}

	if (c == '.') {
		while ((c = *p++), isdigit(c)) {
			if (fl<big) {
				fl = 10*fl + (c-'0');
				exp--;
			}
		}
	}

	negexp = 1;
	eexp = 0;
	if ((c == 'E') || (c == 'e')) {
		if ((c= *p++) == '+')
			;
		else if (c=='-')
			negexp = -1;
		else
			--p;

		while ((c = *p++), isdigit(c)) {
			eexp = 10*eexp+(c-'0');
		}
		if (negexp<0)
			eexp = -eexp;
		exp = exp + eexp;
	}

	negexp = 1;
	if (exp<0) {
		negexp = -1;
		exp = -exp;
	}

	flexp = 1;
	exp5 = 5;
	bexp = exp;
	for (;;) {
		if (exp&01)
			flexp *= exp5;
		exp >>= 1;
		if (exp==0)
			break;
		exp5 *= exp5;
	}
	if (negexp<0)
		fl /= flexp;
	else
		fl *= flexp;
	fl = ldexp(fl, negexp*bexp);
	if (neg<0)
		fl = -fl;
	return(fl);
}

extern double log();

double
log2(x)
double x;
{
	return( log(x) / log((double)2.0) );
}
#if iAPX286
#if NATIVE
#include <stdio.h>
#include "symbols.h"
#include "instab.h"
#include "parse.h"

/*
**	80287 conversion routines 
**
*/


/*
 *	conv287
 *
 *	main conversion routine
 *
 *	type	-	is type of constant expected
 *	asc	-	ascii representation of that value
 *	value	-	pointer to union containg various types for 
 *			return of converted answer
 *
 *	returns 
 *		0	success
 *
 */

conv287 ( type , asc , value )
char *asc ;
floatval *value ;
{

	switch( type )  {

	case PSLONG:
			return dolongs ( 0 , asc , value ) ;
	case PSLLONG:
			return dolongs ( 1 , asc , value ) ;
	case PSFLOAT:
			return doreals ( 0 , asc , value ) ;
	case PSDOUBLE:
			return doreals ( 1 , asc , value ) ;
	case PSTEMP:
			return doreals ( 2 , asc , value ) ;
	case PSBCD:
			return dobcd ( asc , value ) ;
	}

}

dobcd( asc , value )
char *asc ;
floatval *value ;
{
	char buff[20] ;
	int i , j ;
	char *p ;

	p = asc ;
	while ( *p ) 
		p++ ;

	p-- ;

	for(i=0;i<20;i++)
		if (p < asc )
			buff[i] = '0' ;
		else
		{
			if( (*p < '0') || (*p > '9') )
				return 1 ;
			buff[i] = *p-- ;
		}

	for ( i = 0 ;i < 5 ; i++ )
	{
		value->fvala[i] = 0 ;
		for(j=0;j<4;j++)
		{
			value->fvala[i] <<= 4 ;
			value->fvala[i] |= (buff[i*4+j] & 0xf) ;
		}
	}

	return 0 ;
}

/*
 *	doreals - does floating point conversions
 */

doreals(type , asc , value )
char *asc ;
floatval *value ;
{
	double	temp_double ;

	/*
	 * special case for 0.0
	 */

	if( (asc[0] == '0') && (asc[1] == '\0') )
	{
		value->fvala[0] = 0 ;
		value->fvala[1] = 0 ;
		value->fvala[2] = 0 ;
		value->fvala[3] = 0 ;
		value->fvala[4] = 0 ;
		return 0 ;
	}

	sscanf ( asc , "%le" , &temp_double ) ;

	switch( type ) {

	/* float */

	case 0: value->fvalf = temp_double ;
		break ;

	/* double */

	case 1:	value->fvald = temp_double ;
		break ;

	/* temp */

	case 2:	/* NOT YET IMPLEMENTED */
		break ;

	}

	return 0 ;
	
}

dolongs(type , asc , value )
char *asc ;
floatval *value ;
{
	unsigned long val ;
	unsigned short base ;
	char *p = asc ;

	
	val = (*p) - '0';
	if ((*p) == '0') {
		p++ ;
		if ((*p) == 'x' || (*p) == 'X') {
			base = 16;
		} else if (((*p) & ~' ') == 'B') { 
			base = 2;
		} else {
			p++ ;
			base = 8;
		}
	} else
		base = 10;
	while ( (( *(++p) >= '0') && (*p <= '9') )
	    || ((base == 16) &&
		((('a'<=(*p)) && ((*p)<='f'))||(('A'<=(*p)) && ((*p)<='F')))))
	{
		if (base == 8)
			val <<= 3;
		else if (base == 10)
			val *= 10;
		else if (base == 2)
			val <<= 1;
		else
			val <<= 4;
		if ('a' <= (*p) && (*p) <= 'f')
			val += 10 + (*p) - 'a';
		else if ('A' <= (*p) && (*p) <= 'F')
			val += 10 + (*p) - 'A';
		else	val += (*p) - '0';
	}

	if(type)
	{
		/* NOT YET IMPLEMENTED */

		return 0 ;
	}
	else
	{
		value->fvall = val ;
		return 0 ;
	}
}
#else /* not NATIVE */

#include <stdio.h>
#include "symbols.h"
#include "instab.h"
#include "parse.h"

/*
**	80287 conversion routines 
**
*/


/*
 *	conv287
 *
 *	main conversion routine
 *
 *	type	-	is type of constant expected
 *	asc	-	ascii representation of that value
 *	value	-	pointer to 5 shorts to return converted answer
 *
 *	returns 
 *		0	success
 *
 */

conv287 ( type , asc , value )
char *asc ;
unsigned short *value ;
{

	switch( type )  {

	case PSLONG:
			return dolongs ( 0 , asc , value ) ;
	case PSLLONG:
			return dolongs ( 1 , asc , value ) ;
	case PSFLOAT:
			return doreals ( 0 , asc , value ) ;
	case PSDOUBLE:
			return doreals ( 1 , asc , value ) ;
	case PSTEMP:
			return doreals ( 2 , asc , value ) ;
	case PSBCD:
			return dobcd ( asc , value ) ;
	}

}

dobcd( asc , value )
char *asc ;
unsigned short *value ;
{
	char buff[20] ;
	int i , j ;
	char *p ;

	p = asc ;
	while ( *p ) 
		p++ ;

	p-- ;

	for(i=0;i<20;i++)
		if (p < asc )
			buff[i] = '0' ;
		else
		{
			if( (*p < '0') || (*p > '9') )
				return 1 ;
			buff[i] = *p-- ;
		}

	for ( i = 0 ;i < 5 ; i++ )
	{
		value[i] = 0 ;
		for(j=0;j<4;j++)
		{
			value[i] <<= 4 ;
			value[i] |= (buff[i*4+j] & 0xf) ;
		}
	}

	return 0 ;
}

/*
 *	CROSS ASSEMBLY VERSION ONLY
 *	don't worry about accuracy
 */

doreals(type , asc , value )
char *asc ;
unsigned short *value ;
{
	struct cheat { unsigned short a,b,c,d,e;} ;
	union {
		struct cheat cheats ;
		double cheatd ;
		} ch ;
	unsigned short exp ;

	/*
	 * special case for 0.0
	 */

	if( (asc[0] == '0') && (asc[1] == '\0') )
	{
		value[0] = 0 ;
		value[1] = 0 ;
		value[2] = 0 ;
		value[3] = 0 ;
		value[4] = 0 ;
		return 0 ;
	}

	sscanf ( asc , "%le" , &ch.cheatd ) ;

	switch( type ) {

	/* float */

	case 0: exp = ch.cheats.a >> 7 ;
		exp -= 0x81 ;
		exp += 0x7f ;
		value[1] = (ch.cheats.a & 0x007f) | ( exp << 7 ) ;
		value[0] = (ch.cheats.b);
		break ;

	/* double */

	case 1:	exp = ch.cheats.a >> 7 ;
		exp -= 0x81 ;
		exp += 0x3ff ;
		value[3] = (exp << 4 ) + ((ch.cheats.a >> 3) & 0xf) ;
		value[2] = (ch.cheats.a << 13) | (ch.cheats.b >> 3) ;
		value[1] = (ch.cheats.b << 13) | (ch.cheats.c >> 3) ;
		value[0] = (ch.cheats.c << 13) | (ch.cheats.d >> 3) ;
		break ;

	/* temp */

	case 2:	exp = ch.cheats.a >> 7 ;
		exp -= 0x81 ;
		exp += 0x3fff ;
		ch.cheats.e = 0 ;
		value[4] = exp ;
		value[3] = 0x8000 | ( ( ch.cheats.a << 8) & 0x7f00) | 
				( ch.cheats.b >> 8 ) ;
		value[2] = (ch.cheats.b << 8) | (ch.cheats.c >> 8) ;
		value[1] = (ch.cheats.c << 8) | (ch.cheats.d >> 8) ;
		value[0] = (ch.cheats.d << 8) | (ch.cheats.e >> 8) ;
		break ;

	}
/*
#define SWAP(x) ( ((x)>>8) | ((x)<<8) )

	value[0] = SWAP( value[0] );
	value[1] = SWAP( value[1] );
	value[2] = SWAP( value[2] );
	value[3] = SWAP( value[3] );
	value[4] = SWAP( value[4] );
*/
	return 0 ;
	
}

dolongs(type , asc , value )
char *asc ;
unsigned short *value ;
{
	struct cheat { unsigned short a,b ; } ;
	union  {
			struct cheat cheats ;
			unsigned long cheatl ;
		} ch ;
	unsigned long val ;
	unsigned short base ;
	char *p = asc ;

	
	val = (*p) - '0';
	if ((*p) == '0') {
		p++ ;
		if ((*p) == 'x' || (*p) == 'X') {
			base = 16;
		} else if (((*p) & ~' ') == 'B') { 
			base = 2;
		} else {
			p++ ;
			base = 8;
		}
	} else
		base = 10;
	while ( (( *(++p) >= '0') && (*p <= '9') )
	    || ((base == 16) &&
		((('a'<=(*p)) && ((*p)<='f'))||(('A'<=(*p)) && ((*p)<='F')))))
	{
		if (base == 8)
			val <<= 3;
		else if (base == 10)
			val *= 10;
		else if (base == 2)
			val <<= 1;
		else
			val <<= 4;
		if ('a' <= (*p) && (*p) <= 'f')
			val += 10 + (*p) - 'a';
		else if ('A' <= (*p) && (*p) <= 'F')
			val += 10 + (*p) - 'A';
		else	val += (*p) - '0';
	}

	if(type)
	{
		ch.cheatl = val ;
		value[0] = 0 ;
		value[1] = 0 ;
		value[2] = ch.cheats.b ;
		value[3] = ch.cheats.a ;
		return 0 ;
	}
	else
	{
		ch.cheatl = val ;
		value[0] = (ch.cheats.a) ;
		value[1] = (ch.cheats.b) ;
/*
		value[0] = SWAP(ch.cheats.a) ;
		value[1] = SWAP(ch.cheats.b) ;
*/
		return 0 ;
	}
}


#endif
#endif
