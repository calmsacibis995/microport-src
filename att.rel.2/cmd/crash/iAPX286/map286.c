/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)map286.c	1.4 - 85/08/12 */
#include "crash.h"
long lseek();
extern struct uarea x;


static unsigned long baseaddr;
static struct seg_desc seg_area;


prldt(proc,start,number,prntopt)
int proc;
unsigned start,number,prntopt;
{
	int ret;

	if(proc >(int)v.ve_proc)
	{
		printf("%d out of range, use Process Table Slot \n",proc);
		return(1);
	}
	if((ret=getuarea(proc)) == -1)
	{
		error("bad read of uarea");
		return(1);
	}
	if (ret == SWAPPED)
	{
		error("process is swapped.");
		return(1);
	}
	readprsel((int)((long)x.u.u_ldtadv>>16),start,number,prntopt);
	return(0);
}

prgdt(start,number)
unsigned start,number;
{
	readprsel(GDT_SEL<<3,start,number,0);
}

pridt(start,number)
unsigned start,number;
{
	readprsel(IDT_SEL<<3,start,number,0);
}

readprsel(sel,start,number,not_print)
unsigned sel,start,number,not_print;
{
	unsigned limit;
	lseek(mem,(long)(PHYS_GDT + sel),0);
	read(mem,&seg_area,sizeof seg_area);
	if(start >(unsigned)seg_area.sd_limit)
		return;
	limit = (((unsigned)seg_area.sd_limit - (start&0xfff8))/DSC_SZ) + 1;
	if (number < limit)
		limit = number;
	baseaddr=((unsigned long)(seg_area.sd_hibase)<<16)+
					(unsigned long)seg_area.sd_lowbase;
	if(not_print)
		return;
	printf("limit = %.4x physical address = %.6lx\n",seg_area.sd_limit,baseaddr);
	prsegmap(limit,start&0xfff8,(baseaddr+((unsigned long)start&0xfff8L)));
}

