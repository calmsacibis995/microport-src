# makefile for keyset
#	make, make all, make setkey:	this is what you want to type

MAKEFILE = Makefile
NMK=.$(MAKEFILE)
OMK=.x$(MAKEFILE)

CC	= /bin/cc
CPP	= /lib/cpp -DiAPX286
INCRT	= ../..
CPPFLAGS = -I$(INCRT) -I/usr/include 
CFLAGS	= -s $(CPPFLAGS)
ROOT	= ../../../dist

FILES = setkey prtkey tstkey

all:	$(FILES)

clean:
	-rm -f setkey *.o
clobber:
	-rm -f $(ROOT)/etc/setkey

install:	all clobber
	cp setkey $(ROOT)/etc/setkey
	chmod 510 $(ROOT)/etc/setkey
	chown bin $(ROOT)/etc/setkey
	chgrp sys $(ROOT)/etc/setkey
	ln $(ROOT)/etc/setkey $(ROOT)/etc/keyset

depend:
	echo '# DO NOT DELETE THIS LINE - make depend uses it\n' > .dl
	echo "# Dependencies generated on `date`" >> .dl
	for file in $(FILES); \
	do \
	    echo "$$file" >> /dev/tty; \
 	    echo "$$file: \c"; \
 	    $(CPP) $(CPPFLAGS) $$file.c | \
	        grep -v '/usr/include' | \
	        awk '/^# 1 /{printf "\\\n\t%s",$$3};END{print;print}' | \
 	        sed 's/"//g'; \
	done >> .dl
	echo '/^# DO NOT DELETE/,$$d\nr .dl\nw $(NMK)\nq' | ed - $(MAKEFILE)
	mv $(MAKEFILE) $(OMK)
	mv $(NMK) $(MAKEFILE)
	ln $(MAKEFILE) IBMAT.mk
	rm -f .dl

# anything after the next line will disappear!
# DO NOT DELETE THIS LINE - make depend uses it

# Dependencies generated on Fri May  8 04:35:39 PDT 1987
setkey: \
	setkey.c\
	../../sys/types.h\
	../../sys/kd.h\
	../../sys/kd_info.h\
	../../sys/setkey.h

prtkey: \
	prtkey.c\
	../../sys/types.h\
	../../sys/kd.h\
	../../sys/kd_info.h\
	../../sys/setkey.h

tstkey: \
	tstkey.c\
	../../sys/types.h\
	../../sys/kd.h\
	../../sys/kd_info.h\
	../../sys/setkey.h

