#	Copyright (c) 1985 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#/*   @(#)template.mk	1.3 - 85/08/09 */
#	i286 SGS GLOBAL MAKEFILE
#
#
#
#	This is the master makefile for building the i286 SGS.
# You can invoke the lower level makefiles through this one, or
# you can also invoke the lower level makefile directly to build
# a specific tool.  You should only invoke the lower level
# makefile for a tool when your working directory is the tool's
# machine dependent source directory.
#
#
# You may select options for the entire SGS from within this makefile.
# Read on for details....


#	SGS	specifies the name prefix for each of the built tools.
#		For example, if SGS=i286, all of the tool names are
#		prefixed by "i286", as in "i286as", "i286cc", etc.

SGS=


#	ROOT	specifies the absolute root directory node for things
#		which will ultimately be executable.  It is usually
#		the absolute pathname of this directory (.).

ROOT=


#	SGSBASE	specifies the absolute root directory node for the
#		SGS source structure.

SGSBASE=


#	BINDIR	specifies the absolute directory into which the makefile
#		places the executable tools it builds.  This path may
#		be different from the directory from which the tools
#		will execute (see $(SGSBASE)/inc/i286/paths.h).

BINDIR=


#	LIBDIR	specifies the absolute directory into which the makefile
#		places other executable tools and libraries.  As with
#		BINDIR, this directory may be different from the one
#		from which the tools will ultimately be executed.  See
#		$(SGSBASE)/inc/i286/paths.h .

LIBDIR=


#	INCDIR	is the directory where may be found include files that
#		usually reside in /usr/include on native-mode UNIX SGSs
#		to support compiling i286 files.  All files in $(SGSBASE/inc/common
#		and $(SGSBASE)/inc/i286 will be copied to this directory.
#		This will make it possible to remove all of the source directories
#		after the i286 SGS has been built.
#

INCDIR=


#	OWN
#	GRP
#	MODE	define the OWNer, GRouP, and file MODE for files
#		that are installed in LIBDIR and BINDIR.  The mode
#		bits should contain the "execute" bits.  Execute bits for
#		non-executable files, like libraries, will be turned off.

OWN=
GRP=
MODE=0755



#	ARCH	declares the byte ordering of the host processor:
#
#			AR32W	for 3B20S (and native i286)
#			AR32WR	for VAX
#

ARCH=AR32WR


#	ARFORMAT
#		defines the archive format:
#
#			OLDAR	pre-System V archive
#			PORT5AR	portable random access (System V Release 1) 
#				archive
#			PORTAR	portable ascii header (System V Release 2) 
#				archive
#

ARFORMAT=PORTAR


#	FLEX	declares whether the SGS supports "flexnames" --
#		arbitrary length names in C, assembly language,
#		ar, etc.  FLEX=  for non-flexnames and
#		FLEX=-DFLEXNAMES to turn on flexnames.  With
#		FLEXNAMES enabled, you can still force the compiler
#		to truncate names to 8 characters with the SGScc -T
#		option.

FLEX=-DFLEXNAMES

#	NOTE:  To get the equivalent of a UNIX 5.0 (System V) SGS,
#		choose ARFORMAT=PORT5AR and FLEX= .
#		You will also have to include the iAPX magic
#		numbers among those that the 5.0 ar recognizes
#		and build it, rather than this SGS's ar, as the
#		ar for the SGS.


#	FFLAG	will be "-f" if we are trying to build the sgs on 
#		a 3b5 or 3b2.  This flag is used if a component 
#		(compiler front end, as, or dis) contains any 
#		floating point instructions.  The flag is passed to
#		cc (note that the flag is needed at load time only, 
#		but that one must use cc to load rather than ld).

FFLAG=

#	PD_MACH indicate the machine that i286cpp will predefine.  
#		The current default is to not have a machine 
#		defined (PD_MACH=D_nomach).  One example of a 
#		desired machine name is "i286" or "80286".  If it 
#		is desired to have i286cpp automatically predefine
#		a machine name when it is used (just as the cpp on 
#		the VAX machines predefine "vax"), then set 
#		PD_MACH=D_newmach and set PD_MY_MACH= the desired 
#		machine name in escaped quotes (for example, 
#		PD_MY_MACH=\"i286\").

PD_MACH=D_nomach
PD_MY_MACH=

#	PD_SYS	indicate the system that i286cpp will predefine.  
#		The current default is to not have a system defined 
#		(PD_SYS=D_nosys).  Some examples of a desired system 
#		name are "unix" and "standalone".  If it is desired to 
#		have iapxcpp automatically predefine a system name 
#		when it is used (just as the cpp on the VAX systems 
#		predefine "unix"), then set PD_SYS=D_newsys and set 
#		PD_MY_SYS= the desired system name in escaped quotes 
#		(for example, PD_MY_SYS=\"unix\").

PD_SYS=D_unix
PD_MY_SYS=

#	The following parameter specifies the default include
#	directory for cpp.

USRINC=

AR=	ar
CC=	cc
LINT=	lint
MAKE=	make
YACC=	yacc
LIBLD=	$(SGSBASE)/libld/i286/libld.a
CFLAGS=	-O
LDFLAGS=
LFLAGS=	-p

