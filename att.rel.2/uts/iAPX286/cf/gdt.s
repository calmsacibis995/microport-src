	.file	"gdt.s"
/ @(#)gdt.s	1.41
/
/			NOTE
/			----
/ This module must be linked in first in the kernel to
/ guarantee proper placement of the 215 wakeup block and
/ GDT in kernel data.
/

#include	"sys/mmu.h"
#include	"config.h"

	.data
/
/	wake-up block for 215 disk controller
/
	.globl	wub
wub:
	.=.+WUBSIZ

/
/	The Global Descriptor Table. All global data is defined here
/
/	format:
/		.value	limit
/		.value	lowbase		( bits 0-15 of base of segment )
/		.byte	hibase		( bits 16-23 of base of segment )
/		.byte	access
/		.value	reserved
/
	.globl	gdt_base
	.globl	gdt
gdt_base:
gdt:
	.value	0 			/ 0	first entry must be null
	.value	0
	.byte	0
	.byte	0
	.value	0

	. = .+[DSC_SZ\*39]		/ 1-39	reserved for monitor
	. = .+[DSC_SZ\*22]		/ 40-61	reserved for expansion

	.value	0			/ 62	/dev/mem descriptor
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.globl	wntd
wntd:
	.value	43			/ 63	wndump tss descriptor
	.value	0
	.byte	0
	.byte	[DSC_PRESENT | CD_AVAIL_TSK]
	.value	0

	.value	0			/ 64	kernel code (filled in by boot)
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 65	kernel code (filled in by boot)
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 66	kernel code (filled in by boot)
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 67	kernel code (filled in by boot)
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0 			/ 68	kernel data (filled in by boot)
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 69	kernel data (filled in by boot)
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 70	Kernel Data
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 71	Kernel Data
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 72	Kernel Data
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	0			/ 73	Kernel Data
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	. = .+[DSC_SZ\*5]		/ 74	fpbyte/word spbyte/word sel
					/ 75	swtch selector
					/ 76-77	copyseg selectors
					/ 78	clearseg selector

	.value	0xFFFF			/ 79	544 scratch selector
	.value	0x0000
	.byte	0xFE
	.byte	ACC_KDATA
	.value	0

	. = .+[DSC_SZ]			/ 80	swap selector

	.value	sigcode			/ 81	Sigcode entry point
	.value	<s>sigcode
	.byte	16
	.byte	[DSC_PRESENT | USR_DPL | G_CALL]
	.value	0

	. = .+[DSC_SZ\*2]		/ 82	fpsave selector
					/ 83 	message selector

	.globl	tftd
tftd:
	.value	43			/ 84	Tss Fault Task Descriptor
	.value	0
	.byte	0
	.byte	[DSC_PRESENT | CD_AVAIL_TSK]
	.value	0

	.globl	dftd
dftd:
	.value	43			/ 85	Double Fault Task Descriptor
	.value	0
	.byte	0
	.byte	[DSC_PRESENT | CD_AVAIL_TSK]
	.value	0

	.globl	gdtalias
gdtalias:
	.value	0			/ 86	GDT Alias ( limit filled in
					/                   start.s )
	.value	[WUBSIZ+PHYS_KDATA]	/ wake-up blk is first WUBSIZ bytes
	.byte	0
	.byte	ACC_KDATA
	.value	0
	
	.globl	idt
idt:
	.value	2047			/ 87	IDT Alias
	.value	0			/ filled in by start.s
	.byte	0
	.byte	ACC_KDATA
	.value	0
	
	.value	0			/ 88	Custom System Call
	.value	0
	.byte	0
	.byte	0
	.value	0
	
	.value	systemc			/ 89	System Call entry point
	.value	<s>systemc
	.byte	10
	.byte	[DSC_PRESENT | USR_DPL | G_CALL]
	.value	0
/
/ the following are process slot packets.
/ each packet is four descriptors:
/	TSS descriptor
/	TSS Alias descriptor
/	LDT descriptor
/	LDT Alias descriptor
/
	.globl	pslot
	.globl	p0tsssel
	.globl	p0ldtsel
pslot:
p0tsssel:				/ proc 0 tss descriptor
	.value	43
	.value	0
	.byte	0
	.byte	[DSC_PRESENT | CD_AVAIL_TSK]
	.value	0
	
	.value	43			/ proc 0 tss alias descriptor
	.value	0
	.byte	0
	.byte	ACC_KDATA
	.value	0
	
p0ldtsel:				/ proc 0 ldt descriptor
	.value	511 
	.value	0
	.byte	0
	.byte	[DSC_PRESENT | CD_LDT]
	.value	0

	.value	511			/ proc 0 ldt alias descriptor
	.value	0
	.byte	0
	.byte	ACC_KDATA
	.value	0
/
/	declare the rest of the gdt as process slots.
/	4 descriptors per process
/
	. = . + [ [ DSC_SZ \* 4 ] \* [ NPROC - 1 ] ]

	.globl	gdt_end
gdt_end:	.value	0

/
/	Process 0 user structure (includes kernel stack and TSS)
/
/	This next totally unreadable assignment to "." makes
/	sure that the proc0 u-structure is on a 512 byte boundary
/	to make copyseg happy
/
sludge:	. = . + [ 512 - [ [ sludge - wub ] \% 512 ] ]

	.globl	p0u
	.globl	p0tss
	.globl	p0ldt
	.globl	p0usel
	.globl	kstack
p0u:
	.=.+1024
kstack:					/ kernel stack grows down (towards p0u)
p0tss:					/ proc 0 TSS structure
	.value	0			/ back TSS selector
	.value	1024			/ sp for privilege level 0
	.value	[[UPAGE_SEL << 3] | LDT_TI]	/ ss for privilege level 0
	.value	0			/ sp for privilege level 1
	.value	0			/ ss for privilege level 1
	.value	0			/ sp for privilege level 2
	.value	0			/ ss for privilege level 2
	.=.+28				/ saved registers
					/ don't need to be initialized
	.value	[[PSLOT_SEL + 2] << 3]	/ LDT selector
	.=.+[1024 - 44]
/
/	Process 0 LDT (must immediately follow the user structure)
/
p0ldt:
	.=.+64				/ descriptors 0-7 are null
p0usel:					/ proc 0 upage descriptor
	.value	2047
	.value	0
	.byte	0
	.byte	ACC_KDATA
	.value	0
	.=.+[512 - 72]			/ descriptors 9-n are null


/
/ The actual definition of the user structure's location
/
	.globl	u
	.set	u,0x440000

/
/ selector address to access the gdt
/
	.globl	gdtptr
gdtptr:
	.value	0
	.value	[GDT_SEL<<3]

/
/ selector address to access the gdt
/
	.globl	idtptr
idtptr:
	.value	0
	.value	[IDT_SEL<<3]

/
/ selector address to use for scratch segment ( used for sp* and fp* )
/
	.globl	scratchptr
scratchptr:
	.value	0
	.value	[SCRATCH_SEL<<3]

/
/ selector addresses to use for scratch segment ( used for copyseg )
/
	.globl	copy0ptr
	.globl	copy1ptr
copy0ptr:
	.value	0
	.value	[COPY0SEL<<3]
copy1ptr:
	.value	0
	.value	[COPY1SEL<<3]

/
/ selector addresses to use for scratch segment ( used for clearseg )
/
	.globl	clsegptr
clsegptr:
	.value	0
	.value	[CLSEGSEL<<3]

/
/ Tss fault stack
/
tfsbot:
	. = . + 1024
tfstack:
	
/
/ Tss structures for tss fault and double fault tasks
/
	.globl	tftss
tftss:
	.value	0			/ back TSS selector
	.value	tfstack			/ sp for privilege level 0
	.value	<s>tfstack		/ ss for privilege level 0
	.value	0			/ sp for privilege level 1
	.value	0			/ ss for privilege level 1
	.value	0			/ sp for privilege level 2
	.value	0			/ ss for privilege level 2
	.value	tssfault		/ ip
	.value	0			/ flags
	.value	0			/ ax
	.value	0			/ cx
	.value	0			/ dx
	.value	0			/ bx
	.value	tfstack			/ sp
	.value	0			/ bp
	.value	0			/ si
	.value	0			/ di
	.value	0			/ es
	.value	<s>tssfault		/ cs
	.value	<s>tfstack		/ ss
	.value	0			/ ds
	.value	0			/ LDT selector

	.globl	dftss
dftss:
	.value	0			/ back TSS selector
	.value	kstack			/ sp for privilege level 0
	.value	<s>kstack		/ ss for privilege level 0
	.value	0			/ sp for privilege level 1
	.value	0			/ ss for privilege level 1
	.value	0			/ sp for privilege level 2
	.value	0			/ ss for privilege level 2
	.value	dblfault		/ ip
	.value	0			/ flags
	.value	0			/ ax
	.value	0			/ cx
	.value	0			/ dx
	.value	0			/ bx
	.value	kstack			/ sp
	.value	0			/ bp
	.value	0			/ si
	.value	0			/ di
	.value	0			/ es
	.value	<s>dblfault		/ cs
	.value	<s>kstack		/ ss
	.value	0			/ ds
	.value	0			/ LDT selector

/
/ wndump task stack
/
wnsbot:
	. = . + 1024
wnstack:
	
/
/ Tss structures for wndump task
/
	.globl	wntss
wntss:
	.value	0			/ back TSS selector
	.value	wnstack			/ sp for privilege level 0
	.value	<s>wnstack		/ ss for privilege level 0
	.value	0			/ sp for privilege level 1
	.value	0			/ ss for privilege level 1
	.value	0			/ sp for privilege level 2
	.value	0			/ ss for privilege level 2
	.value	wndump			/ ip
	.value	0			/ flags
	.value	0			/ ax
	.value	0			/ cx
	.value	0			/ dx
	.value	0			/ bx
	.value	wnstack			/ sp
	.value	0			/ bp
	.value	0			/ si
	.value	0			/ di
	.value	0			/ es
	.value	<s>wndump		/ cs
	.value	<s>wnstack		/ ss
	.value	0			/ ds
	.value	0			/ LDT selector
