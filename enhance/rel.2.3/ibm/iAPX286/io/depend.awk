	echo "# Dependencies generated on `date`" >> .deplist

	echo 'FILES =\c' > .filelist
	echo $(CFILES) | 
	for i in $(CFILES); \
	do \
	    file=`rootname $i`
 	    echo "\\\\\n\t$$(LIBNAME)($$file.o)\c" >> .filelist; \
 	    echo "$$(LIBNAME)($$file.o): \c"; \
# 	    echo "\\\\\n\t$$file.o\c" >> .filelist; \
# 	    echo "$$file.o: \c"; \
 	    $(CPP) $(SRCDIR)/$$i | \
	     grep -v "/usr/include" | \
	     awk '/^# 1 /{printf "\\\n\t%s",$$3};END{print;print "\t$$(CC) -c $$(CFLAGS) $$(SRCDIR)/'$$i'";print "\tar rv $$(LIBNAME) '$$file.o'";print}'|\
#	     awk '/^# 1 /{printf "\\\n\t%s",$$3};END{print;print "\t$$(CC) -c $$(CFLAGS) $$(SRCDIR)/'$$i'";print}'|\
 	     sed 's/"//g'; \
 	done >> .deplist; \
 	echo '\n' >> .filelist; \
 	echo '/^FILES /\n.,/^$$/d\n-1r .filelist\n/^# DO NOT DELETE/,$$d\nr .deplist\nw $(MAKENEW)\nq' | ed - $(MAKEFILE)
	mv $(MAKEFILE) $(MAKEOLD)
	mv $(MAKENEW) $(MAKEFILE)
	ln -f $(MAKEFILE) IBMAT.mk
	rm -f .deplist .filelist

# anything after the next line will disappear!
# DO NOT DELETE THIS LINE - make depend uses it
