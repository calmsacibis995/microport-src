#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	@(#)	1.3
LIB = $(ROOT)/lib

install:
	make -f makefile clean
	(cd iAPX286 ; make -f makefile clean)
	make -f makefile MSIZE=s MODEL=small all
	(cd iAPX286 ; make -f makefile MSIZE=s MODEL=small all)
	rm -f libm.a
	$(AR) rv libm.a *.o iAPX286/*.o
	cp libm.a $(LIB)/small
	rm -f libm.a
	$(AR) rv libm.a proflib/*.o iAPX286/proflib/*.o
	cp libm.a $(LIB)/libp/small
	
	make -f makefile clean
	(cd iAPX286 ; make -f makefile clean)
	make -f makefile MSIZE=l MODEL=large all
	(cd iAPX286 ; make -f makefile MSIZE=l MODEL=large all)
	rm -f libm.a
	$(AR) rv libm.a *.o iAPX286/*.o
	cp libm.a $(LIB)/large
	rm -f libm.a
	$(AR) rv libm.a proflib/*.o iAPX286/proflib/*.o
	cp libm.a $(LIB)/libp/large

clean:
	make -f makefile clean
	(cd iAPX286 ; make -f makefile clean)

clobber:	clean
	rm libm.a
