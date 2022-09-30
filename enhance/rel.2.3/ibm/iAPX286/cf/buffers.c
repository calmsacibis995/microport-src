/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)buffers.c	1.6 */
/*
** I/O buffer data storage
**
** in a separate file to guarantee separate segments
** Modification History:
** M000: uport!dwight Sun Nov 10 16:39:55 PST 1985
**	Expanded the buffers. Note that NBUF must be a multiple of 4.
*/
#include	"sys/param.h"
#include	"config.h"
char	buf1[ 1 ][ 1 ];
char	buf2[ 1 ][ 1 ];
char	buf3[ 1 ][ 1 ];
char	buf4[ 1 ][ 1 ];

char *buffers[] = { &buf1[0][0], &buf2[0][0], &buf3[0][0], &buf4[0][0]};
