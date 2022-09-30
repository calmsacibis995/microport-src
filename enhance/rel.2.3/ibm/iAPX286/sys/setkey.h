/* uportid = "@(#)setkey.h	Microport Rev Id  1.3.8 11/24/86" */
/* @(#)setkey.h	1.2 */
/*
 * This file describes the console driver setkey ioctl
 */

#define	FKEYLEN		32

struct setkey {
	char k_code;	    /* keyboard code number, see AT Tech Ref., 1-33 */
	char k_shift;	    /* require one of these shift keys, see below */
	char k_len;	    /* length of mapped data */
	char k_data[32];    /* mapped data */
};
#define	IOCSETKEY	('k'<<8 | 3)
#define	IOCGETKEY	('k'<<8 | 4)
#define	IOCCLRKEY	('k'<<8 | 5)

#define	NONE_SHIFTED	0
#define	CAP_SHIFTED	K_SHIFTTAB
#define	CTRL_SHIFTED	K_CTRLTAB
#define	ALT_SHIFTED	K_ALTTAB
