#ident	"@(#)unixsyms.c	1.1"

#include <stdio.h>
#include <fcntl.h>
#include <filehdr.h>
#include <syms.h>
#include <storclass.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include "unixsyms.h"


char *ldgetname();
int cmp();

char *namep = namepool;		/* current pointer into namepool */
int symindex;			/* index into symtable */
SCNHDR sections[20];		/* for section headers */

main(argc, argv)
int argc;
char **argv;
{
    LDFILE *ldptr = NULL;           /* for use by ld* routines */
    SYMENT sym;                     /* to read symtab entries into */
    long patchloc = 0;              /* location to patch output into */
    int patchlen;                   /* length of output */
    int slen;
    int fp;
    int i;
    char *name;

    if (argc != 2)
	    fatal("usage:  %s COFF-file\n", argv[0]);

    if ((ldptr = ldopen(argv[1], ldptr)) == NULL)
	    fatal("cannot open %s\n", argv[1]);

    if (!ISCOFF(HEADER(ldptr).f_magic))
	    fatal("%s is not a COFF object file\n", argv[1]);

    /* for each symbol in input file... */

    for (i = 0; i < HEADER(ldptr).f_nsyms; i++)
    {
	/* read the symbol into sym */

	if (ldtbread(ldptr, (long)i, &sym) == FAILURE)
		fatal("ldtbread() failed\n");

	/* make sure to skip following aux entries */

	i += sym.n_numaux;

	/* we only care about it if it is external */

	if (sym.n_sclass == C_EXT)
	{
	    name = ldgetname(ldptr, &sym);

	    /* read in section info if not already read in */

	    if (sections[sym.n_scnum].s_scnptr == 0)
		    ldshread(ldptr, sym.n_scnum,
				    &sections[sym.n_scnum]);

	    /* if it's the magic symbol, remember patch location */

	    if (!strcmp(name, MAGICSYM))
		    patchloc = sections[sym.n_scnum].s_scnptr +
			    (sym.n_value -
				    sections[sym.n_scnum].s_paddr);

	    /* skip symbols that start with '.' */
	    if (name[0] == '.')
		    continue;

	    /* make a symtable entry for the symbol */

	    if (symindex == MAXSYMS)
		    fatal("too many externs in %s\n", argv[1]);
	    symtable[symindex].value = sym.n_value;
	    symtable[symindex++].nameoffset = namep - namepool;

	    /* copy the symbol's name into the name pool */

	    strcpy(namep, name);
	    namep += strlen(name) + 1;
	}
    }
    ldclose(ldptr);

    /* sort symtable based on value */

    qsort((char *)symtable, symindex, sizeof(struct symbols), cmp);

    /* write out value/name pairs at patchloc */

    if (!patchloc)
	    fatal("no symbol named '%s' found in %s\n", MAGICSYM, argv[1]);

    if ((fp = open(argv[1], O_WRONLY)) == -1)
	    fatal("cannot open %s for writing\n", argv[1]);

    if (lseek(fp, patchloc, 0) == -1)
	    fatal("lseek() failed\n");

    for (i = 0, patchlen = 0; i < symindex; i++)
    {
	    slen = strlen(symtable[i].nameoffset + namepool) + 1;
	    patchlen += (slen + sizeof(long));
	    if (patchlen > PATCHSIZE)
		    fatal("symbol table too long\n");
	    write(fp, &(symtable[i].value), sizeof(long));
	    write(fp, symtable[i].nameoffset + namepool, slen);
    }
    fprintf(stderr, "%d symbols, table length = %d bytes (decimal)\n",
	    symindex, patchlen);
    close(fp);
}

fatal(format, arg1, arg2, arg3)
char *format;
char *arg1, *arg2, *arg3;
{
	fprintf(stderr, format, arg1, arg2, arg3);
	exit(1);
}

cmp(a, b)
struct symbols *a, *b;
{
	return(a->value > b->value ? 1 : -1);
}
