/* @(#)xenix.h	1.1 */

/* descriptor allocation routines */
#define	dscralloc()	io_getsel (0);
#define	dscrfree	io_freesel;
#define	dscraddr	io_physaddr;
#define	RW		1;
#define	RO		0;
