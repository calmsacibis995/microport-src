/* uportid = "@(#)tvi.h	Microport Rev Id  1.3.7 8/11/86 " */
/*  special definitions for TeleVideo hard disk controller */
/* Weaver 7/1/86 */

#define CONTROL TASKF+8
#define LED 0x80	/* LED off */
#define CPUC 0x20
#define IRQA 0x10
#define CBRDY 0x8
#define WGATE 0x4
#define RGATE 0x2
#define RESET 0x1

