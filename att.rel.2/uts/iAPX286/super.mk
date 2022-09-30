#       @(#)super.mk	1.2 - 85/09/05
#
#       make the superoptimizing sed scripts
#
#             Copyright (c) 1985 AT&T"
#                All Rights Reserved"
#
#        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T"
#     The copyright notice above does not evidence any actual or"
#             intended publication of such source code."
#

C = clobber
TYPE = wini40

IO = io
OS = os
CF = cf
PWB = pwb

SEDSCRIPTS = \
	SEDaa\
	SEDab\
	SEDac\
	SEDad

FILES = \
	$(IO)/bio.c\
	$(IO)/tt0.c\
	$(IO)/tty.c\
	$(IO)/err.c\
	$(IO)/mem.c\
	$(IO)/sxt.c\
	$(IO)/sys.c\
	$(IO)/partab.c\
	$(IO)/clist.c\
	$(IO)/console.c\
	$(IO)/wn.c\
	$(IO)/wn.subs.c\
	$(IO)/wn.conf.c\
	$(IO)/wn.dump.c\
	$(IO)/sio.c\
	$(IO)/cico.c\
	$(IO)/c8274.c\
	$(IO)/lp.c\
	$(OS)/acct.c\
	$(OS)/alloc.c\
	$(OS)/clock.c\
	$(OS)/errlog.c\
	$(OS)/fio.c\
	$(OS)/flock.c\
	$(OS)/fp.c\
	$(OS)/iget.c\
	$(OS)/ipc.c\
	$(OS)/ldt.c\
	$(OS)/lock.c\
	$(OS)/machdep.c\
	$(OS)/macherr.c\
	$(OS)/main.c\
	$(OS)/malloc.c\
	$(OS)/msg.c\
	$(OS)/nami.c\
	$(OS)/pipe.c\
	$(OS)/prf.c\
	$(OS)/rdwri.c\
	$(OS)/sem.c\
	$(OS)/shm.c\
	$(OS)/sig.c\
	$(OS)/sigcode.c\
	$(OS)/slp.c\
	$(OS)/spl.c\
	$(OS)/subr.c\
	$(OS)/sys1.c\
	$(OS)/sys2.c\
	$(OS)/sys3.c\
	$(OS)/sys4.c\
	$(OS)/sysent.c\
	$(OS)/text.c\
	$(OS)/trap.c\
	$(OS)/userio.c\
	$(OS)/utssys.c\
	$(CF)/ifile\
	$(CF)/gdt.s\
	$(CF)/buffers.c\
	$(CF)/linesw.c\
	$(PWB)/prof.c

#       Run clobber where needed first, then make unoptimized libraries,
#       build an unoptimized kernel, run CONSTRUCT on it, then run clobber
#       again to make sure the unoptimized libraries are deleted.
#       The sub-directories not explicitly visited here don't need to be
#       rebuilt, since they don't use the SED scripts or the -O flag.
#       If any of them are out-of-date, the make in `cf' will take care
#       of them.
$(SEDSCRIPTS): $(FILES)
	(cd $(PWB) ; $(MAKE) -f pwb.mk $(C) ; $(MAKE) -f pwb.mk.unopt )
	(cd $(OS)  ; $(MAKE) -f os.mk $(C)  ; $(MAKE) -f os.mk.unopt  )
	(cd $(IO)  ; $(MAKE) -f io.mk $(C)  ; $(MAKE) -f io.mk.unopt  )
	(cd $(CF)  ; rm -f *.o current* config.h conf.c handlers.c ;\
	$(MAKE) -f cf.mk $(TYPE) "NAME=unix286.unopt"  )
	PFX=$(PFX) sh CONSTRUCT
	-rm -f unix286.unopt unix.nm
	(cd $(PWB) ; $(MAKE) -f pwb.mk $(C) )
	(cd $(OS)  ; $(MAKE) -f os.mk $(C)  )
	(cd $(IO)  ; $(MAKE) -f io.mk $(C)  )
