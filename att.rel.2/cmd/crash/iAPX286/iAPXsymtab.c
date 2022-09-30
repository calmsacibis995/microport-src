/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)iAPXsymtab.c	1.6 - 85/08/13 */
#include "crash.h"
#include "ldfcn.h"

struct syment *stbl;
extern char *namelist;
int symcnt;
char *strngtab;
#if iAPX286
long sysvad();
long lseek();
long strtbsz;
#else
int strtbsz;
#endif

rdsymtab()
{
	FILE *fp;
	struct filehdr filehdr;
	struct syment *sp;
#if iAPX286
	extern char * malloc();
	LDFILE * ioptr;
	long i;
#else
	int i;
#endif

	if((fp = fopen(namelist, "r")) == NULL)
		fatal("cannot open namelist file");
#if iAPX286
	ioptr = NULL;
	if((ioptr = ldopen(namelist,ioptr)) == NULL)
		fatal("cannot open namelist file");
	fp = IOPTR(ioptr);
	filehdr = HEADER(ioptr);
	if(filehdr.f_magic != I286LMAGIC)
		fatal("namelist not in iAPX286 COFF format");
	i = filehdr.f_nsyms*SYMESZ;
	if(i > 64000L)
		i = 64000L;
	if((stbl=(struct syment *)malloc(i)) == (struct syment *) -1)
		fatal("cannot allocate space for namelist");
	i = filehdr.f_symptr + filehdr.f_nsyms*SYMESZ;
	fseek(fp, i, 0);
	fread(&strtbsz, sizeof (long), 1, fp);
	if (strtbsz) {
		if (strtbsz > 64000L)
			fatal("cannot allocate space for string table");
		if ((strngtab = (char *)malloc(strtbsz)) == (char *)-1)
			fatal("cannot allocate space for string table");
		if (fread(strngtab+4, (int)(strtbsz-4), 1, fp) != 1)
			fatal("cannot read string table");
	}
	fseek(fp, filehdr.f_symptr,0);
	symcnt = 0;
	for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
		symcnt++;
		if(fread(sp,SYMESZ,1,fp) != 1)
			fatal("read error in namelist file");
		if (sp->n_zeroes == 0) {
			if (sp->n_offset < 4 || sp->n_offset >= strtbsz)
				fatal("bad flex symbol offset");
			strncpy(sp->n_name, strngtab + sp->n_offset, 8);
		}
		if(sp->n_numaux) {
			fseek(fp,(long)(AUXESZ*sp->n_numaux),1);
			i += sp->n_numaux;
		}
	}
	ldclose(ioptr);
#else
	if((fp = fopen(namelist, "r")) == NULL)
		fatal("cannot open namelist file");
	if(fread(&filehdr, FILHSZ, 1, fp) != 1)
		fatal("read error in namelist file");
	if(filehdr.f_magic != VAXROMAGIC)
		fatal("namelist not in a.out format");
	if((stbl=(struct syment *)sbrk(filehdr.f_nsyms*20)) == (struct syment *)-1)
		fatal("cannot allocate space for namelist");
	i = (int)filehdr.f_symptr + filehdr.f_nsyms*18;
	fseek(fp, i, 0);
	fread(&strtbsz, 4, 1, fp);
	if (strtbsz) {
		if ((strngtab = (char *)sbrk(strtbsz)) == (char *)-1)
			fatal("cannot allocate space for string table");
		fread(strngtab+4, strtbsz-4, 1, fp);
	}
	fseek(fp, filehdr.f_symptr, 0);
	symcnt = 0;
	for(i=0, sp=stbl; i < filehdr.f_nsyms; i++, sp++) {
		symcnt++;
		if(fread(sp, SYMESZ, 1, fp) != 1)
			fatal("read error in namelist file");
		if (sp->n_zeroes == 0) {
			if (sp->n_offset < 4 || sp->n_offset >= strtbsz)
				fatal("bad flex symbol offset");
			strncpy(sp->n_name, strngtab + sp->n_offset, 8);
		}
		if(sp->n_numaux) {
			fseek(fp, AUXESZ*sp->n_numaux, 1);
			i += sp->n_numaux;
		}
	}
	brk(sp);
	fclose(fp);
#endif
}

