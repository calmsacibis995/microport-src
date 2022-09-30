static char *uportid = "@(#)patch.c	Microport Rev Id  1.3.2 6/10/86";

/*
 * Object file patcher.  Works on small & large model files.
 * Usage: patch [-k] file [+n] [-bilcs] symbol [datum [datum ...]]
 * 		-k pokes /dev/kmem, for use with a kernel file.
 *		+n starts n bytes after symbol
 *		-i means datum is an integer (default)
 *		-b means its a byte, -l long, -c character, -s string
 *		-s patches each datum argument and a null at end.
 *		successive data arguments are patched in sequence.
 *
 *	To compile: cc -O patch.c -o patch -lld
 *	First release:	March 22, 1986. by Lance Norskog
 *		(C) Microport Systems, Inc.
 *	To implement:
 *		Does not recognize symbols which are more than 8 chars long.
 *		Does not recognize escape sequences in char and string data.
 * M000:	Thu Jun 11 01:27:41 PDT 1987	uport!rex
 *		Modified to call ldgetname() for symbol names longer
 *		than 8 chars.
 */

#include <stdio.h>
#include <fcntl.h>

#include <ar.h>
#include <filehdr.h>
#include <ldfcn.h>
#include <scnhdr.h>
#include <syms.h>

SYMENT symbol;
SCNHDR secthead;

/* defines for datatype */
#define	BIN		1
#define	BYTE	2
#define	STRING	3

char *myname, *symfile, *searchsym;
int	symfd, dokmem = 0, datasize = 2, datatype = BIN;
LDFILE *symld;
long offset;

#define	debug if (0) printf

int	n;
char **args;

main(argv, argc)
int	argv;
char **argc;
{
	myname = args[0];
	n = argv - 1;
	args = &argc[1];
	doargs();

	ldclose(symld);
	close(symfd);
	exit(0);
}

char *usagemsg[] = {
"Usage: %s [-k] objectfile [-bilcs] [+offset] symbol [datum [datum ...]]\n", 
"Where -k patches /dev/kmem, \n",
"      -bilcs selects byte, int, long, char, or string data,\n",
"      and +offset patches offset bytes from the symbol\n",
0
};

usage() {
	int	i;

	for (i=0; usagemsg[i]; i++) 
		printf(usagemsg[i], myname);
	exit(1);
}

char *cptr, *objfile, *sym;

doargs() {
	int	i;

	if (n < 2)
		usage();
	if (!strcmp("-k", args[0])) {
		if (-1 == (symfd = open("/dev/kmem", O_RDWR))) {
			perror("Open /dev/kmem");
			error("%s: Cannot open /dev/kmem\n", myname);
			}
		dokmem = 1;
		n--, args++;
		if (n < 2)
			usage();
	} else
		dokmem = 0;

	/* get object file */
	objfile = args[0];
	if ((symld = ldopen(objfile, (LDFILE *) NULL)) == (LDFILE *) NULL) {
		error("%s: cannot open object file '%s'\n", myname, args[0]);
		}
	if (!dokmem && ((symfd = open(objfile, O_RDWR)) == -1)) {
		perror(objfile);
		error("%s: Cannot open %s\n", myname, objfile);
		}
	debug("File = '%s', TYPE = %o, Symld = %x, symfd = %d, dokmem = %d\n", 	
		objfile, TYPE(symld), symld, symfd, dokmem);

	/* do data size and offset options */
	for(i=0;i<2;i++) {
		if (args[1][0] == '-') {
			switch(args[1][1]) {
			case	'b': datasize = 1;  datatype = BIN;		break;
			case	'i': datasize = 2;  datatype = BIN;		break;
			case	'l': datasize = 4;  datatype = BIN;		break;
			case	'c': datasize = 1;  datatype = BYTE;	break;
			case	's': datasize = 1;  datatype = STRING;	break;
			default:
				usage();
			}
		} else if (args[1][0] == '+') {
			if (! scan_num(&args[1][1], &offset, 4)) 
				usage();
		} else break;
		args++, n--;
		if (n < 2)
			usage();
	}

	sym = args[1];
	debug("Searching for symbol '%s'\n", sym);
	if (!findsym(sym) || (symbol.n_scnum > HEADER(symld).f_nscns) || 	
			(symbol.n_scnum < 1)) 
		error("Symbol %s not found in file %s\n", sym, objfile);
	/* get section header for symbol */
	ldshread(symld, symbol.n_scnum, &secthead);
	if (n == 2)
		getdata();
	else
		putdata();
}

