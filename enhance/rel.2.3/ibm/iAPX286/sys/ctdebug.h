/* uportid = "@(#)ctdebug.h	Microport Rev Id  2.3 6/29/87" */

/* Copyright 1987 by Microport. All Rights Reserved.
 *
 *	DEBUG switches and defines
 *
 * Initial Coding:
 *		uport!doug Thu Jan 22 20:25:07 PST 1987
 *		created for use in changing debug defines
 *
 */

/*
 *	DEFINE SWITCHES 
 *
 * Change this file to define various diagnostic switches, as well
 * as to determine the driver type and prefix.  Two driver types are
 * supported, a buffer manager, allowing reads and writes to a buffer
 * of available core, or a cartridge tape driver.
 *
 * The following "#ifdef" switches are supported: 
 *	 DEBUG		- Control level diagnostic printfs
 *	 MEMLOG		- Turns device into a buffer server
 *			- Allowing you to read and write to
 *			- a buffer.  You must open the Fast
 *			  buffer minor device 128.  
 *	 KERRLOG	- creates a kernel error loging function.
 *			- This is used in conjunction with the
 *			- MEMLOG switch to provide a printf function
 *			- that writes to the buffer.  This device
 *			- should always be read/only.  The dd utility
 *			- can be used to copy the printf messages to
 *			- a file.
 *
 * The following constants define parameters for the above modes.  
 *	 PREFIX		ct	- defines the driver name prefix
 *				  ct for cartridge tape drivers, or
 *				  mb for a memory buffer manager.
 *	 CTDBGLVL	3	- debug level mask, each debug
 *				level can be turned on and off
 *				by a bit in this 16 bit mask.
 *				The lsb controls Debug level 1.
 *				The next controls Debug level 2,
 *				and so on thru level 16.  The
 *				default (7) turns on the first 3 
 *				debug levels, 1-3.  This mask defines 
 *				the debug level only when DEBUG is 
 *				defined.
 *				
 *	 CTDELAY	20   	Time delay constant for cttimer()
 *				(debug level 1 must be on).
 *
 */
#define DEBUG
#define CTDBGLVL	0x1ff
#define INTDEBUG	(CTDBGLVL > 0xff)
#define CTDELAY		20
/*
#define MEMLOG		
#define KERRLOG
*/

/* type of driver Buffer Manager or Cartridge Tape driver */
#ifndef MEMLOG
#define CT
#endif

#ifdef	MEMLOG
/* Functions defined in the master file for the buffer manager */
#define CTOPEN		ctopen
#define CTCLOSE		ctclose
#define CTSTRATEGY	ctstrategy
#define CTREAD		ctread
#define CTWRITE		ctwrite
#define CTIOCTL		ctioctl
#else
/* Functions defined in the master file for the cartridge tape driver */
#define CTOPEN		ctopen
#define CTCLOSE		ctclose
#define CTSTRATEGY	ctstrategy
#define CTREAD		ctread
#define CTWRITE		ctwrite
#define CTIOCTL		ctioctl
#endif
/* Actual debug defines, this should not be changed.
 */
#ifndef	DBG
# ifdef	DEBUG
#  define DBG(l, p)	if (ctdbglvl & (1<<l)) { p; }
# else	not DEBUG
#  define DBG(l, p)
# endif	DEBUG
# define	DBG0(p)		DBG(0, p)		/* error diagnostics */
# define	DBG1(p)		DBG(1, p)		/* addresses & counts*/
# define	DBG2(p)		DBG(2, p)		/* kernel buffer */
# define	DBG3(p)		DBG(3, p)		/* DMA settings */
# define	DBG4(p)		DBG(4, p)		/* proc entry/exit */
# define	DBG5(p)		DBG(5, p)		/* ctinit / reset */
# define	DBG6(p)		DBG(6, p)		/* Functions 	*/
# define	DBG7(p)		DBG(7, p)		/* ctintr 	*/
# if INTDEBUG
#  define	DBG8(p)		DBG(8, p)		/* I/O buf list	*/
#  define	DBG9(p)		DBG(9, p)		/* data dump    */
#  define	DBG10(p)	DBG(10, p)		/* ctsetdma	*/
#  define	DBG11(p)	DBG(11, p)		/* _ctsetdma 	*/
#  define	DBG12(p)	DBG(12, p)		/* chkrdstatus	*/
#  define	DBG13(p)	DBG(13, p)		/* tapeioirq 	*/
#  define	DBG14(p)	DBG(14, p)		/* alternate buffer*/
#  define	DBG15(p)	DBG(15, p)		/* alternate buffer*/
# else NOT INTDEBUG
#  define	DBG8(p)	
#  define	DBG9(p)	
#  define	DBG10(p)
#  define	DBG11(p)
#  define	DBG12(p)
#  define	DBG13(p)
#  define	DBG14(p)
#  define	DBG15(p)
# endif INTDEBUG
#endif	DBG

extern int	ctdbglvl;
