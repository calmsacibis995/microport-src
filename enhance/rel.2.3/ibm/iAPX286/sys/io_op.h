/* uportid = "@(#)io_op.h	Microport Rev Id  1.3.3 6/18/86" */
/* 
 * This file defines various operations implemented as ioctl()
 * commands on /dev/mem.
 *	286 I/O operation user interface
 *	Reboot command ( sync; umount; reboot )
 *	Login table management
 *	Return information on system such as root, pipe, swap devices,
 *		swap and memory sizes.
 * LCN 1-15-86
 * M000:	uport!mike	Dec 1986
 * M001:	uport!dwight	Sat Jan 17 22:33:35 PST 1987
 *	Added ioctl to reboot the AT.
 * M002:	uport!rex	Thu Jan 22 1987
 *	Added commands to manage a login table.
 * M003:	uport!rex	Tue Feb 24 1987
 *	Added command to return system information
 */

typedef struct io_op {
	unsigned int	io_port;	/* Port number 			*/
	unsigned int	io_word;	/* Word data stored here	*/
	unsigned char	io_byte;	/* Byte data in here 		*/
	unsigned char	io_byte2;	/* Byte data in here 		*/
	unsigned int	io_port2;	/* Port number 			*/
} io_op_t;

struct sys_info {			/* M003 */
	int	bdevcnt;
	int	cdevcnt;
	dev_t	rootdev;
	dev_t	pipedev;
	dev_t	dumpdev;
	dev_t	swapdev;
	daddr_t	swplo;
	int	nswap;
	long	foundmem;
};

#define	IOCIOP_RB	(('I'<<8)|0)	/* Read  a byte 		*/
#define	IOCIOP_RW	(('I'<<8)|1)	/* Read  a word 		*/
#define	IOCIOP_WB	(('I'<<8)|2)	/* Write a byte			*/
#define	IOCIOP_WW	(('I'<<8)|3)	/* Write a word			*/
#define	IOCIOP_WB2	(('I'<<8)|4)	/* Write two bytes to same port */
#define	IOCIOP_WB21	(('I'<<8)|5)	/* Write 2 bytes -> port, ++port */
#define	IOCIOP_RWB2	(('I'<<8)|6)	/* Rd port2, Wrt 2 -> port	*/
#define	IOCIOP_RWB21	(('I'<<8)|7)	/* Rd port2, Wrt 2 -> port,++port */
#define	IOCIOP_RWBM2	(('I'<<8)|8)	/* Rd/Mask, then RWB2		*/
/* M001: */
#define	IOCIOP_REBOOT	(('D'<<8)|0)	/* Polite (sync/umount) reboot	*/
/* M003: */
#define	IOCIOP_SYSINFO	(('S'<<8)|0)	/* Information about system	*/
/* M002 */
#define	IOCIOP_LOGADD	(('L'<<8)|0)	/* Add to login table		*/
#define	IOCIOP_LOGDEL	(('L'<<8)|1)	/* Delete to login table	*/
