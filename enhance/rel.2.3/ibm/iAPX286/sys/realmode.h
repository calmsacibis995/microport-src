/* DO NOT MODIFY THIS FILE.  THIS IS realmode.h WHICH IS MADE FROM osmerge.h */

/* # SCCSID(%W%	LCC);	/* Modified: %U% %G%*/

/* 	@(#)	1.8	*/
/* atmerge.h and realmode.h                                                */
/*                                                                         */
/* Since this file is automagically converted from atmerge.h to realmode.h */
/* please be sure that any changes you make are translatable by the simple */
/* sed script used, or make the sed script intelligent enough to handle    */
/* your change.                               				  */
/*                                                                         */
/* Rules:                                                                  */
/*      Carriage returns and the CTRL-Z are stripped.                      */
/*      Lines beginning with ";;; " have the ";;; " stripped off.          */
/*      Comments change from ";" type to "/ * . . . * /" type.             */
/*      "EQU" lines change to "#define" lines.                             */
/*      "OR" operators change to "|" operators.                            */
/*      Numbers like "0ABCDH" change to "0x0ABCD".                         */
/*                                                                         */
/* Note again that this script is extremely simple-minded and easily       */
/* broken.                                                                 */
/*                                                                         */

#ifdef MASM_ASSEMBLER
/* Switchdata structure                                                 */
sw_data         STRUC
sw_sbufaddr     dw      0,0     /* send buffer address              */
sw_rbufaddr     dw      0,0     /* receive buffer address           */
sw_altscreen    dw      0,0     /* location of alternate screen     */
sw_gdtcopy      dw      0,0,0,0 /* copy of gdt alias from gdt       */
sw_idtcopy      dw      0,0,0,0 /* copy of idt alias from gdt       */
sw_unxaddr      dw      0       /* address of unix switch ret code  */
sw_unxsel       dw      0       /* selector of unix switch ret code */
sw_memsize      dw      0       /* size of DOS memory               */
sw_devassign    dw      0       /* assign bit mask; 1=unix, 0=dos   */
sw_devinuse     dw      0       /* current Unix devices in use      */
sw_intvecnum    dw      0       /* int number across switch         */
sw_recontrol    db      0       /* flag to recompute ctrl settings  */
sw_curscreen    db      0       /* current screen assignment        */
sw_swscrchar    dw      0       /* switchscreen character           */
sw_swmodchar    dw      0       /* switchmode character             */
sw_kbstate      db      0       /* copy of keyboard state           */
sw_reserved     db      0       /* reserved                         */
sw_dosip        dw      0       /* DOS ip register for profiling    */
sw_doscs        dw      0       /* DOS cs register for profiling    */
sw_isr		dw	0	/* in-service record of 8259	   */
sw_intmask	dw	0	/* 8259 mask save during reset	   */
sw_dosticks	dw	0	/* DOS timer ticks missed	   */
sw_hotkeyhit	dw	0	/* scan code put here when hk hit   */
sw_modkeyhit    dw      0       /* scan code for mode switch key    */
sw_lastprimary	db	0	/* last primary interrupt	   */
sw_data         ENDS
#endif /* MASM_ASSEMBLER */

#ifndef UNIX_ASSEMBLER

struct sw_data {
        int sw_sbufaddr[2];
        int sw_rbufaddr[2];
        int sw_altscreen[2];
        int sw_gdtcopy[4];
        int sw_idtcopy[4];
        int sw_unxaddr;
        int sw_unxsel;
        int sw_memsize;
        int sw_devassign;
        int sw_devinuse;
        int sw_intvecnum;
        char sw_recontrol;
        char sw_curscreen;
        int sw_swscrchar;
        int sw_swmodchar;
        char sw_kbstate;
        char sw_reserved;
        int sw_dosip;
        int sw_doscs;
    int sw_isr;
    int sw_intmask;
    int sw_dosticks;
    int sw_hotkeyhit;
    int sw_modkeyhit;
    char sw_lastprimary;
};
#endif /* -UNIX_ASSEMBLER */

/* Flag codes in sw_flag                                            */
#define MEM_SEND         0           /* bridge did a memory network send */
#define IOCTLOUT         1           /* user program did an ioctl out    */
#define TRYBOOT          2           /* user program jumped to bootstrap */
#define WENTINSANE	 3

#define DEVN_COM0	 4		/* Communication port 0		   */
#define DEVN_COM1        3           /* Communication port 1             */
#define DEVN_FLOPPY      6           /* Floppy                           */
#define DEVN_PRINTER     7           /* Printer                          */

#define NDEVS            16          /* Number of devices                */

#define DEV_ALLUNIX      0x0FFFF      /* Mask to assign all devs to Unix  */
#define DEV_UNIX         0x4100       /* Devices which must be Unix       */
#define DEV_NOASSIGN     0x6104       /* not assignable        	   */

/* Machine Status Word                                              */
#define MSW_PE           0x0001       /* MSW protection enable bit        */

#ifdef MASM_ASSEMBLER
/* Macro for flushing the 286 instruction queue			   */
flush	MACRO
	db	0ebH, 0
ENDM
#endif MASM_ASSEMBLER

/********************** Screen Control  *********************************/

#define UNIXMODE         0          /* UNIX screen is currently displayed*/
#define DOSMODE          1          /* DOS screen is currently displayed */