prsegmap(maxlim,start,physaddr)
unsigned maxlim,start;
long physaddr;
{
	struct seg_desc seg;
	int i=start;
	long addr = physaddr;
	if (lseek(mem,physaddr,0) == -1L)
		{
		error("segment address out of file limit");
		return(-1);
		}
	for( ; maxlim > 0; i += DSC_SZ, addr += DSC_SZ, maxlim--)
	{
		if (read(mem,&seg,sizeof seg) != sizeof seg)
		{
			error("read error");
			return(-1);
		}
		else
		{
			if ((seg.sd_limit != 0) || (seg.sd_lowbase != 0) ||
			     (seg.sd_hibase != '\0') || (seg.sd_access != '\0'))
			{
			printf("sel= %.3x,seg= %.3x ",i,i>>3);
			if (seg.sd_access&DSC_PRESENT)
				printf("   pres.");
			else
				printf("notpres.");
			printf(" priv. %d",((seg.sd_access&DSC_DPL)>>5));
			if (seg.sd_access&DSC_SEG)
			{
				printf(" seg.");
				printf(" limit= %.4x addr= %.6lx",seg.sd_limit,
					(((unsigned long)(seg.sd_hibase))<<16)+ 
					((long)seg.sd_lowbase&0xffffL));
				if (seg.sd_access&SD_CODE)
				{
					printf(" exec");
					if (seg.sd_access&SD_CONFORM)
						printf(" conf.");
					if (seg.sd_access&SD_READ)
						printf(" read");
				}
				else
				{
				if (seg.sd_access&SD_WRITE)
					printf(" write");
				if (seg.sd_access&SD_EXPND_DN)
					printf(" exp.d.");
				}
			}
			else   /* not segment */
			{
				printf("  ");
				if (((seg.sd_access&0xf)==CD_AVAIL_TSK) ||
					((seg.sd_access&0xf)==CD_LDT) ||
					((seg.sd_access&0xf)==CD_BUSY_TSK))
				{
					printf(" limit= %.4x addr= %.6lx",
						seg.sd_limit,
						(long)(seg.sd_hibase<<16)+ 
						((long)seg.sd_lowbase&0xffffL));
				}
				switch(seg.sd_access&0xf)
				{
				case CD_AVAIL_TSK:
					printf(" available tss");
					break;
				case CD_LDT:
					printf(" ldt");
					break;
				case CD_BUSY_TSK:
					printf(" busy tss");
					break;
				case G_CALL:
					printf(" call gate");
					printf(" dest.= %.8lx",
						((((long)seg.sd_lowbase)<<16) +
						((long)seg.sd_limit&0xffffL)));
					printf(" wordc= %d",(seg.sd_hibase&
									0x1f));
					break;
				case G_TASK:
					printf(" task gate");
					printf(" sel.= %.4x",seg.sd_lowbase);
					break;
				case G_INT:
					printf(" int. gate");
					printf(" dest.= %.8lx",
						((((long)seg.sd_lowbase)<<16) +
						((long)seg.sd_limit&0xffffL)));
					break;
				case G_TRAP:
					printf(" trap gate");
					printf(" dest.= %.8lx",
						((((long)seg.sd_lowbase)<<16) +
						((long)seg.sd_limit&0xffffL)));
					break;
				default:
					printf(" invalid");
				}
			}
			printf("\n");
			}
		}	
	}
}
prstacki(proc)
int proc;
{
	unsigned i;
	int ret;

	if(proc >(int)v.ve_proc)
	{
		printf("%d out of range, use Process Table Slot \n",proc);
		return;
	}
	if((ret=getuarea(proc)) == -1)
	{
		error("bad read of uarea");
		return;
	}
	if (ret == SWAPPED)
	{
		error("process is swapped.");
		return;
	}
#define z x.u.u_stack
	i = KSTACKSZ - 1;
/* print out the system stack interface */
	printf(
"   |  %%ss   |  %%sp   |flg /a9 |%%cs /a8 |%%ip /a7 | iv /a6 | v  /a5 | p  /a4 |");
	printf("\n");
	printf(
"   |--------|--------|--------|--------|--------|--------|--------|--------|");
	printf("\n");
	printf(
"   |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |"
,z[i],z[i-1],z[i-2],z[i-3],z[i-4],z[i-5],z[i-6],z[i-7]);
	printf("\n");
	printf("\n");
	i -= 8;
	printf(
"   | p  /a3 | p  /a2 | p  /a1 | p  /a0 | p  %%cs | p  %%ip |  %%ax   |  %%cx   |");
	printf("\n");
	printf(
"   |--------|--------|--------|--------|--------|--------|--------|--------|");
	printf("\n");
	printf(
"   |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |"
,z[i],z[i-1],z[i-2],z[i-3],z[i-4],z[i-5],z[i-6],z[i-7]);
	printf("\n");
	printf("\n");
	i -= 8;
	printf(
"   |   %%dx  |   %%bx  |   %%sp  |   %%bp  |   %%si  |   %%di  |   %%ds  |   %%es  |");
	printf("\n");
	printf(
"   |--------|--------|--------|--------|--------|--------|--------|--------|");
	printf("\n");
	printf(
"   |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |  %.4x  |"
,z[i],z[i-1],z[i-2],z[i-3],z[i-4],z[i-5],z[i-6],z[i-7]);
	printf("\n");
	return;
}
prodldt(proc,addr,number,style)
int proc;
ADDR addr;
unsigned number;
char * style;
{
	int ret;

	if(proc >(int)v.ve_proc)
	{
		printf("%d out of range, use Process Table Slot \n",proc);
		return;
	}
	if((ret=getuarea(proc)) == -1)
	{
		error("bad read of uarea");
		return;
	}
	if (ret == SWAPPED)
	{
		error("process is swapped.");
		return;
	}
}