struct syment *
search(addr, sect1, sect2)
#if iAPX286
ADDR 	addr;
#else
unsigned addr;
#endif
register sect1, sect2;
{
	register struct syment *sp;
	register struct syment *save;

#if iAPX286
	long	 value;
	value = 0L;
#else
	unsigned value;
	value = 0;
#endif
	save = 0;
#ifdef DEBUG
#if iAPX286
	fprintf(stderr,"addr:\t%lx\tsect1:\t%d\tsect2:\t%d\n",addr,sect1,sect2);
#else
	fprintf(stderr,"addr:\t%x\tsect1:\t%d\tsect2:\t%d\n",addr,sect1,sect2);
#endif
#endif
	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
#if !iAPX286
/*    for 286 dont need to check for TEXT DATA BSS implied in address */
		if (sp->n_scnum == sect1 || sp->n_scnum == sect2) { 
#endif
#ifdef DEBUG
#if iAPX286
	fprintf(stderr,"nm=%s scnum=%o sp_val=%x <=> sect1=%o sect2=%o addr=%lx\n",
#else
	fprintf(stderr,"nm=%s scnum=%o sp_val=%x <=> sect1=%o sect2=%o addr=%x\n",
#endif
		sp->n_name, sp->n_scnum, sp->n_value, sect1, sect2, addr);
#endif
			if(sp->n_sclass == C_EXT && sp->n_value <= addr
		  		&& sp->n_value > value) {
#ifdef DEBUG
#if iAPX286
	fprintf(stderr,"save=%lx value=%lx\n", save, value);
#else
	fprintf(stderr,"save=%x value=%x\n", save, value);
#endif
#endif
				value = sp->n_value;
				save = sp;
				if (sp->n_value == addr)
					return(save);
			}
#if !iAPX286
/* take out the corresponding } for removed conditional */
		}
#endif
	}
	return(save);
}

struct syment *
symsrch(s)
register char *s;
{
	register struct syment *sp;
	register struct syment *found;

#if iAPX286
	found = NULL;
#else
	found = 0;
#endif
	for(sp = stbl; sp < &stbl[symcnt]; sp++) {
		if(sp->n_sclass == C_EXT && strncmp(sp->n_name, s, 8) == 0) {
			found = sp;
			break;
		}
	}
	return(found);
}

prnm(s)
register char *s;
{
	register char *cp;
	register struct syment *sp;
	struct syment *nmsrch();

	printf("%-8.8s ", s);
	if((sp = nmsrch(s)) == NULL) {
		printf("no match\n");
		return;
	}
	printf("%08.8lx  ", sp->n_value);
#if iAPX286
	printf("\n");
#else
/* need to go to secnhdr for this information so ignore for time being */
	switch(sp->n_scnum) {
	case N_TEXT:
		cp = " text";
		break;
	case N_DATA:
		cp = " data";
		break;
	case N_BSS:
		cp = " bss";
		break;
	case N_UNDEF:
		cp = " undefined";
		break;
	case N_ABS:
		cp = " absolute";
		break;
	default:
		cp = " type unknown";
	}
	printf("%s\n", cp);
#endif
}

struct syment *
nmsrch(s)
	register  char  *s;
{
	char	ct[20];
	register  struct  syment  *sp;
	struct syment *symsrch();

	if(strlen(s) > 19)
		return(0);
	if((sp = symsrch(s)) == NULL) {
#if iAPX286
		strcpy(ct, s);
#else
		strcpy(ct, "_");
		strcat(ct, s);
#endif
		sp = symsrch(ct);
	}
	return(sp);
}

prod(addr, units, style)
#if iAPX286
	ADDR	addr;
#else
	unsigned	addr;