getdata() {
	unsigned long datum, peek();
	
	/* Check section header for size of data to be poked */
	debug("Datasize %x, value %lx, size %lx\n",
		datasize, symbol.n_value, secthead.s_size);
	if ((symbol.n_value - secthead.s_paddr) + datasize > secthead.s_size)
		error("%s: Too much data for segment\n", myname);
	debug("peeking datum\n"); 
	switch (datatype) {
		unsigned char c;

		case	BIN: 
			datum = peek(datasize);
			printf("0x%lx\n", datum);
			break;
		case	BYTE:
			datum = peek(datasize);
			c = datum;
			printf("%c\n", c);
			break;
		case 	STRING:
			/* peek all of string and newline at end */
			for(;;) {
				datum = peek(datasize);
				c = datum;
				if (c == 0)
					break;
				printf("%c", c);
				}
			printf("\n");
			break;
		default:
			error("How did we get here?");
		}
}

putdata() {
	int	i;
	unsigned long pokelen, datum;

	pokelen = 0;
	for(i=2; i<n; i++) {
		debug("Checking datum '%s'\n", args[i]); 
		if (datatype == STRING)
			pokelen += strlen(args[i]) + 1;	/* trailing 0 */
		else
			pokelen += datasize;
		/* check size problem here */
		if (datatype == BIN) {
			int size;
			if (!scan_num(args[i], &datum, &size))		
				error("%s: Datum not valid\n", myname);
			if (size > datasize)
				error("%s: Datum too large to fit\n", myname);
			}
	}
	/* Check section header for size of data to be poked */
	debug("Pokelen %lx, value %lx, size %lx\n",
		pokelen, symbol.n_value, secthead.s_size);
	if ((symbol.n_value - secthead.s_paddr) + pokelen > secthead.s_size)
		error("%s: Too much data for segment\n", myname);
	for(i=2; i<n; i++) {
		debug("Poking datum '%s'\n", args[i]); 
		switch (datatype) {
			int dummy;
			case	BIN: 
				scan_num(args[i], &datum, &dummy);
				poke(datum, datasize);
				break;
			case	BYTE:
				datum = args[i][0];
				datum &= 0xff;
				poke(datum, datasize);
				break;
			case 	STRING:
				/* poke all of string and 0 at end */
				cptr = args[i];
				while(*cptr) {
					scan_byte(&cptr, &datum);
					poke(datum, datasize);
					}
				datum = 0;
				poke(datum, datasize);
				break;
			default:
				error("How did we get here?");
			}
		}
}

findsym(sym) 
char *sym;
{
	int	nsyms, i, j;
	char	buf[100], *psymname, *ldgetname();	/* M000 */
	long	index;

	nsyms = HEADER(symld).f_nsyms;
	ldtbseek(symld);
	index = ldtbindex(symld);
	for(i = 0; i < nsyms; i++) {
		ldtbread(symld, index++, &symbol);
		if (symbol.n_numaux) {			/* skip over multi-entry symbols */
			i += symbol.n_numaux;
			index += symbol.n_numaux;
			continue;
		}
		if ((psymname=ldgetname(symld, &symbol)) != NULL) /* M000 */
			strcpy(buf, psymname);
		else {
			for(j=0; j< SYMNMLEN; j++)
				buf[j] = symbol.n_name[j];
			buf[j] = '\0';
		}
		if (!strcmp(buf, sym))
			return 1;
	}
	return 0;
}

