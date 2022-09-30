/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)	1.1	*/

#define BRSIZ	512
struct brbuf {
	int	nl, nr;
	char	*next;
	char	b[BRSIZ];
	int	fd;
};
long lseek();