#endif
	int	units;
	char	*style;
{
#if iAPX286
	long 	physaddr;
#endif
	register  int  i;
	register  struct  prmode  *pp;
	int	word;
	long	lword;
	char	ch;
	extern	struct	prmode	prm[];

	if(units == -1)
		return;
	for(pp = prm; pp->pr_sw != 0; pp++) {
		if(strcmp(pp->pr_name, style) == 0)
			break;
	}
#if iAPX286
	if((physaddr = sysvad(addr,sizeof (long))) == -1L)
		error("bad address");
	if(lseek(mem, physaddr, 0) == -1L) {
#else
	if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
#endif
		error("bad seek of addr");
	}
	switch(pp->pr_sw) {
	default:
	case 0:
		error("invalid mode");
		break;

	case OCTAL:
	case DECIMAL:
		if(addr & 01) {
			printf("warning: word alignment performed\n");
			addr &= ~01;
#if iAPX286
			if((physaddr = sysvad(addr,sizeof (long))) == -1L)
				error("bad address");
			if(lseek(mem, physaddr, 0) == -1L) {
#else
			if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
#endif
				error("bad seek of addr");
			}
		}
		for(i = 0; i < units; i++) {
			if(i % 8 == 0) {
				if(i != 0)
					putc('\n', stdout);
#if iAPX286
				printf(FMT, addr + i * NBPW);
#else
				printf(FMT, (int)addr + i * NBPW);
#endif
				printf(":");
			}
#if iAPX286
			if((physaddr=sysvad((addr+i*NBPW),sizeof word)) == -1L)
				error("address cannot be reached");
			lseek(mem,physaddr,0);
#endif
			if(read(mem, &word, NBPW) != NBPW) {
				printf("  read error");
				break;
			}
			printf(pp->pr_sw == OCTAL ? " %7.7o" :
				"  %5u", word);
		}
		break;

	case LOCT:
	case LDEC:
		if(addr & 01) {
			printf("warning: word alignment performed\n");
			addr &= ~01;
#if iAPX286
			if((physaddr = sysvad(addr,sizeof (long))) == -1L)
				error("bad address");
			if(lseek(mem, physaddr, 0) == -1L) {
#else
			if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
#endif
				error("bad seek of addr");
			}
		}
		for(i = 0; i < units; i++) {
			if(i % 4 == 0) {
				if(i != 0)
					putc('\n', stdout);
#if iAPX286
				printf(FMT, addr + ( i * NBPW * 2));
#else
				printf(FMT, (int)addr + i * NBPW);
#endif
				printf(":");
			}
#if iAPX286
			if((physaddr=sysvad((addr+(i*2*NBPW)),sizeof (long))) == -1L)
				error("address cannot be reached");
			lseek(mem,physaddr,0);	
#endif
			if(read(mem, &lword, sizeof (long)) != sizeof (long)) {
				printf("  read error");
				break;
			}
			printf(pp->pr_sw == LOCT ? " %12.12lo" :
				"  %10lu", lword);
		}
		break;

	case CHAR:
	case BYTE:
	case STRING:
		for(i = 0; i < units; i++) {
#if iAPX286
			if (pp->pr_sw != STRING)
#endif
			if(i % (pp->pr_sw == CHAR ? 16 : 8) == 0) {
				if(i != 0)
					putc('\n', stdout);
#if iAPX286
				printf(FMT, addr + i * sizeof (char));
#else
				printf(FMT, (int)addr + i * sizeof (char));
#endif
				printf(":");
			}
#if iAPX286
			if((physaddr = 
			 sysvad((addr+i * sizeof (char)),sizeof (char))) == -1L)
				error("address cannot be reached");
			lseek(mem,physaddr,0);	
#endif
			if(read(mem, &ch, sizeof (char)) != sizeof (char)) {
				printf("  read error");
				break;
			}
			if(pp->pr_sw == CHAR)
				putch(ch);
			else if(pp->pr_sw == BYTE)
#if iAPX286
				printf(" %2.2x", ch & 0xff);
			else	/* for string */
				printf("%c",ch);
#else
				printf(" %4.4o", ch & 0377);
#endif
		}
		break;
	case HEX:
		if(addr & 01) {
			printf("warning: word alignment performed\n");
			addr &= ~01;
#if iAPX286
			if((physaddr = sysvad(addr,sizeof (long))) == -1L)
				error("bad address");
			if(lseek(mem, physaddr, 0) == -1L) {
#else
			if(lseek(mem, (long)(addr & VIRT_MEM), 0) == -1) {
#endif
				error("bad seek of addr");
			}
		}
		for(i = 0; i < units; i++) {
			if(i % 4 == 0) {
				if(i != 0)
					putc('\n', stdout);
#if iAPX286
				printf(FMT, addr + (i * NBPW * 2));
#else
				printf(FMT, (int)addr + i * NBPW);
#endif
				printf(":");
			}
#if iAPX286
			if((physaddr = 
			     sysvad((addr + (i*NBPW*2)),sizeof (long))) == -1L)
				error("address cannot be reached");
			lseek(mem,physaddr,0);	
#endif
			if(read(mem, &lword, sizeof (long)) != sizeof (long)) {
				printf("  read error");
				break;
			}
#if iAPX286
			printf(" %08lx", lword);
#else
			printf(" %08x", lword);
#endif
		}
		break;


	}
	putc('\n', stdout);
}