#	Special defines for building a target large & small model libld

MODEL=
MSPEC=

MAKEARGS=	-$(MAKEFLAGS) SGS="$(SGS)" ROOT="$(ROOT)" \
		SGSBASE="$(SGSBASE)" \
		OWN="$(OWN)" GRP="$(GRP)" MODE="$(MODE)" \
		AR="$(AR)" CC="$(CC)" YACC="$(YACC)" \
		LINT="$(LINT)" LFLAGS="$(LFLAGS)" \
		CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
		BINDIR="$(BINDIR)" LIBDIR="$(LIBDIR)" \
		ARCH="$(ARCH)" ARFORMAT="$(ARFORMAT)" FLEX=$(FLEX) \
		USRINC=$(USRINC) INCDIR="$(INCDIR)" LIBLD="$(LIBLD)" FFLAG=$(FFLAG)

first:
		if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi
		if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
		if [ ! -d $(INCDIR) ] ; then mkdir $(INCDIR) ; fi

all:		libld
		cd $(SGSBASE)/ar/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/as/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/cmd/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/cpp/common; \
		$(MAKE) $(MAKEARGS) PD_MACH=$(PD_MACH) PD_MY_MACH=$(PD_MY_MACH) \
			PD_SYS=$(PD_SYS) PD_MY_SYS=$(PD_MY_SYS) \
			-f $(SGSBASE)/cpp/common/cpp.mk
		cd $(SGSBASE)/dis/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/dump/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/inc/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/ld/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/lorder/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/nm/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/optim/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/comp/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/size/i286; $(MAKE) $(MAKEARGS)
		cd $(SGSBASE)/strip/i286; $(MAKE) $(MAKEARGS)

libld:
		cd $(SGSBASE)/libld/i286; \
		$(MAKE) $(MAKEARGS) -f $(SGSBASE)/libld/common/makefile

install:	libld
		cd $(SGSBASE)/ar/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/as/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/cmd/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/cpp/common; \
		$(MAKE) $(MAKEARGS) PD_MACH=$(PD_MACH) PD_MY_MACH=$(PD_MY_MACH) \
			PD_SYS=$(PD_SYS) PD_MY_SYS=$(PD_MY_SYS) \
			-f $(SGSBASE)/cpp/common/cpp.mk install
		cd $(SGSBASE)/dis/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/dump/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/inc/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/ld/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/lorder/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/nm/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/optim/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/comp/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/size/i286; $(MAKE) $(MAKEARGS) install
		cd $(SGSBASE)/strip/i286; $(MAKE) $(MAKEARGS) install
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/$(SGS)env $(SGSBASE)/xenv/i286/env-i286 
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/make $(SGSBASE)/xenv/i286/make.tmp
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/vax $(SGSBASE)/xenv/i286/vax
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/u3b $(SGSBASE)/xenv/i286/u3b
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/u370 $(SGSBASE)/xenv/i286/u370
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/u3b5 $(SGSBASE)/xenv/i286/u3b5
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/u3b2 $(SGSBASE)/xenv/i286/u3b2
		sh $(SGSBASE)/sgs.install $(MODE) $(OWN) $(GRP) \
		$(BINDIR)/iAPX286 $(SGSBASE)/xenv/i286/iAPX286 

libldinstall:
		cd $(SGSBASE)/libld/i286; \
		$(MAKE) $(MAKEARGS) MODEL=$(MODEL) MSPEC=$(MSPEC) \
			-f $(SGSBASE)/libld/i286/makefile install

save:
		cd $(SGSBASE)/ar/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/as/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/cmd/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/cpp/common; \
		$(MAKE) $(MAKEARGS) PD_MACH=$(PD_MACH) PD_MY_MACH=$(PD_MY_MACH) \
			PD_SYS=$(PD_SYS) PD_MY_SYS=$(PD_MY_SYS) \
			-f $(SGSBASE)/cpp/common/cpp.mk save
		cd $(SGSBASE)/dis/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/dump/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/inc/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/ld/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/lorder/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/nm/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/optim/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/comp/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/size/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/strip/i286; $(MAKE) $(MAKEARGS) save
		cd $(SGSBASE)/libld/i286; \
		$(MAKE) $(MAKEARGS) -f $(SGSBASE)/libld/common/makefile save

shrink:		libldshrink
		cd $(SGSBASE)/ar/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/as/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/cmd/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/cpp/common; \
		$(MAKE) $(MAKEARGS) PD_MACH=$(PD_MACH) PD_MY_MACH=$(PD_MY_MACH) \
			PD_SYS=$(PD_SYS) PD_MY_SYS=$(PD_MY_SYS) \
			-f $(SGSBASE)/cpp/common/cpp.mk shrink
		cd $(SGSBASE)/dis/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/dump/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/ld/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/lorder/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/nm/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/optim/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/comp/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/size/i286; $(MAKE) $(MAKEARGS) shrink
		cd $(SGSBASE)/strip/i286; $(MAKE) $(MAKEARGS) shrink

libldshrink:
		cd $(SGSBASE)/libld/common; \
		$(MAKE) $(MAKEARGS) -f $(SGSBASE)/libld/common/makefile shrink