prodl(proc,addr, units, style)
int	proc;
ADDR	addr;
unsigned int	units;
char	*style;
{
	long 	physaddr;
	register  int  i;
	register  struct  prmode  *pp;
	unsigned int	word;
	unsigned int 	base;
	unsigned int     limit;
	unsigned int	no_bytes;
	unsigned int 	no_units;
	long	lword;
	char	ch;
	extern	struct	prmode	prm[];

	/* translate the virtual address to real */

	prldt(proc,(unsigned int)(addr>>16),1,NOPRINT);

	/* now seg_area has ldt segment desc */
	/* and baseaddr its base physical address */

	if(lseek(mem,(baseaddr+(unsigned int)((addr>>16)&0xfff8)),0) == -1L)
	{
		error("bad seek of seg addr");
	}
	read(mem,&seg_area,sizeof seg_area);

/* if segment not available then say so			*/

	if(!(seg_area.sd_access&DSC_PRESENT))
	{
		error("segment not available");
		return;
	}
	if(!(seg_area.sd_access&DSC_SEG))
	{
		error("segment not an area descriptor");
		return;
	}

	for(pp = prm; pp->pr_sw != 0; pp++) {
		if(strcmp(pp->pr_name, style) == 0)
			break;
	}
	switch(pp->pr_sw) 
	{
		default:
		case 0:
			error("invalid mode");
			return;
		case LDEC:
		case LOCT:
			no_bytes = 4;
			break;
		case CHAR:
		case BYTE:
			no_bytes = 1;
			break;
		case HEX:
		case OCTAL:
		case DECIMAL:
			no_bytes = 2;
			break;
	}

	word = no_bytes*units;

	if((!(seg_area.sd_access&SD_CODE))&&(seg_area.sd_access&SD_EXPND_DN))
	{
		base = (((unsigned)0xffff)-(unsigned)seg_area.sd_limit);
		if((unsigned long)((addr&0x0000ffff)+(unsigned long)word) >= 
			(unsigned long)seg_area.sd_limit+1)
		{
			if((unsigned)addr <= ((unsigned)seg_area.sd_limit))
			{
				no_units=((unsigned int)addr + word -
					(unsigned)seg_area.sd_limit-1);
				if (no_units>base)
					no_units = base;
				no_units /= no_bytes;
				addr = (addr&0xffff0000)+
					(unsigned long)seg_area.sd_limit+1;
				printf("warning: address is %lx  no. units is %d\n",addr,no_units);
			}
			else
			{
				if(((long)addr&0xffff)+word >0x10000)
				{
					no_units = (unsigned int)((long)0x10000-
					     ((long)addr&0xffff))/no_bytes;
					printf("warning: no. units is %d \n",no_units);
				}
				else
				{
					no_units = units;
				}
			}
		}
		else
		{
			error("area not within segment");
			return;
		}
	}
	else
	{
		if((unsigned int)addr >(unsigned)seg_area.sd_limit)
		{
			error("area not within segment");
			return;
		}
		limit = (unsigned)seg_area.sd_limit - (unsigned int)(addr) +1;
		no_units = units;
		if (word > limit)
		{
		no_units = limit/no_bytes;
			printf("warning: units reduced to %d \n",no_units);
		}
	}
	physaddr=((unsigned long)(seg_area.sd_hibase)<<16)+(addr&0x0000ffff)+
					(unsigned long)seg_area.sd_lowbase;
	if(lseek(mem, physaddr, 0) == -1L) {
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
			if(lseek(mem, physaddr, 0) == -1L) {
				error("bad seek of addr");
			}
		}
		for(i = 0; i < no_units; i++) {
			if(i % 8 == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, addr + i * NBPW);
				printf(":");
			}
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
			if(lseek(mem, physaddr, 0) == -1L) {
				error("bad seek of addr");
			}
		}
		for(i = 0; i < no_units; i++) {
			if(i % 4 == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, addr + ( i * NBPW * 2));
				printf(":");
			}
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
		for(i = 0; i < no_units; i++) {
			if(i % (pp->pr_sw == CHAR ? 16 : 8) == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, addr + i * sizeof (char));
				printf(":");
			}
			if(read(mem, &ch, sizeof (char)) != sizeof (char)) {
				printf("  read error");
				break;
			}
			if(pp->pr_sw == CHAR)
				putch(ch);
			else
				printf(" %2.2x", ch & 0xff);
		}
		break;
	case HEX:
		if(addr & 01) {
			printf("warning: word alignment performed\n");
			addr &= ~01;
			if(lseek(mem, physaddr, 0) == -1L) {
				error("bad seek of addr");
			}
		}
		for(i = 0; i < no_units; i++) {
			if(i % 8 == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, addr + (i * NBPW ));
				printf(":");
			}
			if(read(mem, &word, sizeof (word)) != sizeof (word)) {
				printf("  read error");
				break;
			}
			printf(" %04x", word);
		}
		break;


	}
	putc('\n', stdout);
}
