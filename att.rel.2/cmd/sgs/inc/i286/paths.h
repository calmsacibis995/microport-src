/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Pathnames for i286
 */


/*
 * i286 define constants
 */
#define I286DIR		"/usr/bin"
#define I286DRLNG	sizeof(I286DIR)-1
#define I286TMP		"/usr/tmp"
#define I286TMPLN	sizeof(I286TMP)-1

/*
 * Directory containing libraries and executable files (e.g. assembler
 *	pass 1)
 */
#define LIBDIR		"/lib"
#define LLIBDIR1	"/lib/../usr/lib"
#define NDELDIRS	2

/*
 * Directory containing executable ("bin") files
 */
#define BINDIR	"/bin"

/*
 * Directory for "temp"  files
 */
#define TMPDIR	"/tmp"

/*
 * Name of default output object file
 */
#define A_OUT	"a.out"

/*
 * The following pathnames will be used by the "cc" command
 *
 *
 *	i286 cross compiler
 */
#define CPP	"/lib/cpp"
/*
 * Directory containing include ("header") files for users' use
 */
#define INCDIR  "/usr/include"
#define CCOM	"/lib/ccom"
#define OPTIM	"/lib/optim"
/*
 *	i286 cross assembler
 */
#define AS	"/bin/as"
#define AS1	"/lib/as1"	/* assembler pass 1 */
#define AS2	"/lib/as2"	/* assembler pass 2 */
/*
 *	i286 link editor
 */
#define LD	"/bin/ld"
#define LD2	"/lib/ld2"	/* link editor pass 2 */
