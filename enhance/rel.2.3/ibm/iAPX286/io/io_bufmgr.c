/* @(#)io_bufmgr.c	1.8 */

/* 
 * IO Buffer Manager
 *
 * Routines:
 * io_getbuf	- Obtains a memory buffer and a selector to address it with.
 * io_expbuf	- Expands a buffer previously obtained by io_getbuf
 * io_freebuf   - Frees a buffer obtained by io_getbuf.
 * io_mapbuf	- Maps a physical buffer to a kernel virtual address.
 * io_unmapbuf	- Unmaps a physical buffer from a kernel virtual address.
 * io_getsel	- Get a selector from the io pool
 * io_freesel	- Free a selector
 * io_buftosel	- Translates kernel virtual buffer address to global selector.
 *
 * Modification History:
 * 	uport!mike Thu Jun 18 15:51:16 PDT 1987
 *	Created.
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/map.h"
#include "sys/sysmacros.h"
#include "sys/io_bufmgr.h"

/* labeled global descriptors */
extern struct seg_desc gdt[], io_bufsel[], io_bufselend[];
static struct seg_desc *io_bufsel0 = io_bufsel;	/* first free selector */

/*
 * io_getbuf
 *	Obtains a memory buffer and a selector to address it with.
 *	Returns kernel virtual address or 0 if not available;
 *
 *   flags & 1 = clear buffer
 *   flags & 2 = prevent buffer from spanning 64k boundary (not yet implemented)
 */
char *
io_getbuf (size, flags)
    unsigned int size;
    int flags;
{
    unsigned int temp, malloc(), sel;
    unsigned long blocksize;
    paddr_t physbuf;
    char *kvadrs;

    if (!(sel = io_getsel (0)))
	return (char *) 0;

    if ((blocksize = (size + 1) & 0xFFFE) == 0)
	blocksize = 0x10000L;
    temp = btoc (blocksize);

    if (!(temp = malloc (coremap, temp))) {
	io_freesel (sel);
	return (char *) 0;
    }
    physbuf = (paddr_t) ctob ((unsigned long) temp);

    gdt [sel].sd_lowbase =  physbuf         & 0xFFFF;
    gdt [sel].sd_hibase  = (physbuf >> 16)  & 0x00FF;
    gdt [sel].sd_limit   =  blocksize - 1;
    gdt [sel].sd_access  =  ACC_KDATA | DSC_PRESENT;
    kvadrs = (char *) gstokv ((unsigned long) sel);
    if (flags & 1)
	wzero (kvadrs, (unsigned int) (blocksize / 2));

    return kvadrs;
}

mmudesc (sel, addr, lim, acc)
    int sel;
    paddr_t addr;
    int lim;
    int acc;
{
    gdt [sel].sd_lowbase =  addr         & 0xFFFF;
    gdt [sel].sd_hibase  = (addr >> 16)  & 0x00FF;
    gdt [sel].sd_limit   =  lim;
    gdt [sel].sd_access  =  DSC_PRESENT | DSC_SEG | KER_DPL
			 |  (acc ? SD_WRITE : SD_READ);
}

static
io_adrstosel (adrs, subrname)
    char *adrs;
    char *subrname;
{
    int gdtx = io_buftosel (adrs);

    if (gdtx < (io_bufsel - gdt) || (io_bufselend - gdt) <= gdtx)  {
	printf ("%s: invalid address\n", subrname);
	return 0;
    }
    return gdtx;
}

/*
 * io_expbuf
 *	Expands a buffer previously obtained by io_getbuf.
 *	Returns offset to new section (word aligned) or 0 if not expandable.
 *
 *	Note: Needs to check if current buffer can be expanded in-place.
 */
