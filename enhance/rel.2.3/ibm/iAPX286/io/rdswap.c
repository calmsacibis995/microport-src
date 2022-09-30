/*
 * Copyright (@) 1987 Microport Systems Inc. All rights reserved.
 * 
 * Ram Disk Driver for the swap device; Microport's System V/AT, Rel. 5.2
 * 	Initial Coding:	uport!dwight	Sat Apr 18 23:54:18 PST 1987
 * With this driver, one uses memory as the swap device. However, in
 * order to run in small amounts of memory, one doesn't simply malloc
 * a bunch of ram. Instead, you malloc each blkno as it's needed.
 *
 * Modification History:
 */
#ident "@(#)rdswap.c	1.0"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/seg.h"
#include "sys/map.h"

#define OK	1
#define FAIL	0

int rds0 = 50;				/* selector used by this driver */
int rds1 = 51;				/* selector used by this driver */

int rddebug = 0x0000;
int rdinitflag = 0;			/* 1 = ramdisk has been initialized */

int rdslimit = 0;			/* flag for ram disk. see rdinit */

extern long mapin();
extern bscopy();

/* rdopen(dev):	Open the ramdisk for use.
 */
unsigned long limit;

struct buf rdlist;		/* head of the memory used for the ram disk */
struct buf rrdbuf;		/* buffer for raw io */

rdopen(dev)
dev_t dev;
{
	extern int rdinit();

	if (rdinit() == FAIL) {
		u.u_error = EIO;
		return;
	}
}

/* rdclose(dev):	Close the ramdisk
 *	A dummy routine at this time.
 */
rdclose(dev)
dev_t	dev;
{
}

/* rdstrategy:
 * 	Does the transfer.
 */
rdstrategy(bp)
register struct buf *bp;
{
	int x;
	paddr_t physaddr();
	
	bp->b_resid = bp->b_bcount;

	/* validate io according to these rules:
	 *	any request that starts inside the ramdisk is ok.
	 *	if it's going to run over the end, trim back b_bcount.
	 *	after i/o, b_resid will show how many bytes weren't xfered.
	 *	any request beyond the end of the ramdisk won't be serviced,
	 *	and will be bounced back with ENXIO.
	 */

	/* request within bounds of ram disk? */

	if (bp->b_blkno > limit) {
		printf("rdstrat: over limit = %ld\n", limit);
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}

	/* within the ram disk's limits. will the request go over? */

	if ((daddr_t) bp->b_blkno == limit) {
		printf("rdstrat: at the limit\n");
		if ((bp->b_flags & B_READ) != B_READ) {
			bp->b_error = ENXIO;
			bp->b_flags |= B_ERROR;
		}
		iodone(bp);
		return;
	}

	/* req over? */
	if ((bp->b_blkno + ((bp->b_bcount + 511)/512) > ((daddr_t) limit))) {
		iodone(bp);
		return;
	}

	/* req is valid. get physical address of buffer */
	bp->b_physaddr = physaddr(bp->b_un.b_addr);

	x = splbio();

	/*
	 * Do the copy, via rdstart(). 
	 */
	 rdstart(bp);
	 splx(x);
}

/* rdstart():	Performs the actual transfer.
 */
rdstart(bp)
struct buf *bp;
{
	struct buf *vabp;		/* virtual address buffer ptr */
	daddr_t bno;			/* ramdisk block # */
	extern unsigned int rdxfer();
	extern struct buf *rdfindbuf();

	bno = bp->b_blkno;

	while (bp->b_resid) {		/* while's there's data to xfer */
		/* find the ramdisk block that corresponds to our request */
		if ((vabp = rdfindbuf(bno)) == NULL) {
			bp->b_error = EIO;	/* buffer not found */
			bp->b_flags |= B_ERROR;
			iodone(bp);
			return;
		}

		bp->b_resid -= rdxfer(bp, vabp);
		bno++;
	}

	iodone(bp);
}

/* rdread():	
 * 	raw device interface, for reads.
 */
rdread(dev)
dev_t	dev;
{
	/* check here if minor dev and request is OK */
	/* if not, post u.u_error = ENXIO, and return */
	/* physio(rdstrategy, &rflbuf, dev, B_READ); */
}

/* rdwrite():	
 * 	raw device interface, for writes.
 */
rdwrite(dev)
dev_t	dev;
{
	/* check here if minor dev and request is OK */
	/* if not, post u.u_error = ENXIO, and return */
	/* physio(rdstrategy, &rflbuf, dev, B_WRITE); */
}

/* rdioctl()
 * 	A null routine at this time.
 */
rdioctl()
{
	u.u_error = ENXIO;
	return(ENXIO);
}

/* rdprint():
 * 	print error messages
 */
