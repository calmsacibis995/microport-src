	.file	"cerror.s"
/	@(#)	1.1
	.text
/
/ C return sequence which sets errno, returns -1.

#include "lib.s.h"
	.globl	_cerror
	.globl	errno

_cerror:
#if LARGE_M | HUGE_M | COMPACT_M
	mov	$<s>errno,%dx
	mov	%dx,%ds
#endif
	mov	%ax,errno
	mov	$-1,%ax
	cwd
	LVRET

	.data
errno:	.value	0
