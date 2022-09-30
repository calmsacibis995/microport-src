        .file   "gdt.s"
/ @(#)gdt.s     1.41
/*		Copyright 1985 Microport Systems
 *		All Rights Reserved.
 *
 *	Modification History:
 * M002:	uport!dwight	 Wed Oct  2 10:11:29 PDT 1985
 *	Removed the descriptor for the 544. Replaced it with the
 *	descriptor for the video monitor.
 * M003:    lance			 Sat. Feb. 15, 1986
 *  Added descriptor 90 to access A0000-AFFFF for NCR color card
 *  and IBM EGA card.
 * M004:	lance			 Sat. Feb. 15, 1986
 *  Added selectors after pslot for init-time allocation.
 *  This should be added to config.
 * M005:	uport!dwight	Thu Feb 27 11:29:51 PST 1986
 *  Changed M003. Video descriptor 90 now resides in descriptor 73.
 *  This used to be reserved for kernel data, but was never used.
 *  The reason for this change is due to kernel boot problems upon startup,
 *  when descriptor 90 was changed.
 */

/
/                       NOTE
/                       ----
/ This module must be linked in first in the kernel to
/ guarantee proper placement of the 215 wakeup block and
/ GDT in kernel data.
/

#include 	"sys/param.h"
#include        "sys/mmu.h"
#include        "config.h"


        .data
/
/       wake-up block for 215 disk controller
/
        .globl  wub
wub:
        .=.+WUBSIZ

/
/       The Global Descriptor Table. All global data is defined here
/
/       format:
/               .value  limit
/               .value  lowbase         ( bits 0-15 of base of segment )
/               .byte   hibase          ( bits 16-23 of base of segment )
/               .byte   access
/               .value  reserved
/
        .globl  gdt_base
        .globl  gdt
gdt_base:
gdt:
        .value  0                       / 0     first entry must be null
        .value  0
        .byte   0
        .byte   0
        .value  0

        . = .+[DSC_SZ\*39]              / 1-39  reserved for monitor
#ifdef ATMERGE
	. = .+[DSC_SZ\*4]		/ 40-43 reserved for expansion

	.value	0x1fff			/ 44	Atron Probe code descriptor
	.value	0
	.byte	0
	.byte	0x9a
	.value	0

	.value	0			/ 45	alternate screen descriptor
	.value	0
	.byte	0
	.byte	0
	.value	0

	.value	0xffff			/ 46	LOW_SEL (0) descriptor
	.value	0
	.byte	0
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 47	LOW_SEL (1) descriptor
	.value	0
	.byte	1
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 48	LOW_SEL (2) descriptor
	.value	0
	.byte	2
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 49	LOW_SEL (3) descriptor
	.value	0
	.byte	3
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 50	LOW_SEL (4) descriptor
	.value	0
	.byte	4
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 51	LOW_SEL (5) descriptor
	.value	0
	.byte	5
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 52	LOW_SEL (6) descriptor
	.value	0
	.byte	6
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 53	LOW_SEL (7) descriptor
	.value	0
	.byte	7
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 54	LOW_SEL (8) descriptor
	.value	0
	.byte	8
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 55	LOW_SEL (9) descriptor
	.value	0
	.byte	9
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 56	LOW_SEL (10) descriptor
	.value	0
	.byte	10
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 57	LOW_SEL (11) descriptor
	.value	0
	.byte	11
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 58	LOW_SEL (12) descriptor
	.value	0
	.byte	12
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 59	LOW_SEL (13) descriptor
	.value	0
	.byte	13
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 60	LOW_SEL (14) descriptor
	.value	0
	.byte	14
        .byte   ACC_KDATA
	.value	0

	.value	0xffff			/ 61	LOW_SEL (15) descriptor
	.value	0
	.byte	15
        .byte   ACC_KDATA
	.value	0

#else  /* -ATMERGE */
        . = .+[DSC_SZ\*22]              / 40-61 reserved for expansion
#endif /* ATMERGE */

        .value  0                       / 62    /dev/mem descriptor
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .globl  wntd
wntd:
        .value  43                      / 63    hddump tss descriptor
        .value  0
        .byte   0
        .byte   [DSC_PRESENT | CD_AVAIL_TSK]
        .value  0

        .value  0                       / 64    kernel code (filled in by boot)
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 65    kernel code (filled in by boot)
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 66    kernel code (filled in by boot)
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 67    kernel code (filled in by boot)
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 68    kernel data (filled in by boot)
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 69    kernel data (filled in by boot)
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 70    Kernel Data
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 71    Kernel Data
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  0                       / 72    Kernel Data
        .value  0
        .byte   0
        .byte   0
        .value  0

/ M005: Segment Descriptor #73 - NCR Monitor (B/W & Color)
/ For the AT, the area reserved for the display buffers begins
/ at address 0A0000 and extends to 0BFFFF.  This gives access to the
/ second page of graphics memory, from A0000-AFFFF.
/ Set up the gdt as starting at base=A0000, limit=FFFFh (64k).
/ This segment used to be for kernel data, but was never used.
        .value  0xFFFF                  / 73    Video RAM segment limit
        .value  0x0000			/	2nd Video RAM base (low)
        .byte   0x0A			/	2nd Video RAM base (high)
        .byte   ACC_KDATA
        .value  0

        . = .+[DSC_SZ\*5]               / 74    fpbyte/word spbyte/word sel
                                        / 75    swtch selector
                                        / 76-77 copyseg selectors
                                        / 78    clearseg selector

/ M002: Segment Descriptor #79 - Video Monitor (B/W & Color)
/ For the AT, the area reserved for the display buffers begins
/ at address 0A0000 and extends to 0BFFFF.  We need only one
/ gdt descriptor, and the physical boards respond to start addresses
/  at 0B0000, so we set up the gdt as starting at base=B0000, limit=FFFFh (64k).
/ M003: Not any more. The NCR color card and the IBM EGA have memory in 
/ the A0000 bank, so descriptor 73 gives access to that area. lcn 2-15-86

        .value  0xFFFF                  / 79    Video RAM segment limit
        .value  0x0000			/	Video RAM base (low)
        .byte   0x0B			/	Video RAM base (high)
        .byte   ACC_KDATA
        .value  0

        . = .+[DSC_SZ]                  / 80    swap selector

        .value  sigcode                 / 81    Sigcode entry point
        .value  <s>sigcode
        .byte   16
        .byte   [DSC_PRESENT | USR_DPL | G_CALL]
        .value  0

        . = .+[DSC_SZ\*2]               / 82    fpsave selector
                                        / 83    message selector

        .globl  tftd
tftd:
        .value  43                      / 84    Tss Fault Task Descriptor
        .value  0
        .byte   0
        .byte   [DSC_PRESENT | CD_AVAIL_TSK]
        .value  0

        .globl  dftd
dftd:
        .value  43                      / 85    Double Fault Task Descriptor
        .value  0
        .byte   0
        .byte   [DSC_PRESENT | CD_AVAIL_TSK]
        .value  0

        .globl  gdtalias
gdtalias:
        .value  0                       / 86    GDT Alias ( limit filled in
                                        /                   start.s )
        .value  [WUBSIZ+PHYS_KDATA]     / wake-up blk is first WUBSIZ bytes
        .byte   0
        .byte   ACC_KDATA
        .value  0
        
        .globl  idt
idt:
        .value  2047                    / 87    IDT Alias
        .value  0                       / filled in by start.s
        .byte   0
        .byte   ACC_KDATA
        .value  0
        
        .value  0                       / 88    Custom System Call
        .value  0
        .byte   0
        .byte   0
        .value  0
        
        .value  systemc                 / 89    System Call entry point
        .value  <s>systemc
        .byte   10
        .byte   [DSC_PRESENT | USR_DPL | G_CALL]
        .value  0

/
/ the following are process slot packets.
/ each packet is four descriptors:
/       TSS descriptor
/       TSS Alias descriptor
/       LDT descriptor
/       LDT Alias descriptor
/
        .globl  pslot
        .globl  p0tsssel
        .globl  p0ldtsel
pslot:
p0tsssel:                               / proc 0 tss descriptor
        .value  43
        .value  0
        .byte   0
        .byte   [DSC_PRESENT | CD_AVAIL_TSK]
        .value  0
        
        .value  43                      / proc 0 tss alias descriptor
        .value  0
        .byte   0
        .byte   ACC_KDATA
        .value  0
        
p0ldtsel:                               / proc 0 ldt descriptor
        .value  511 
        .value  0
        .byte   0
        .byte   [DSC_PRESENT | CD_LDT]
        .value  0

        .value  511                     / proc 0 ldt alias descriptor
        .value  0
        .byte   0
        .byte   ACC_KDATA
        .value  0

/
/       declare the rest of the gdt as process slots.
/       4 descriptors per process
/
        . = . + [ [ DSC_SZ \* 4 ] \* [ NPROC - 1 ] ]

/ M004: Segment Descriptors for valloc() at end of gdt.
        		                        / ?    Selectors for valloc
		.globl	vall_sel
		.globl	vall_selend
/* Add this to config */
#define	NVSEL	40
vall_sel:
        . = . + [ DSC_SZ \* NVSEL ]
vall_selend:

/* I/O Buffer Selectors */
/* Add this to config */
#define	NIOBUFSEL	40
	.globl	io_bufsel
io_bufsel:
        . = . + [ DSC_SZ \* NIOBUFSEL ]
	.globl	io_bufselend
io_bufselend:

        .globl  gdt_end
gdt_end:        .value  0

#ifdef ATPROBE
	.globl	probethere
probethere: .value	0
#endif ATPROBE

#ifdef ATMERGE
	.globl	msm_ok
	.globl	fulpage
	.globl	lulpage
	.globl	fuepage
	.globl	aetext
	.globl	reloclist
msm_ok:		.value	0
fulpage:	.value	0
lulpage:	.value	0
fuepage:	.value	0
aetext:		.value	0, 0		/ will be physical address of etext

/ list of selector indices that have to have their bases adjusted when
/ the kernel is moved to extended memory
reloclist:	.byte	63, 64, 65, 66, 67, 68, 69, 70, 71, 72
		.byte	84, 85, 86, 87, 90, 91, 92, 93, 0
	/ and, the p0usel descriptor, which unfortunately has to be relocated
	/ as a special case, since it is not in the gdt. 
#endif /* ATMERGE */

/
/       Process 0 user structure (includes kernel stack and TSS)
/
/       This next totally unreadable assignment to "." makes
/       sure that the proc0 u-structure is on a 512 byte boundary
/       to make copyseg happy
/
#ifdef LCCFIX
sludge: . = . + [[ 512 - [ [ sludge - wub ] \% 512 ] ] \% 512]
#else /* -LCCFIX */
sludge: . = . + [ 512 - [ [ sludge - wub ] \% 512 ] ]
#endif /* LCCFIX */

        .globl  p0u
        .globl  p0tss
        .globl  p0ldt
        .globl  p0usel
        .globl  kstack
p0u:
        .=.+[KSTACKSZ \* 2]
kstack:                                 / kernel stack grows down (towards p0u)
p0tss:                                  / proc 0 TSS structure
        .value  0                       / back TSS selector
        .value  [KSTACKSZ \* 2]         / sp for privilege level 0
        .value  [[UPAGE_SEL << 3] | LDT_TI]     / ss for privilege level 0
        .value  0                       / sp for privilege level 1
        .value  0                       / ss for privilege level 1
        .value  0                       / sp for privilege level 2
        .value  0                       / ss for privilege level 2
        .=.+28                          / saved registers
                                        / don't need to be initialized
        .value  [[PSLOT_SEL + 2] << 3]  / LDT selector
        .=.+[ [USIZE \* NBPC] - [KSTACKSZ \* 2] - 44]
/
/       Process 0 LDT (must immediately follow the user structure)
/
p0ldt:
        .=.+64                          / descriptors 0-7 are null
p0usel:                                 / proc 0 upage descriptor
        .value  [ [ USIZE \* NBPC ] - 1]
        .value  0
        .byte   0
        .byte   ACC_KDATA
        .value  0
        .=.+[512 - 72]                  / descriptors 9-n are null


/
/ The actual definition of the user structure's location
/
        .globl  u
        .set    u,0x440000

/
/ selector address to access the gdt
/
        .globl  gdtptr
gdtptr:
        .value  0
        .value  [GDT_SEL<<3]

/
/ selector address to access the gdt
/
        .globl  idtptr
idtptr:
        .value  0
        .value  [IDT_SEL<<3]

/
/ selector address to use for scratch segment ( used for sp* and fp* )
/
        .globl  scratchptr
scratchptr:
        .value  0
        .value  [SCRATCH_SEL<<3]

/
/ selector addresses to use for scratch segment ( used for copyseg )
/
        .globl  copy0ptr
	.globl  copy1ptr
        .globl	wndmaptr
copy0ptr:
        .value  0
        .value  [COPY0SEL<<3]
copy1ptr:
        .value  0
        .value  [COPY1SEL<<3]
wndmaptr:
	.value	0
	.value	[WNDMASEL<<3]
/
/ selector addresses to use for scratch segment ( used for clearseg )
/
        .globl  clsegptr
clsegptr:
        .value  0
        .value  [CLSEGSEL<<3]

/
/ Tss fault stack
/
tfsbot:
        . = . + [ KSTACKSZ \* 2]
tfstack:
        
/
/ Tss structures for tss fault and double fault tasks
/
        .globl  tftss
tftss:
        .value  0                       / back TSS selector
        .value  tfstack                 / sp for privilege level 0
        .value  <s>tfstack              / ss for privilege level 0
        .value  0                       / sp for privilege level 1
        .value  0                       / ss for privilege level 1
        .value  0                       / sp for privilege level 2
        .value  0                       / ss for privilege level 2
        .value  tssfault                / ip
        .value  0                       / flags
        .value  0                       / ax
        .value  0                       / cx
        .value  0                       / dx
        .value  0                       / bx
        .value  tfstack                 / sp
        .value  0                       / bp
        .value  0                       / si
        .value  0                       / di
        .value  0                       / es
        .value  <s>tssfault             / cs
        .value  <s>tfstack              / ss
        .value  0                       / ds
        .value  0                       / LDT selector

        .globl  dftss
dftss:
        .value  0                       / back TSS selector
        .value  kstack                  / sp for privilege level 0
        .value  <s>kstack               / ss for privilege level 0
        .value  0                       / sp for privilege level 1
        .value  0                       / ss for privilege level 1
        .value  0                       / sp for privilege level 2
        .value  0                       / ss for privilege level 2
        .value  dblfault                / ip
        .value  0                       / flags
        .value  0                       / ax
        .value  0                       / cx
        .value  0                       / dx
        .value  0                       / bx
        .value  kstack                  / sp
        .value  0                       / bp
        .value  0                       / si
        .value  0                       / di
        .value  0                       / es
        .value  <s>dblfault             / cs
        .value  <s>kstack               / ss
        .value  0                       / ds
        .value  0                       / LDT selector

/
/ hddump task stack
/
wnsbot:
        . = . + [ KSTACKSZ \* 2]
wnstack:
        
/
/ Tss structures for hddump task
/
        .globl  wntss
wntss:
        .value  0                       / back TSS selector
        .value  wnstack                 / sp for privilege level 0
        .value  <s>wnstack              / ss for privilege level 0
        .value  0                       / sp for privilege level 1
        .value  0                       / ss for privilege level 1
        .value  0                       / sp for privilege level 2
        .value  0                       / ss for privilege level 2
        .value  hddump                  / ip
        .value  0                       / flags
        .value  0                       / ax
        .value  0                       / cx
        .value  0                       / dx
        .value  0                       / bx
        .value  wnstack                 / sp
        .value  0                       / bp
        .value  0                       / si
        .value  0                       / di
        .value  0                       / es
        .value  <s>hddump               / cs
        .value  <s>wnstack              / ss
        .value  0                       / ds
        .value  0                       / LDT selector


