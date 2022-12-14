#/* iAPX286 %W% %D% */
#	@(#)libF77.mk	1.10
#	LIBF77 MAKEFILE

PFX =
CC = $(PFX)cc
AR = $(PFX)ar
STRIP = $(PFX)strip
CFLAGS = -O
INS = install
INSDIR = $(ROOT)/usr/lib

INCRT=$(ROOT)/usr/include
LIB=$(DEST)/usr/lib/$(MODEL)
LIBP=$(DEST)/usr/lib/libp/$(MODEL)
VARIANT =
LIBNAME = lib$(VARIANT)F77.a


OBJ =	Version.o abort_.o c_abs.o c_cos.o c_div.o c_exp.o c_log.o \
	c_sin.o c_sqrt.o cabs.o d_abs.o d_acos.o d_asin.o d_atan.o \
	d_atn2.o d_cnjg.o d_cos.o d_cosh.o d_dim.o d_exp.o d_imag.o \
	d_int.o d_lg10.o d_log.o d_mod.o d_nint.o d_prod.o d_sign.o \
	d_sin.o d_sinh.o d_sqrt.o d_tan.o d_tanh.o derf_.o derfc_.o \
	ef1asc_.o ef1cmc_.o erf_.o erfc_.o getarg_.o getenv_.o h_abs.o \
	h_dim.o h_dnnt.o h_indx.o h_len.o h_mod.o h_nint.o h_sign.o \
	hl_ge.o hl_gt.o hl_le.o hl_lt.o i_abs.o i_dim.o i_dnnt.o \
	i_indx.o i_len.o i_mod.o i_nint.o i_sign.o iargc_.o l_ge.o \
	l_gt.o l_le.o l_lt.o main.o mclock_.o outstr_.o pow_ci.o \
	pow_dd.o pow_di.o pow_hh.o pow_ii.o pow_ri.o pow_zi.o pow_zz.o \
	r_abs.o r_acos.o r_asin.o r_atan.o r_atn2.o r_cnjg.o r_cos.o \
	r_cosh.o r_dim.o r_exp.o r_imag.o r_int.o r_lg10.o r_log.o \
	r_mod.o r_nint.o r_sign.o r_sin.o r_sinh.o r_sqrt.o r_tan.o \
	r_tanh.o rand_.o s_cat.o s_cmp.o s_copy.o s_paus.o s_rnge.o \
	s_stop.o signal_.o sinh.o subout.o system_.o tanh.o z_abs.o \
	z_cos.o z_div.o z_exp.o z_log.o z_sin.o z_sqrt.o

all:
	$(MAKE)  libF77.a  -e -f libF77.mk $(COMMON) MODEL=small \
		CFLAGS="$(CFLAGS)  -Ms  -I$(INCRT)"\
		clean; 
		mv libF77.a libF77.a.sm;
	$(MAKE)  libF77.a  -e -f libF77.mk  $(COMMON) MODEL=large \
		CFLAGS="$(CFLAGS)  -Ml  -I$(INCRT)";
		mv libF77.a libF77.a.lg;
#	$(MAKE)  libF77.a  -e -f libF77.mk  $(COMMON) MODEL=huge \
		CFLAGS="$(CFLAGS)  -Mh  -I$(INCRT)" \
		; \
	echo "No huge model C compiler yet."


libF77.a:	$(OBJ)
		$(AR) r libF77.a $?
	if pdp11; then \
		$(STRIP) libF77.a; \
	else \
		$(STRIP) -r libF77.a; \
		$(AR) ts libF77.a; \
	fi;

lio.o:	lio.h

lread.o: lio.h

.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	-rm -f $(OBJ)

profiledlib:
	make -f libm.mk -e libp.$(LIBNAME) LIBNAME=libp.$(LIBNAME) \
						CFLAGS="-p $(CFLAGS)"

install: all
	mv libF77.a.sm libF77.a
	$(INS) -f $(INSDIR)/small libF77.a
	mv libF77.a.lg libF77.a
	$(INS) -f $(INSDIR)/large libF77.a

clobber:	clean
	-rm -f $(LIBNAME) libp.$(LIBNAME)