unsigned long
peek(size)
int	size;
{
	unsigned int		idat;
	unsigned char	cdat;
	unsigned long	secoff, ldat, datum;
	static long	myoff = 0;
	long ftell();

	if (dokmem) {
		secoff = secthead.s_paddr;	
		debug("Peeking kmem datum at sec base %lx label %lx off %lx\n", 
			secoff, symbol.n_value - secthead.s_paddr, offset + myoff + secoff);
	} else {
		ldsseek(symld, symbol.n_scnum);
		secoff = FTELL(symld);
		debug("Peeking file datum at sec base %lx label %lx off %lx\n", 
			secoff, symbol.n_value - secthead.s_paddr, offset + myoff);
		}
	secoff += offset + myoff;
	lseek(symfd, secoff + (symbol.n_value - secthead.s_paddr), 0);
	switch(size) {
		case 1: read(symfd, &cdat, size); datum = cdat; break;
		case 2: read(symfd, &idat, size); datum = idat; break;
		case 4: read(symfd, &ldat, size); datum = ldat; break;
		default: error("Funky size in peek\n");
		}
	myoff += size;
	return datum;
}

poke(datum, size)
long datum;
int	size;
{
	int		idat;
	char	buf[100], cdat, *datptr;
	long	secoff, ldat;
	static long	myoff = 0;
	long ftell();
	extern errno;

	if (dokmem) {
		secoff = secthead.s_paddr;	
		debug("Poking kmem datum %ld at sec base %lx label %lx off %lx\n", datum, 
			secoff, symbol.n_value - secthead.s_paddr, offset + myoff + secoff);
	} else {
		ldsseek(symld, symbol.n_scnum);
		secoff = FTELL(symld);
		debug("Poking file datum %ld at sec base %lx label %lx off %lx\n", datum, 
			secoff, symbol.n_value - secthead.s_paddr, offset + myoff);
		}
	switch(size) {
		case 1: cdat = datum; datptr = (char *) &cdat; break;
		case 2: idat = datum; datptr = (char *) &idat; break;
		case 4: ldat = datum; datptr = (char *) &ldat; break;
		}
	secoff += offset + myoff;
	lseek(symfd, secoff + (symbol.n_value - secthead.s_paddr), 0);
	write(symfd, datptr, size);
	myoff += size;
}

error(str, arg1, arg2, arg3, arg4)
char	*str, *arg1, *arg2, *arg3, *arg4;
{
	printf(str, arg1, arg2, arg3, arg4);
	exit(1);
}

char tab1[] = "0123456789abcdef";
char tab2[] = "0123456789ABCDEF";

int
scan_num(str, datum, size)
char	*str;
long	*datum;
int		*size;
{
	int	indx, ret = 0, base = 10, nbytes;
	long sign = 1;

	if (*str == '\0')
		return 0;
	if (*str == '-') {
		str++; sign = -1;
		}
	if (*str == '0') {
		str++; base = 8;
		*datum = 0;
		*size = 1;
		ret = 1;
		if (*str == 'x') {
			str++; base = 16;
			ret = 0;
			}
		}
	if (*str == '\0')
		return ret;
	ret = 0;
	nbytes = 0;
	*datum = 0;
	while(1) {
		if (*str == '\0') {
			/* do this before sign reversal */
			if ((*datum & 0xff) == *datum)
				*size = 1;
			else if ((*datum & 0xffff) == *datum)
				*size = 2;
			else *size = 4;
			if (ret)
				*datum *= sign;		/* set sign now */
			return ret;
			}
		/* check small letters */
		for(indx = 0; indx < base; indx++) 
			if (tab1[indx] == *str)
				break;
		if (indx < base) {
			*datum *= base;
			*datum += indx;
			str++;
			ret = 1;
			continue;
		} 
		/* check cap letters */
		for(indx = 0; indx < base; indx++) 
			if (tab2[indx] == *str)
				break;
		if (indx < base) {
			*datum *= base;
			*datum += indx;
			str++;
			ret = 1;
			continue;
		} 
		/* not a valid character */
		return 0;
		}
}

/*
 * Steps through string, parsing multi-char C-isms like \r, \n, \007, etc.
 * and leaves *cpp at next char.  Someday it will do C-isms.
 */
int
scan_byte(cpp, datum)
char **cpp;
long *datum;
{
	*datum = **cpp;
	*cpp = &((*cpp)[1]);
}




