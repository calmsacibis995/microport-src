
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)debugger.h	1.2"

#define DBSTKSIZ    32
#define VARTBLSIZ   32
#define LINBUFSIZ   161
#define MAXSTRSIZ   128
#define STRSPCSIZ   512

#define EOF         -1

#define streq(s1,s2) (dbstrcmp((s1),(s2)) == 0)

typedef unsigned char   uchar;

extern short    dbgetchar();
extern short    dbgetitem();
extern char     *dbstrdup();
extern char     *dbstralloc();
extern void     dbstrfree();
extern int      dbstrcmp();
extern void     dbstrcpy();
extern int      dbextname();
extern int      dbstackcheck();
extern int      dbtypecheck();
extern void     dberror();
extern void     dbprintitem();

extern char     dbverbose;
extern ushort   dbibase;
extern ushort   dbtos;
extern struct item dbstack[];

struct item {
    union {
	uchar byte[4];
	ushort word[2];
	ulong number;
	char *string;
    } value;
    unsigned type : 3;
};

/* item types */

/* #define EOF         -1 */
/* #define NULL         0 */

#define NUMBER          1
#define STRING          2
#define NAME            3

#define TYPEMAX         3

struct variable {
    char *name;
    struct item item;
};
