#
# ${R_HOME}/src/nmath/Makefile


srcdir = .
top_srcdir = ../..

top_builddir = ../..
subdir = src/nmath

include $(top_builddir)/Makeconf

## <NOTE>
## Keep these in sync with ./standalone/Makefile.in and *.win!
SOURCES = \
	mlutils.c \
	d1mach.c i1mach.c \
	fmax2.c fmin2.c fprec.c fround.c ftrunc.c \
	sign.c fsign.c \
	imax2.c imin2.c \
	chebyshev.c log1p.c lgammacor.c gammalims.c stirlerr.c bd0.c \
	gamma.c lgamma.c gamma_cody.c beta.c lbeta.c polygamma.c cospi.c \
	bessel_i.c bessel_j.c bessel_k.c bessel_y.c \
	choose.c \
	snorm.c sexp.c \
	dgamma.c pgamma.c qgamma.c rgamma.c \
	dbeta.c pbeta.c qbeta.c rbeta.c \
	dunif.c punif.c qunif.c runif.c \
	dnorm.c pnorm.c qnorm.c rnorm.c \
	dlnorm.c plnorm.c qlnorm.c rlnorm.c \
	df.c pf.c qf.c rf.c dnf.c \
	dt.c pt.c qt.c rt.c dnt.c \
	dchisq.c pchisq.c qchisq.c rchisq.c rnchisq.c \
	dbinom.c pbinom.c qbinom.c rbinom.c \
	rmultinom.c \
	dcauchy.c pcauchy.c qcauchy.c rcauchy.c \
	dexp.c pexp.c qexp.c rexp.c \
	dgeom.c pgeom.c qgeom.c rgeom.c \
	dhyper.c phyper.c qhyper.c rhyper.c \
	dnbinom.c pnbinom.c qnbinom.c rnbinom.c \
	dpois.c ppois.c qpois.c rpois.c \
	dweibull.c pweibull.c qweibull.c rweibull.c \
	dlogis.c plogis.c qlogis.c rlogis.c \
	dnchisq.c pnchisq.c qnchisq.c \
	dnbeta.c pnbeta.c qnbeta.c \
	pnf.c pnt.c qnf.c qnt.c \
	ptukey.c qtukey.c \
	toms708.c wilcox.c \
	signrank.c
## </NOTE>
DEPENDS = $(SOURCES:.c=.d)
OBJECTS = $(SOURCES:.c=.o)
HEADERS = bessel.h dpq.h nmath.h nmath2.h

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)
DISTFILES = Makefile.in Makefile.win $(SOURCES) $(HEADERS)

SUBDIRS_WITH_NO_BUILD = standalone

#ALL_CFLAGS = $(ALL_CFLAGS_LO)

all: R

Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status $(SOURCES)
	@cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

Makedeps: Makefile $(DEPENDS)
	@cat $(DEPENDS) >> Makefile
	@touch $@

R: Makedeps
	@$(MAKE) libnmath.a

libnmath.a: $(OBJECTS)
	@rm -Rf $@
	$(AR) -cr $@ $(OBJECTS)
	$(RANLIB) $@

mostlyclean: clean
clean:
	@(cd standalone; $(MAKE) clean)
	@-rm -Rf .libs _libs
	@-rm -f Makedeps *.d *.o *.a *.lo *.la stamp-lo
distclean: clean
	@(cd standalone; $(MAKE) distclean)
	@-rm -f Makefile
maintainer-clean: distclean

install install-strip uninstall TAGS info dvi check:

distdir: $(DISTFILES)
	@for f in $(DISTFILES); do \
	  test -f $(distdir)/$${f} \
	    || ln $(srcdir)/$${f} $(distdir)/$${f} 2>/dev/null \
	    || cp -p $(srcdir)/$${f} $(distdir)/$${f}; \
	done
	@for d in $(SUBDIRS_WITH_NO_BUILD); do \
	  ((cd $(srcdir); $(TAR) -c -f - $(DISTDIR_TAR_EXCLUDE) $${d}) \
	      | (cd $(distdir); $(TAR) -x -f -)) \
	    || exit 1; \
        done

## Automagically generated dependencies:

mlutils.o: mlutils.c ../../src/include/config.h nmath.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
d1mach.o: d1mach.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
i1mach.o: i1mach.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
fmax2.o: fmax2.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
fmin2.o: fmin2.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
fprec.o: fprec.c ../../src/include/config.h nmath.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
fround.o: fround.c ../../src/include/config.h nmath.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
ftrunc.o: ftrunc.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
sign.o: sign.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
fsign.o: fsign.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
imax2.o: imax2.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
imin2.o: imin2.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
chebyshev.o: chebyshev.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
log1p.o: log1p.c ../../src/include/config.h nmath.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
lgammacor.o: lgammacor.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
gammalims.o: gammalims.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
stirlerr.o: stirlerr.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
bd0.o: bd0.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
gamma.o: gamma.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
lgamma.o: lgamma.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
gamma_cody.o: gamma_cody.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
beta.o: beta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
lbeta.o: lbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
polygamma.o: polygamma.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
cospi.o: cospi.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
bessel_i.o: bessel_i.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h bessel.h ../../src/include/R_ext/Memory.h
bessel_j.o: bessel_j.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h bessel.h ../../src/include/R_ext/Memory.h
bessel_k.o: bessel_k.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h bessel.h ../../src/include/R_ext/Memory.h
bessel_y.o: bessel_y.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h bessel.h ../../src/include/R_ext/Memory.h
choose.o: choose.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
snorm.o: snorm.c ../../src/include/R_ext/Random.h \
 ../../src/include/R_ext/Boolean.h nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h
sexp.o: sexp.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dgamma.o: dgamma.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pgamma.o: pgamma.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qgamma.o: qgamma.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rgamma.o: rgamma.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dbeta.o: dbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pbeta.o: pbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qbeta.o: qbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rbeta.o: rbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dunif.o: dunif.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
punif.o: punif.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qunif.o: qunif.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
runif.o: runif.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dnorm.o: dnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pnorm.o: pnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qnorm.o: qnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rnorm.o: rnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dlnorm.o: dlnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
plnorm.o: plnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qlnorm.o: qlnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rlnorm.o: rlnorm.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
df.o: df.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h dpq.h
pf.o: pf.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h dpq.h
qf.o: qf.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h dpq.h
rf.o: rf.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h
dnf.o: dnf.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
dt.o: dt.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h dpq.h
pt.o: pt.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h dpq.h
qt.o: qt.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h dpq.h
rt.o: rt.c nmath.h ../../src/include/config.h ../../src/include/Rconfig.h \
 ../../src/include/Rmath.h ../../src/include/R_ext/Boolean.h \
 ../../src/include/R_ext/RS.h ../../src/include/R_ext/Error.h \
 ../../src/include/R_ext/Print.h ../../src/include/R_ext/Arith.h \
 ../../src/include/R_ext/libextern.h ../../src/include/libintl.h
dnt.o: dnt.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
dchisq.o: dchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pchisq.o: pchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qchisq.o: qchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rchisq.o: rchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
rnchisq.o: rnchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dbinom.o: dbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pbinom.o: pbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qbinom.o: qbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rbinom.o: rbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rmultinom.o: rmultinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dcauchy.o: dcauchy.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pcauchy.o: pcauchy.c ../../src/include/config.h nmath.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qcauchy.o: qcauchy.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rcauchy.o: rcauchy.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dexp.o: dexp.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pexp.o: pexp.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qexp.o: qexp.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rexp.o: rexp.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dgeom.o: dgeom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pgeom.o: pgeom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qgeom.o: qgeom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rgeom.o: rgeom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dhyper.o: dhyper.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
phyper.o: phyper.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qhyper.o: qhyper.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rhyper.o: rhyper.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
dnbinom.o: dnbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pnbinom.o: pnbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qnbinom.o: qnbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rnbinom.o: rnbinom.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dpois.o: dpois.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
ppois.o: ppois.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qpois.o: qpois.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rpois.o: rpois.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dweibull.o: dweibull.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pweibull.o: pweibull.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qweibull.o: qweibull.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rweibull.o: rweibull.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dlogis.o: dlogis.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
plogis.o: plogis.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qlogis.o: qlogis.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
rlogis.o: rlogis.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h
dnchisq.o: dnchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pnchisq.o: pnchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qnchisq.o: qnchisq.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
dnbeta.o: dnbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pnbeta.o: pnbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qnbeta.o: qnbeta.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pnf.o: pnf.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
pnt.o: pnt.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qnf.o: qnf.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qnt.o: qnt.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
ptukey.o: ptukey.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
qtukey.o: qtukey.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
toms708.o: toms708.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
wilcox.o: wilcox.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h ../../src/include/R_ext/Utils.h \
 ../../src/include/R_ext/Complex.h
signrank.o: signrank.c nmath.h ../../src/include/config.h \
 ../../src/include/Rconfig.h ../../src/include/Rmath.h \
 ../../src/include/R_ext/Boolean.h ../../src/include/R_ext/RS.h \
 ../../src/include/R_ext/Error.h ../../src/include/R_ext/Print.h \
 ../../src/include/R_ext/Arith.h ../../src/include/R_ext/libextern.h \
 ../../src/include/libintl.h dpq.h
