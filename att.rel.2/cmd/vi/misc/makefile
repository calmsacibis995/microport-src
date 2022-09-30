#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.1
CFLAGS=

all:  xstr mkstr fold ctags

# Must use cc
mkstr: mkstr.c
	cc $(CFLAGS) mkstr.c -o mkstr

xstr:  xstr.c
	cc $(CFLAGS) xstr.c -o xstr

fold:  fold.c
	cc $(CFLAGS) fold.c -o fold

ctags: ctags.c
	cc $(CFLAGS) ctags.c -o ctags

clean:
	rm -f ctags xstr mkstr fold
