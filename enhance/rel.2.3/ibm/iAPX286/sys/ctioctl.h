
/* uportid = "@(#)ctioctl.h	Microport Rev Id  1.3.8 1/9/87" */

/* Copyright 1987 by Microport. All Rights Reserved.
 *
 * TVI TeleCAT-286 Streaming Tape Device Driver 
 * References:
 *	TVI Streaming Tape Interface Hardware Specifications
 *
 * Initial Coding:
 *		unix-net!doug Fri Jan 16 17:13:42 PST 1987 
 *		created for use by tape utilities doing
 *		ioctl calls to the tape driver.
 *
 * Modification History:
 *	M001:	uport!doug Thu Jan 22 20:25:07 PST 1987
 *		put debug defines in debug.h to preserve modification
 *		time of this file.  Move altbuf define to ct.h.
 *
 * To do:
 *		1. Test ioctl interface in tape utilities.
 */

/* ioctl functions
  
   TVI TeleCAT-286 Streaming Tape Driver ioctl interface
  

	DEBUG_LVL:	set the debug level for debug printfs
			input: 	 flag = new debug level (int)
				(see ctioctl.h for debug levels)
			returns: none

   The following functions all assume the ioctl arg (bp) to  be a 
   pointer to a buf structure setup by the user process as if he 
   were calling ctstrategy.


	CT_LOCK:	lock the user process in memory
			input: 	 bp-> buf structure
			returns: u.u_error set if process was not locked

	CT_READ:	read b_bcount blocks (clicks) into b_bun.b_addr
			input: 	 bp->b_un.b_addr
				 bp->b_bcount (clicks)
			returns: bp->b_resid (residual click count)
				 u.u_error set if error occurred

	CT_WRITE:	write b_bcount clicks from b_bun.b_addr
			input: 	 bp->b_un.b_addr
				 bp->b_bcount (clicks count)
			returns: bp->b_resid (residual clicks)
				 u.u_error set if error occurred
*/

/* ioctl command defines */
#define DEBUG_LVL		1
#define	CT_LOCK			(('T'<<8)|0)
#define	CT_READ			(('T'<<8)|1)
#define	CT_WRITE		(('T'<<8)|2)