rdprint (dev,str)
dev_t	dev;
char	*str;
{
	printf("%s on  Ram Disk partition %d\n", str, minor(dev));
}

/* rdinit:
 *	build the memory used for the ram disk.
 *	the rdlist scheme is used because we cannot be assured of getting
 *	contiguous addresses through malloc(). 
 *
 *	returns OK if the ram disk is properly initialized.
 *	returns FAIL if there were problems (malloc couldn't get all of
 *	the memory that the driver wanted.
 *
 *	the rdlist queue is NULL terminated, linked by the b_forw
 *	pointers only. the catch here is that each buffer has *physical*
 *	addresses in b_addr and b_forw. 
 */
static paddr_t cpbsa, cpbba;

int
rdinit()
{
	extern int nswap;

	if (rdinitflag++)
		return(OK);

	rdlist.b_bcount = 0;

	/* Nothing is on the queue yet.  */

	rdlist.b_forw = NULL;

	if (rdslimit == 0) {			/* is this the swap dev? */
		limit = (unsigned long) nswap;
	} else {				/* must just be a ram disk */
		limit = rdslimit;
	}
	return(OK);
}

struct buf *
rdfindbuf(bno)
daddr_t bno;
{
	paddr_t pabp;		/* physical address of buffer pointer */
	unsigned count;
	struct buf *vabp;	/* virtual address of buffer pointer */
	extern struct buf *rdmalloc();

	count = 0;
	pabp = (paddr_t) rdlist.b_forw;
	vabp = (struct buf *) mapin(pabp, rds1);

	while (pabp != NULL) {
		if (vabp->b_blkno == bno) {
			break;
		}
		if (count++ > rdlist.b_bcount) {
			return((struct buf *) NULL);
		}

		pabp = (paddr_t) vabp->b_forw;
		vabp = (struct buf *) mapin(pabp, rds1);
	} 

	if (pabp == NULL) {
	/* the block # on the ramdisk wasn't found. so malloc a buffer
	 * and put it on the queue.
	 */
		vabp = rdmalloc(bno);
	}

	return((struct buf *) vabp);
}

unsigned int
rdxfer(bp, vabp)
struct buf *bp;					/* I/0 buffer */
struct buf *vabp;				/* virtual addr buffer ptr */
{
	caddr_t ramblock;			/* ramdisk block addr */
	unsigned count;				/* # transfer bytes */
	unsigned long bpaddr;

	ramblock = vabp->b_un.b_addr;
	count = bp->b_resid;
	bpaddr = bp->b_physaddr;

#ifdef RAWRD
	if (count > SBUFSIZE) {			/* req crosses ram block */
		count = SBUFSIZE;
	}
#endif RAWRD
		
	if (bp->b_flags & B_READ) {		/* read xfer */
		bscopy((unsigned long) ramblock, (unsigned long) bpaddr, count);
	} else {				/* write xfer */
		bscopy((unsigned long) bpaddr, (unsigned long) ramblock, count);
	}
#ifdef RAWRD
	bp->b_physaddr += (unsigned long) count;
#endif RAWRD
	return(count);
}

/* rdmalloc: malloc a buffer area in memory to be used for the swap device.
 *	input: block # desired.
 *	output:	NULL if failure; buffer address is successful.
 */
struct buf *
rdmalloc(bno)
daddr_t bno;
{
	struct buf *pabp;		/* phys addr of buffer pointer */
	struct buf *vabp;		/* virt addr of buffer pointer */
	unsigned int cba;		/* click buffer addr - tmp variable */

	/* get a buffer structure from memory */
	cba = malloc(coremap, btoc(sizeof(struct buf)));
	if (cba == 0) {
		mfree(coremap, sizeof (struct buf), cba);
		return(NULL);
	}
	pabp = (struct buf *) ctob((unsigned long) cba);
	vabp = (struct buf *) mapin(pabp, rds0); 

	/* malloc buffer data area: */
	cba = malloc(coremap, btoc(SBUFSIZE));
	if (cba == 0) {
		mfree(coremap, SBUFSIZE, cba);
		mfree(coremap, sizeof (struct buf), btoc(pabp));
		return(NULL);
	}
	
	vabp->b_un.b_addr = (caddr_t) ctob((unsigned long) cba);

	/* initialize buffer for use */
	vabp->b_dev = NODEV;
	vabp->b_flags = B_BUSY;
	vabp->b_bcount = 0;
	vabp->b_blkno = bno;

	/* insert new buffer at head of queue - not efficent, but simple */
	vabp->b_forw = rdlist.b_forw;
	rdlist.b_forw = (struct buf *) pabp;
	rdlist.b_bcount++;

	return((struct buf *) vabp);
}