io_expbuf (oldadrs, expandsize, flags)
    char *oldadrs;
    unsigned int expandsize;
    int flags;
{
    char *newadrs;
    unsigned int gdtold, gdtnew, copysize;
    unsigned long newsize;
    struct seg_desc gdttmp;
    int x;

    if (! (gdtold = io_adrstosel (oldadrs, "io_expbuf")))
	return 0;

    gdttmp = gdt [gdtold];
    if ((copysize = (unsigned int) gdttmp.sd_limit + 1) == 0)
	return 0;

    newsize = (unsigned long) copysize + (unsigned long) expandsize;
    if (newsize > 0x10000L)
	return 0;

    if (newadrs = io_getbuf ((unsigned int) newsize, flags)) {
	gdtnew  = io_buftosel (newadrs);

	x = spl7();		/* not sure this is needed */
	wcopy (oldadrs, newadrs, copysize >> 1);
	gdt [gdtold] = gdt [gdtnew];
	splx (x);

	gdt [gdtnew] = gdttmp;
	io_freebuf (newadrs);
	return copysize;
    }
    return 0;
}

/* 
 * io_mapbuf
 *	Maps a physical buffer to a logical kernel virtual address.
 */
char *
io_mapbuf (addr, size)
    paddr_t addr;
    unsigned int size;
{
    int sel = io_getsel (1);		/* wait for selector */

    gdt [sel].sd_lowbase =  addr        & 0xFFFF;
    gdt [sel].sd_hibase  = (addr >> 16) & 0x00FF;
    gdt [sel].sd_limit   =  size - 1;
    gdt [sel].sd_access  =  ACC_KDATA | DSC_PRESENT;
    return (char *) gstokv ((unsigned long) sel);
}

/*
 * io_freebuf
 *     Frees a buffer obtained by io_getbuf.
 */
void
io_freebuf (adrs)
    char *adrs;
{
    unsigned int bufcadrs, bufcsize, gdtx;
    paddr_t physbuf;

    if (gdtx = io_adrstosel (adrs, "io_freebuf")) {

	physbuf = (paddr_t) (((unsigned long) gdt [gdtx].sd_hibase << 16)
			   |  ((unsigned long) gdt [gdtx].sd_lowbase));
	bufcadrs = btoc ((unsigned long) physbuf);
	bufcsize = btoc ((unsigned long) gdt [gdtx].sd_limit + 1);
	mfree (coremap, bufcsize, bufcadrs);
	io_freesel (gdtx);
    }
}

/*
 * io_unmapbuf
 *     Frees the selector being used to map a buffer.
 */
void
io_unmapbuf (adrs)
    char *adrs;
{
    unsigned int gdtx;

    if (gdtx = io_adrstosel (adrs, "io_unmapbuf"))
	io_freesel (gdtx);
}

paddr_t
io_physaddr (sel)
    int sel;
{
    return (paddr_t) (((unsigned long) gdt [sel].sd_hibase << 16)
		   |  ((unsigned long) gdt [sel].sd_lowbase));
}

/*
 * io_getsel
 *	Get a selector from the io pool.
 */
io_getsel (wait)
    int wait;
{
    struct seg_desc *gdtp;
    int x;

    while (1) {
	x = spl7();			/* not sure this is needed */
	for (gdtp = io_bufsel0; gdtp < io_bufselend; gdtp++)
	    if (gdtp->sd_access == 0) {
		gdtp->sd_access = DSC_PRESENT;
		splx (x);
		io_bufsel0 = gdtp + 1;
		return gdtp - gdt;
	    }
	splx (x);
	if (wait)
	    sleep (&io_bufsel0, PZERO + 1);	/* wait for selector */
	else
	    return 0;
    }
}

/* 
 * io_freesel
 *	Free a selector.
 */
void
io_freesel (sel)
    int sel;
{
    struct seg_desc *gdtp;

    if (sel < (io_bufsel - gdt) || (io_bufselend - gdt) <= sel)
	printf ("io_freesel: invalid selector\n");

    gdtp = &gdt [sel];
    gdtp->sd_access = 0;	/* free up selector */
    if (io_bufsel0 > gdtp)
	io_bufsel0 = gdtp;	/* update first free selector */

    wakeup (&io_bufsel0);	/* in case someone is waiting */
}

/* === */
