/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)sysvad.c	1.3 - 85/08/09 */
#include	"crash.h"
#ifdef	vax
int Sbrptr;	/* start of page table in memory */

vaddrinit()
{
	register i;

	if(lseek(mem,(long)(SYM_VALUE(Sbrpte)),0) == -1) {
		error("seek error on sbrpte");
		return(-1);
	}
	if(read(mem,(char *)&Sbrptr, sizeof Sbrptr) == -1) {
		error("read error on Sbrptr");
		return(-1);
	}
	Sbrptr &= VIRT_MEM;

	/*for (i = 0; i < (sizeof Pte/ sizeof Pte[0]); i += 128) {
		if(lseek(mem, Sbrptr + i, 0) == -1)
			error("seek error on memory page");
		if(read(mem, (char *)&Pte[i], 512) != 512)
			error("read error on memory page");
	}*/
}


sysvad(vad)
unsigned vad;
{
	int	i;
	int	ww, spte;
	int	binp; /* byte in page */

	vad &= VIRT_MEM;
	binp = vad & 0x1ff;
	ww = vad >> 9;
	/*if (ww < 1024)
		return(((Pte[ww]&0x1fffff) << 9) | binp);*/
#ifdef DEBUG
	printf("addr %x out of Pte\n", ww);
#endif
	ww = ww * 4 + Sbrptr;
	lseek(mem, (long)ww, 0);
	read(mem, &spte, sizeof spte);
	return(((spte&0x1fffff) << 9) | binp);
}
#endif
#if iAPX286
long
sysvad(vad,len)
unsigned long vad;
unsigned len;
{
/*  generate the physical address from the virtual address 	*/
/*	check for the address in gdt.				*/
/*	check that segment is data segment			*/
/*	check that segment displacement + len lies in the limit	*/
	long physaddr;
	struct seg_desc seg;
	unsigned seg_sel;
	seg_sel = vad>>16;
/*   Is the segment selector in the gdt?			*/
	if (seg_sel&SEL_TI)
		return(-1L); /*  error if not gdt */
	if (lseek(mem,(long)PHYS_GDT+(unsigned long)(seg_sel&SEL_INDEX),0)==-1)
		return(-1L);
	if (read(mem,&seg,sizeof seg) != sizeof seg)
		return(-1L);
/*   Check to see if in limits					*/
	physaddr = ((unsigned long)seg.sd_hibase<<16) + 
		(unsigned long)seg.sd_lowbase + (vad & 0xffff);
#if DEBUG
	printf("physaddr=%lx,va=%lx,len=%x\n",physaddr,vad,len);
#endif
	/*
	**	decrement len by 1 
	**	a limit of 1 in (gl)dt implies a len of 2
	*/

	len-- ;

	if ((seg.sd_access&(ACC_KDATA)) == (ACC_KDATA))
	{
		if (seg.sd_access&SD_EXPND_DN)
		{
			if (((unsigned)vad > seg.sd_limit) && 
				((unsigned long)((unsigned)vad)+
					(unsigned long)len < 0x10000))
			{
				return(physaddr);
			}
			else
			{
				return(-1L);
			}
		}
		else
		{
			if ((unsigned long)((unsigned)vad)+(unsigned long)len
				<= (unsigned long)seg.sd_limit)
			{
				return(physaddr);
			}
		}
	}
	return(-1L);
}
#endif

readmem(buf, vad, len)
char *buf;
long vad;
int len;
{
#if iAPX286
	long	physaddr;
#endif
#ifdef	vax
	int	physaddr,
		i,
		binp,
		get,
		start;
#endif
#if iAPX286
	if ((physaddr = sysvad(vad,len)) != -1L)
	{
		if (lseek(mem,physaddr,0) != -1)
		{
			if (read(mem,&buf[0],len) == len)
			{
#if DEBUG
				printf("va=%lx,physaddr=%lx,len=%x\n",
					vad,physaddr,len);
#endif
				return(len);
			}
		}
	}
	return(0);
#endif
#ifdef	pdp11
#ifdef DEBUG
	printf("seeking to %lo\n", vad);
#endif
	if (lseek(mem, (long)vad, 0) == -1)
		return(0);
	if (read(mem, &buf[0], len) != len)
		return(0);
	return(len);
#endif
#ifdef	vax
	binp = vad & 0x1ff;
	start = 0;
	while (len) {
		physaddr = sysvad(vad);
		get = (binp + len > 0x200) ? 0x200 - binp : len;
		if (lseek(mem, physaddr, 0) == -1)
			return(0);
		if (read(mem, &buf[start], get) != get)
			return(0);
		start += get;
		len -= get;
		vad = (vad + 0x1ff) & ~0x1ff;
		binp = 0;
	}
#ifdef DEBUG
	printf("start = %d\n", start);
#endif
	return(start);
#endif
}
