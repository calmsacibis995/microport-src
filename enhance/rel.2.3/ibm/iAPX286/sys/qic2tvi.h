/* uportid = "@(#)qic2tvi.h		Microport Rev Id  1.3.8 11/24/86" */
/* Copyright 1987 by Microport. All Rights Reserved.
 *
 * Televideo Cartridge Tape Specifics Header File
 *
 * TVI TeleCAT-286 Streaming Tape Device Driver 
 * References:
 *	TVI Streaming Tape Interface Hardware Specifications
 *
 * Initial Coding:
 *		unix-net!doug
 *		unix-net!mark Mon Nov 10 22:19:51 PST 1986
 *		this .h was created to separate h/w specific from
 *		generic function.
 * Modification History:
 * To do:
 *	1) try adapting driver to another tape controller to test
 *	   and improve separation of generic from hardware specific
 *	   portions.
 */


/* Function definitions  (required the definitions for the function array) */
#define MAXFUNCS 10
#define DEVRESET(p)     qc_reset(p)
#define DEVINIT(p)	qc_init(p)
#define DEVRDST(p)	qc_rnstb(p)
#define DEVCMD(p)	qc_cmd(p)


#define Q2BASE	0x208		                  /* DEFAULT 208 */
#define RD_STATUS	Q2BASE
#define RD_DR_STATUS	Q2BASE+3
#define WR_COMREQ	Q2BASE
#define RDSTM_ON	Q2BASE+1
#define WR_CNTL Q2BASE+2
#define INTACKTAPE	Q2BASE+3

/* use DMA Channel 5 (DMA #2 , channel 1) */
#define CTDMACH	5	 

/* controller commands */
#define QC_RESET	0	                 /* Reset the Drive */
#define CTRESET	QC_RESET	
#define QC_SD		1	                 /* Select Drive */
#define CTSD	QC_SD	
#define QC_RSTU		0xC0	                 /* Read Status */
#define CTRSTU	QC_RSTU	
#define QC_BOT		0x21	                 /* Rewind the Tape */
#define CTBOT	QC_BOT	
#define QC_ERA		0x22	                 /* ERASE the TAPE */
#define CTERA	QC_ERA	
#define QC_PRW		0x24	                 /* Prewind/Retension */
#define CTPRW	QC_PRW	
#define QC_WRT		0x40	                 /* Write to the tape */
#define CTWRT	QC_WRT	
#define QC_WFM		0x60	                          /* Write file mark */
#define CTWFM	QC_WFM	
#define QC_RD		0x80	                       /* Read from the tape */
#define CTRD	QC_RD	
#define QC_RFM		0xA0	                     /* Read to the filemark */
#define CTRFM	QC_RFM	
#define QC_ESTU		0xC8	                     /* read extended status */
#define CTESTU	QC_ESTU	
		       
/* tape write control bit definitions (bits 0-3) */
/* tape drive status bit definitions (bits 0-3) */
#define QC_ONLINE	0x01			        /* tape drive online */
#define QC_RST		0x02			              /* reset drive */
#define QC_RDSTAT	0x04			    /* request status active */
#define QC_IRQ		0x08			        /* interrupt request */
/* tape drive status bit definitions (bits 4-7) */
#define QC_DIR		0x80	                   /* 1 => info to drive     */
#define QC_OK		0x40	                   /* 1 => no error occurred */
#define QC_RDY		0x20                           /* 0 => tape is ready */
#define QC_ACK		0x10                      /* 0 => tape data was ready */

/* tape RSTU byte 0 status bit definitions (bits 0-7) */
#define QC_FILEMK	0x01			           /* tape file mark */
#define QC_NOBLOCK	0x02			        /* block not located */
#define QC_DATAERR	0x04			               /* data error */
#define QC_EOT		0x08			             /* end of media */
#define QC_WRPROT	0x10	                    /* write protected media */
#define QC_NOTSEL	0x20	                         /* unselected drive */
#define QC_NOTAPE	0x40                         /* no cassette in drive */
#define QC_ERROR	0x80                               /* error occurred */


/* number of status bytes to read */
#define QC_STATL	6
#define QC_EXSTATL	6

