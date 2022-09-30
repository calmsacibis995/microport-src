#ident	"@(#)unixsyms.h	1.1"

#define MAXSYMS (0xF000/sizeof(struct symbols)) /* max # of external syms */
#define	MAGICSYM "symtable"		/* name of symbol in kernel where
					   output will be written */
#define PATCHSIZE 20000                 /* size of patch area in kernel */

struct symbols
{
	short nameoffset;	/* offset into namepool */
	long value;		/* symbol value (n_value) */
} ;


extern struct symbols symtable[];       /* an entry per symbol in input */
extern char namepool[];                 /* to hold all symbol names */
