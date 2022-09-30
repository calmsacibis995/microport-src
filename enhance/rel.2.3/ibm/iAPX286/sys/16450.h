/* uportid = "@(#)16450.h	Microport Rev Id  1.3.3 6/18/86" */
/*
** @(#)16450.h	1.0
** ** Definitions for the Parallel Serial Adapter card
** *  M000 lcn - Even parity and mark parity defines were reversed
** *  M000	   - manual refers to mark parity as 'stuck' parity. 
**			is this right?
** *  M001 lcn - add OUT1 and OUT2 output defines.  These gate interrupts.
** *  M001 	   - The documentation doesn't mention them, see the schematics.
** *  M002 lcn - Channel A & B ints were backwards
** *  M003 mac - changed INTDRV to accomodate AST board
*/

/*
** Port addresses
*/
#define	CHA_BASE	0x03F8		/* channel A data		*/
#define	CHB_BASE	0x02F8		/* channel B data		*/

#define	CHA_VEC		0x0C		/* channel A interrupt vector  M002 */
#define CHB_VEC		0x0B		/* channel B interrupt vector  M002 */

/*
** Interrupt Enable Register  --  Base reg + 1
*/
#define	ENB_RX_INT	0x01		/* enable receiver interrupt	*/
#define	ENB_TX_INT	0x02		/* enable xmiter interrupt	*/
#define	ENB_LS_INT  0x04		/* enable receiver status intrpt*/
#define	ENB_MS_INT	0x08		/* enable modem status interrupt*/
#define	ENB_ALL_INT	0x0F		/* enable intr on all chars, 	*/

/*
** Interrupt Identification Register  --  Base reg + 2
*/

#define	INT_PEND	0x01		/* Interrupt Pending bit   	*/
#define	MODM_STAT	0x00		/* Modem status change interrupt*/
#define	TX_EMPTY	0x02		/* Transmiter buffer empty	*/
#define	RX_RDY		0x04		/* Receiver ready interrupt     */
#define	LS_STAT		0x06		/* Receiver status change interrupt */
#define INTR_BITS	0x07		/* Mask for all the interrupt bits */

/*
** Line Control Register -- Base reg + 3              
*/

#define DL_5BIT		0x00		/* 5 data bits */
#define DL_6BIT		0x01		/* 6 data bits */
#define DL_7BIT		0x02		/* 7 data bits */
#define DL_8BIT		0x03		/* 8 data bits */
#define STOPBIT2	0x04		/* Number of Stop Bits */
#define ENB_PARITY	0x08		/* Parity enable bit location */
#define EVEN_PAR	0x10		/* Even / Odd parity select   M000 */
#define MARK_PAR  	0x20		/* Parity at Mark, not Space  M000 */
#define SET_BREAK	0x40		/* Break Control bit          */
#define DLAB		0x80		/* Divisor Latch Access bit   */


/*
** Modem Control Register -- Base reg + 4
*/
#define	DTR   		0x01		/* Data Terminal Rdy Control    */
#define	RTS         	0x02	 	/* request to send control      */
#define	OUT1		0x04		/* Output 1 of chip, gates ints	M001*/
#define	OUT2		0x08		/* Output 2 of chip, gates ints	M001*/

#define ASTVEC

#ifdef ASTVEC
#define	INTDRIV		mcr[unit]	/* AST Board value "common int" M003*/
#else /* NOT ASTVEC */
#define	INTDRIV		0x0c		/* Above 2 form hokey bus driver */
					/* for int M001 */
#endif /* ASTVEC */

/*
** Line Status Register -- Base reg + 5
*/
#define	DATA_RDY  	0x01		/* Data ready in buffer		*/
#define	OVERRUN_ERR	0x02		/* Overrun Error		*/
#define	PARITY_ERR	0x04		/* Parity Error    		*/
#define	FRAME_ERR	0x08		/* Framing Error		*/
#define	BREAKIN		0x10		/* Break received bit      	*/
#define	TXH_EMPTY	0x20		/* TX holding empty		*/
#define	TXS_EMPTY	0x40		/* TX shifter empty   		*/
#define LSR_INTS	0x1E		/* bits which cause interrupts */

/*
** Modem status register -- Base reg + 6
*/
#define	DELT_CTS 	0x01		/*                		*/
#define	DELT_DSR	0x02		/*           			*/
#define	TEDGE_RI	0x04		/*                    		*/
#define	DELT_DCD	0x08		/*               		*/
#define	CTS      	0x10		/* Clear to Send mask		*/
#define	DSR    		0x20		/* Data Set Ready Mask 		*/
#define	RI     		0x40		/* Ring Indicator mask 		*/
#define	DCD		0x80		/* Data Carrier Detect mask	*/
#define MSR_INTS	0x0F		/* bits which cause interrupts  */
