/* @(#)kd_mode.h	1.2 Microport 9/23/87 */

/* these relate to internal modes */
/* #define	EGA_NUMMODES	27 */
#define	EGA_MONOMODE	7
#define	EGA_NUMSEQREGS	0x4
#define	EGA_NUMCTLREGS	0x19
#define	EGA_NUMATRBREGS	0x14
#define	EGA_NUMGFXREGS	0x9

/* #define	CGA_NUMMODES	8 */
#define	CGA_NUMCTLREGS	0x10

/* structure of the BIOS initialize data */
/* note: can't use arrays because 'c' aligns start of each on a word boundary */
struct biosinitdata {
    unsigned char cols, rows, pels;
    unsigned int  pglen;
    unsigned char s0,s1,s2,s3;
    unsigned char misc;
    unsigned char c0,c1,c2,c3,c4,c5,c6,c7,c8,c9,ca,cb,cc,cd,ce,cf;
    unsigned char c10,c11,c12,c13,c14,c15,c16,c17,c18;
    unsigned char a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,aa,ab,ac,ad,ae,af;
    unsigned char a10,a11,a12,a13;
    unsigned char g0,g1,g2,g3,g4,g5,g6,g7,g8;
};

/* external data references defined in kd_mode.c */
extern struct biosinitdata *kd_egamodes [];
extern unsigned char *cga_crtc [];
extern unsigned char herc_crtc [];
extern int cga_mode [];
extern int sw_mode [];
extern int cga_colr [];
extern int kd_numegamodes;	/* includes hercmode */
extern int kd_numcgamodes;	/* includes hercmode */
extern int kd_egahercmode;
extern int kd_cgahercmode;
extern int kd_extm;	/* offset from basic to extended memory */
extern int kd_ecd;	/* offset from basic to enhanced color display */

/* === */
