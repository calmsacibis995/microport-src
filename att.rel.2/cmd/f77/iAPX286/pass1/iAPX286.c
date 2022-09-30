/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* iAPX286 @(#)iAPX286.c	1.4 85/09/06 */
#include "defs"

#ifdef SDB
#	include <a.out.h>
extern int types2[];
#endif

#include "pccdefs"
/*
	VAX-11/780 - SPECIFIC ROUTINES
*/

extern int modelflg;

char adjfmt[20] = "	sub	%%ax,%d(%%bp)" ;
int maxregvar = MAXREGVAR;
int regnum[] =  { 11, 10, 9, 8, 7, 6 } ;
static int regmask[] = { 0, 0x800, 0xc00, 0xe00, 0xf00, 0xf80, 0xfc0 };



ftnint intcon[14] =
	{ 2, 2, 2, 2,
	  15, 31, 24, 56,
	  -128, -128, 127, 127,
	  32767, 2147483647 };

#if HERE == VAX & TARGET != IAPX286
	/* then put in constants in octal */
long realcon[6][2] =
	{
		{ 0200, 0 },
		{ 0200, 0 },
		{ 037777677777, 0 },
		{ 037777677777, 037777777777 },
		{ 032200, 0 },
		{ 022200, 0 }
	};
#else
double realcon[6] =
	{
	2.9387358771e-39,
	2.938735877055718800e-39,
	1.7014117332e+38,
	1.701411834604692250e+38,
	5.960464e-8,
	1.38777878078144567e-17
	};
#endif




prsave()
{
int proflab;
if(profileflag)
	{
	proflab = newlabel();
	fprintf(asmfile, ".%d:\t.value\t0\n", proflab);
	if (modelflg != 0)
		p2pi("\tmov\t$<s>.%d,%%bx", proflab);
	p2pi("\tmov\t$.%d,%%si", proflab);
	if (modelflg == 0)
		p2pass("\tcall\t_mcount");
	else
		p2pass("\tlcall\t_mcount");
	}
}


goret(type)
int type;
{
if (procclass == CLPROC && proctype != TYSUBR)
	{
	if (proctype == TYDREAL)
		if (modelflg == 0)
			p2pass("\tleave\n\tret /#2");
		else
			p2pass("\tleave\n\tlret /#2");
	else if (proctype != TYERROR && proctype != TYUNKNOWN)
		if (modelflg == 0)
			p2pass("\tleave\n\tret /#1");
		else
			p2pass("\tleave\n\tlret /#1");
	else
		badthing("procedure type", proctype, "goret");
	}
else if (modelflg == 0)
	p2pass("\tleave\n\tret /#0");
else
	p2pass("\tleave\n\tlret /#0");
}




/*
 * move argument slot arg1 (relative to ap)
 * to slot arg2 (relative to ARGREG)
 */

mvarg(arg1)
int arg1;
{
p2pi("\tpush\t%d(%%bp)", arg1+ARGOFFSET);
}
mvdummy()
{
p2pi("\tpush\t$%d",0);
}

prlabel(fp, k)
FILEP fp;
int k;
{
fprintf(fp, ".%d:\n", k);
}



prconi(fp, type, n)
FILEP fp;
int type;
ftnint n;
{
fprintf(fp, "\t%s\t%ld\n", (type==TYSHORT ? ".value" : ".long"), n);
}


prnicon(fp, type, num, n)
register FILEP fp;
int type;
register int n;
ftnint num;
{
char buf[20];

sprintf(buf, "%ld", num);
fprintf(fp, "\t%s\t%s", (type==TYSHORT ? ".value" : ".long"), buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}



prcona(fp, a)
FILEP fp;
ftnint a;
{
fprintf(fp, "\t.value\t%ld\n", a);
}



prnacon(fp, a, n)
register FILEP fp;
ftnint a;
register int n;
{
char buf[20];

sprintf(buf, ".%ld", a);
if (modelflg == 0) {
	fprintf(fp, "\t.value\t%s", buf);
} else {
	fprintf(fp, "\t.value\t%s, <s>%s", buf, buf);
}
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}

prconr(fp, type, x)
FILEP fp;
int type;
float x;
{
fprintf(fp, "\t%s\t%.20g\n", (type==TYREAL ? ".float" : ".double"), x);
}

prnrcon(fp, type, x, n)
register FILEP fp;
int type;
register int n;
float x;
{
char buf[30];

sprintf(buf, "%.20g", x);
fprintf(fp, "\t%s\t%s", (type==TYREAL ? ".float" : ".double"), buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}

prncxcon(fp, type, real, imag, n)
register FILEP fp;
int type;
register int n;
double real, imag;
{
char buf[50];

sprintf(buf, "%.20g,%.20g", real, imag);
fprintf(fp, "\t%s\t%s", (type == TYREAL ? ".float" : ".double"), buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}



#if TARGET != IAPX286
prconr(fp, type, x)
FILEP fp;
int type;
double x;
{
union { double xd; long int xl[2]; } cheat;
cheat.xd = x;
if(type == TYREAL)
	fprintf(fp, "\t.long\t0x%lx\n", cheat.xl[0]);
else
	fprintf(fp, "\t.long\t0x%lx,0x%lx\n", cheat.xl[0], cheat.xl[1]);
}

prnrcon(fp, type, x, n)
register FILEP fp;
int type;
double x;
register int n;
{
char buf[50];
union {double xd; long xl[2]; } cheat;

cheat.xd = x;
if (type == TYREAL)
	sprintf(buf, "0x%lx", cheat.xl[0]);
else
	sprintf(buf, "0x%lx,0x%lx", cheat.xl[0], cheat.xl[1]);
fprintf(fp, "\t.long\t%s", buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}

prncxcon(fp, type, real, imag, n)
register FILEP fp;
int type;
register int n;
double real, imag;
{
char buf[60];
union {double xd; long xl[2];} cheat1, cheat2;

cheat1.xd = real;
cheat2.xd = imag;
if (type == TYREAL)
	sprintf(buf, "0x%lx,0x%lx", cheat1.xl[0], cheat2.xl[0]);
else
	sprintf(buf, "0x%lx,0x%lx,0x%lx,0x%lx", cheat1.xl[0], cheat1.xl[1],
	  cheat2.xl[0], cheat2.xl[1]);
fprintf(fp, "\t.long\t%s", buf);
while (--n)
	fprintf(fp, ",%s", buf);
putc('\n', fp);
}
#endif


praddr(fp, stg, varno, offset)
FILE *fp;
int stg, varno;
ftnint offset;
{
char *memname();

if(stg == STGNULL)
	fprintf(fp, "\t.long\t0\n");
else
	{
	if (modelflg==0)
	fprintf(fp, "\t.value\t%s", memname(stg,varno));
	else
	fprintf(fp, "\t.value\t%s, <s>%s", memname(stg,varno),
		memname(stg, varno));
	if(offset)
		fprintf(fp, "+%ld", offset);
	fprintf(fp, "\n");
	}
}




preven(k)
int k;
{
fprintf(asmfile, "\t.even\n");
}



vaxgoto(index, nlab, labs)
expptr index;
register int nlab;
struct Labelblock *labs[];
{
register int i;
register int arrlab, beglab;
putforce(TYINT, index);
p2pi("\tsub\t$1,%%ax\n\tjl\t.%d",arrlab=newlabel());
p2pi("\tcmp\t$%d,%%ax",nlab-1);
p2pi("\tjg\t.%d\n\tsal\t%%ax\n\tmov\t%%ax,%%si",arrlab);
p2pi("\tjmp\t*%%cs:.%d(%%si)",beglab=newlabel());
p2pi(".%d:", beglab);
for(i = 0; i< nlab ; ++i)
	if( labs[i] )
		p2pij("\t.value\t.%d", labs[i]->labelno);
p2pi(".%d:", arrlab);
}


prarif(p, neg, zer, pos)
expptr p;
int neg, zer, pos;
{
int t;

putforce(t = p->headblock.vtype, p);
if (ISINT(t))
	{
	p2pass("	test	%ax");
	p2pi("\tjl\t.%d", neg);
	}
else	{
	p2pass("	ftst\n	fstsw	%ax\n	fwait\nsahf");
	p2pi("\tjb\t.%d", neg);
	}
p2pi("\tje\t.%d", zer);
p2pi("\tjmp\t.%d", pos);
}




char *memname(stg, mem)
int stg, mem;
{
static char s[20];

switch(stg)
	{
	case STGCOMMON:
	case STGEXT:
		sprintf(s, "%s", varstr(XL, extsymtab[mem].extname) );
		break;

	case STGBSS:
	case STGINIT:
		sprintf(s, "v.%d", mem);
		break;

	case STGCONST:
		sprintf(s, ".%d", mem);
		break;

	case STGEQUIV:
		sprintf(s, "q.%d", mem+eqvstart);
		break;

	default:
		badstg("memname", stg);
	}
return(s);
}




prlocvar(s, len, align)
char *s;
ftnint len;
int align;
{
fprintf(asmfile, "\t.bss\t%s,%ld\n",
	s, len);
}



prext(name, leng)
char *name;
ftnint leng;
{
if(leng == 0)
	fprintf(asmfile, "\t.globl\t%s\n", name);
else
	fprintf(asmfile, "\t.comm\t%s,%ld\n", name, leng);
}

prendproc()
{
p2pi("\t.set\t.F%d,1", procno);
}




prtail()
{
}

#define NSLOT 30


prolog(ep, argvec)
struct Entrypoint *ep;
Addrp  argvec;
{
int i, argslot, size, nlabno;
register chainp p;
register Namep q;
register struct Dimblock *dp;
expptr tp, mkaddcon();
int perm[NSLOT] ;

if(procclass == CLBLOCK)
	return;
p2pi(".%d:", ep->backlabel);
for (i=0; i<NSLOT; i++) perm[i]= -1;
p2pi("\tenter\t$.S%d,$0",procno);
if(argvec)
	{
	argloc = argvec->memoffset->constblock.const.ci + SZINT;
fprintf(stderr, "argloc=%x\n", argloc);
			/* first slot holds count */
	if(proctype == TYCHAR)
		{
		perm[chslot/SZADDR]=0;
		perm[chlgslot/SZLENG]=SZADDR;
		argslot = SZADDR + SZLENG;
		}
	else if( ISCOMPLEX(proctype) )
		{
		perm[cxslot/SZADDR]=0;
		argslot = SZADDR;
		}
	else
		argslot = 0;

	for(p = ep->arglist ; p ; p =p->nextp)
		{
		q = (Namep) (p->datap);
		perm[q->vardesc.varno/SZADDR]=argslot;
		argslot += SZADDR;
		}
	for(p = ep->arglist ; p ; p = p->nextp)
		{
		q = (Namep) (p->datap);
		if(q->vtype==TYCHAR && q->vclass!=CLPROC)
			{
			if(q->vleng && ! ISCONST(q->vleng) )
		perm[q->vleng->addrblock.memno/SZLENG]=argslot;
			argslot += SZLENG;
			}
		}
i=NSLOT-1; while(perm[i]==(-1) && i>=0) i--;
while (i>=0) {
	if (perm[i]!= -1) mvarg(perm[i]);
	else mvdummy();
	i--;
	}
/* When generating code for procedures with more than one entry point,
 * the arguments are restacked. Two additional lines of code are needed.
 * line 1: Set ap past argument list + 1 slot over.
 * line 2: Place # of arguments in extra slot pointed to by ap.
 *      low address                      high address
 *      ---------------------------------------------
 *      |        | # args |  arg   |  arg   |       |
 *      ---------------------------------------------
 *                   ap                         fp
 */
	p2pi("\tcall\t.%d\n\tleave\n\tret",ep->entrylabel);
	}

for(p = ep->arglist ; p ; p = p->nextp)
	{
	q = (Namep) (p->datap);
	if(dp = q->vdim)
		{
		for(i = 0 ; i < dp->ndim ; ++i)
			if(dp->dims[i].dimexpr)
				puteq( fixtype(cpexpr(dp->dims[i].dimsize)),
					fixtype(cpexpr(dp->dims[i].dimexpr)));
		size = typesize[ q->vtype ];
		if(q->vtype == TYCHAR)
			if( ISICON(q->vleng) )
				size *= q->vleng->constblock.const.ci;
			else
				size = -1;

		/* on VAX, get more efficient subscripting if subscripts
		   have zero-base, so fudge the argument pointers for arrays.
		   Not done if array bounds are being checked.
		*/
		if(dp->basexpr)
			puteq( 	cpexpr(fixtype(dp->baseoffset)),
				cpexpr(fixtype(dp->basexpr)));

		if(checksubs)
			{
			if(dp->basexpr)
				{
				if(size > 0)
					tp = (expptr) ICON(size);
				else
					tp = (expptr) cpexpr(q->vleng);
				putforce(TYINT,
					fixtype( mkexpr(OPSTAR, tp,
						cpexpr(dp->baseoffset)) ));
				p2pi(adjfmt ,
					p->datap->nameblock.vardesc.varno +
						ARGOFFSET);
				}
			else if(dp->baseoffset->constblock.const.ci != 0)
				{
				char buff[25];
				if(size > 0)
					{
					sprintf(buff, adjfmt,
						dp->baseoffset->constblock.const.ci * size,
						p->datap->nameblock.vardesc.varno +
							ARGOFFSET);
					}
				else	{
					putforce(TYINT, mkexpr(OPSTAR, cpexpr(dp->baseoffset),
						cpexpr(q->vleng) ));
					sprintf(buff, adjfmt,
						p->datap->nameblock.vardesc.varno +
							ARGOFFSET);
					}
				p2pass(buff);
				}
			}
		}
	}

if(typeaddr)
	puteq( cpexpr(typeaddr), mkaddcon(ep->typelabel) );
/* replace to avoid long jump problem
putgoto(ep->entrylabel);
*/
#if TARGET != IAPX286
p2pij("\t.set\t.R%d,0x%x", procno, regmask[highregvar]);
#endif
p2pij("\t.set\t.SSP%d,%d", procno, (autoleng * sizeof(char) + 3) & ~3);
}




prhead(fp)
FILEP fp;
{
#if (!defined(FONEPASS)) && (FAMILY == PCC)
	p2triple(P2LBRACKET, ARGREG-highregvar, procno);
	p2word(0L);
	p2flush();
#endif
}



prdbginfo()
{
}

#ifdef SDB
prstab(name, val, stg, type)
char *name, *val;
int stg, type;
{
	char buf[BUFSIZ];

	if ((type & 0x0f) == T_ENUM)	/* got to be complex */
		sprintf(buf, "\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t010;\t.tag\tComplex;\t.size\t8;\t.endef",
		    name, val, stg);
	else if ((type & 0x0f) == T_UCHAR)	/* got to be double complex */
		sprintf(buf, "\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t010;\t.tag\tDcomplex;\t.size\t16;\t.endef",
		    name, val, stg);
	else
	   sprintf(buf,"\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t0x%x;\t.endef"
		, name, val, stg, type);
	p2pass(buf);
}


prarstab(name, val, stg, type, np)
char *name, *val;
int stg, type;
register Namep np;
{
	register int i, ts;
	char buf[BUFSIZ];

	sprintf(buf, "\t.def\t%s;\t.val\t%s-0x%x;\t.scl\t%d;\t.type\t",
	    name, val, np->vdim->baseoffset->constblock.const.ci *
	    (ts = typesize[(int) np->vtype]), stg);
	if ((i = type & 0x0f) == T_ENUM )
		strcat(buf, "010;\t.tag\tComplex;");
	else if (i == T_UCHAR)
		strcat(buf, "010;\t.tag\tDcomplex;");
	else
		sprintf(buf + strlen(buf), "0x%x;", type);
	sprintf(buf + strlen(buf), "\t.size\t0x%x;\t.dim\t",
	    np->vdim->nelt->constblock.const.ci * ts);
	for (i = 0; i != np->vdim->ndim; i++)
		sprintf(buf + strlen(buf), "%d,",
		    np->vdim->dims[i].dimsize->constblock.const.ci);
	sprintf(buf + strlen(buf) - 1, ";\t.endef");
	p2pass(buf);
}


prststab(name, val, stg, type, tag, size)
char *name, *val, *tag;
int stg, type, size;
{
	char buf[BUFSIZ];

	sprintf(buf, "\t.def\t%s;\t.val\t%s;\t.scl\t%d;\t.type\t%d;\t.tag\t%s;\t.size\t%d;\t.endef",
	    name, val, stg, type, tag, size);
	p2pass(buf);
}



stabtype(p)
register Namep p;
{
int d[7];		/* Type Word |d6|d5|d4|d3|d2|d1|typ|		 */
register int *pd;	/* Pointer to d. d fields = 2 bits, typ = 4 bits */
register int type;
register int i;

d[1] = d[2] = d[3] = d[4] = d[5] = d[6] = 0;
pd = &d[1];

if (p->vtype == TYCOMPLEX)
	d[0] = T_ENUM;
else if (p->vtype == TYDCOMPLEX)
	d[0] = T_UCHAR;
else
	d[0] = types2[p->vtype];

/* For each dimension of an array, fill a d slot with 3 (array).
 * If array is argument, fill 1 less slot. Later on ptr value will be added.
 */
if (p->vdim) {
	i = (p->vstg == STGARG) ? 2 : 1;
	for (; i <= p->vdim->ndim; i++, pd++)
		*pd = 3;	/* 3 = array	*/
	}

if(p->vstg == STGARG)
	*pd++ = 1;		/* 1 = pointer	*/
if (p->vclass == CLPROC)
	*pd++ = 2;		/* 2 = function	*/

for (type = 0, pd--; pd > &d[0]; pd--)
	type = (type << 2) | *pd;
type = (type << 4) | d[0];
return(type);
}
#endif
