/*	Copyright (c) 1985 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#/*   @(#)paths.h	1.3 - 85/08/09 */
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
#define LIBDIR		"I286LIBDIR"
#define LLIBDIR1	"I286LIBDIR/../usr/lib"
#define NDELDIRS	2

/*
 * Directory containing executable ("bin") files
 */
#define BINDIR	"I286BINDIR"

/*
 * Directory for "temp"  files
 */
#define TMPDIR	"I286TMPDIR"

/*
 * Name of default output object file
 */
#define A_OUT	"SGSa.out"

/*
 * The following pathnames will be used by the "cc" command
 *
 *
 *	i286 cross compiler
 */
#define CPP	"I286CPP"
/*
 * Directory containing include ("header") files for users' use
 */
#define INCDIR	"I286INCDIR"
#define CCOM	"I286LIBDIR/SGSccom"
#define OPTIM	"I286LIBDIR/SGSoptim"
/*
 *	i286 cross assembler
 */
#define AS	"I286BINDIR/SGSas"
#define AS1	"I286LIBDIR/SGSas1"	/* assembler pass 1 */
#define AS2	"I286LIBDIR/SGSas2"	/* assembler pass 2 */
/*
 *	i286 link editor
 */
#define LD	"I286BINDIR/SGSld"
#define LD2	"I286LIBDIR/SGSld2"	/* link editor pass 2 */
