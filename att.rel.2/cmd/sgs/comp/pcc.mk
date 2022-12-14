#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ccs-s5:comp/pcc.mk	1.1"
#	@(#)pcc.mk	1.9
#
#	This makefile generates the vax or pdp11 portable C compiler.
#
#	The directories assumed (multi-machine view) are:
#
#			     /pci/iAPX286	optim	(iAPX286 only)
#			    /
#			   /c2 --- c2.mk		(vax only)
#			  /
#			 / cc.c
#			/  /
#		       /  /  pcc.mk		(this makefile)
#	/usr/src/cmd/cc --   /
#		       \    /   /mip
#			\  /     /
#		       /pcc ----- /iAPX286
#			   \     \
#			    \   /vax --- pcc.mk
#			     \
#			      \
#			     /pdp11 --- pcc.mk
#
TESTDIR=.
INS= /etc/install -n /bin
INSDIR=
CFLAGS=-O
LDFLAGS = -s -n
INCS=-I/uts/usr/include -I/cmd/src/cmd/sgs/inc/iAPX286

all: make_cc make_pcc

make_cc: ../cc.c
	if pdp11; \
	then \
		$(CC) $(LDFLAGS) $(CFLAGS) -UFLEXNAME -DCSW=SCJ1 \
			-'DCCNAME="pcc"' -o $(TESTDIR)/pcc ../cc.c; \
	elif iAPX286; \
	then \
		$(CC) $(LDFLAGS) $(CFLAGS) $(INCS) -o $(TESTDIR)/cc ../cc.c; \
	elif vax; \
	then \
		$(CC) $(LDFLAGS) $(CFLAGS) -DCSW=JFR -o $(TESTDIR)/cc ../cc.c; \
	fi

make_pcc:
	-if iAPX286; \
	then \
		cd iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if iAPX286; \
	then \
		cd ../pci/iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f optim.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if vax; \
	then \
		cd ../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if pdp11; \
	then \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS) -UFLEXNAME" TESTDIR=$(TESTDIR) all; \
	fi

clean:
	-rm -f *.o
	-if pdp11; \
	then \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if iAPX286; \
	then \
		cd iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if iAPX286; \
	then \
		cd ../pci/iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f optim.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if vax; \
	then \
		cd ../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk TESTDIR=$(TESTDIR) clean; \
	fi

clobber: clean
	-if pdp11; \
	then \
		rm -f $(TESTDIR)/pcc; \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if iAPX286; \
	then \
		rm -f $(TESTDIR)/cc; \
		cd iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if iAPX286; \
	then \
		cd ../pci/iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f optim.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if vax; \
	then \
		rm -f $(TESTDIR)/cc; \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if vax; \
	then \
		cd ../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk TESTDIR=$(TESTDIR) clobber; \
	fi

install: all
	-if pdp11; \
	then \
		$(INS) $(TESTDIR)/pcc; \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS) -UFLEXNAME" TESTDIR=$(TESTDIR) \
			INSTALL=/lib install; \
	fi
	-if iAPX286; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../pci/iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f optim.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi
	-if vax; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi

justinstall: make_pcc
	#
	# justinstall is just like install except that it only depends
	# on make_pcc.  This forces the C compiler interface (cc or pcc)
	# to be assumed to be up to date.  This is needed when there is
	# incompatability between the old compilation package, and the
	# new one.  justinstall must only do installs.
	#
	-if pdp11; \
	then \
		$(INS) $(TESTDIR)/pcc; \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS) -UFLEXNAME" TESTDIR=$(TESTDIR) \
			INSTALL=/lib install; \
	fi
	-if iAPX286; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../pci/iAPX286; \
		$(MAKE) -$(MAKEFLAGS) -f optim.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi
	-if vax; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi

FRC:	# what is this for?
