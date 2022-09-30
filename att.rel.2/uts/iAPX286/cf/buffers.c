/*      Copyright (c) 1985 AT&T */
/*        All Rights Reserved   */

/*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T     */
/*      The copyright notice above does not evidence any        */
/*      actual or intended publication of such source code.     */

/* @(#)buffers.c	1.6 */
/*
** I/O buffer data storage
**
** in a separate file to guarantee in a separate segment
*/
#include	"sys/param.h"
#include	"config.h"
char	buffers[ NBUF ][ SBUFSIZE ];
