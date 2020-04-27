#
# ${R_HOME}/src/extra/intl/Makefile


srcdir = .
top_srcdir = ../../..

top_builddir = ../../..
subdir = src/extra/intl

include $(top_builddir)/Makeconf

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)
DISTFILES = Makefile.in Makefile.win R_changes VERSION \
  config.charset $(SOURCES) $(HEADERS)

HEADERS = \
  gmo.h gettextP.h hash-string.h loadinfo.h plural-exp.h eval-plural.h \
  localcharset.h xsize.h printf-args.h printf-args.c printf-parse.h \
  wprintf-parse.h printf-parse.c vasnprintf.h vasnwprintf.h vasnprintf.c \
  libgnuintl.h.in lock.h intl-exports.c tsearch.c tsearch.h

SOURCES = \
 bindtextdom.c dcgettext.c dgettext.c gettext.c finddomain.c loadmsgcat.c \
 textdomain.c l10nflist.c explodename.c dcigettext.c \
 dcngettext.c dngettext.c ngettext.c plural.c plural-exp.c langprefs.c \
 localcharset.c localename.c printf.c osdep.c intl-compat.c \
 hash-string.c lock.c

DEPENDS = $(SOURCES:.c=.d)
OBJECTS = $(SOURCES:.c=.o)
#ALL_CFLAGS = $(R_XTRA_CFLAGS) $(CPICFLAGS) $(SHLIB_CFLAGS) $(CFLAGS) 

libintl_a_SOURCES = $(SOURCES)
libintl_a_OBJECTS = $(OBJECTS)
noinst_LIBRARIES = libintl.a

# LOCALEDIR is the default location for catalogs in dcigettext.c,
# and unused in R.
DEFS=-DLOCALEDIR=\"\" -DLOCALEALIAS_PATH=\"\" -DIN_LIBINTL -DHAVE_CONFIG_H

ALL_CPPFLAGS=$(R_XTRA_CPPFLAGS) -I$(srcdir) $(CPPFLAGS) $(DEFS) -I/System/Library/Frameworks/CoreFoundation.framework/Headers

R all: all-yes
all-yes: Makefile 
	@$(MAKE) headers charsetalias.h
	@$(MAKE) Makedeps
	@$(MAKE) $(noinst_LIBRARIES)
all-no: Makefile

Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	@cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

Makedeps: Makefile $(DEPENDS)
	@cat $(DEPENDS) >> Makefile
	@touch $@

libintl.a: $(libintl_a_OBJECTS)
	@rm -f $@
	$(AR) -cr $@ $(libintl_a_OBJECTS)
	$(RANLIB) $@

headers: libgnuintl.h
	@$(SHELL) $(top_srcdir)/tools/copy-if-change libgnuintl.h \
	  $(top_builddir)/src/include/libintl.h
	@$(SHELL) $(top_srcdir)/tools/copy-if-change libgnuintl.h \
	 $(top_builddir)/include/libintl.h

libgnuintl.h: $(srcdir)/libgnuintl.h.in
	@sed -e 's,@''HAVE_POSIX_PRINTF''@,1,g' \
	    -e 's,@''HAVE_ASPRINTF''@,1,g' \
	    -e 's,@''HAVE_SNPRINTF''@,1,g' \
	    -e 's,@''HAVE_WPRINTF''@,0,g' \
	  < $(srcdir)/libgnuintl.h.in > libgnuintl.h

charset.alias: $(srcdir)/config.charset
	@$(SHELL) $(srcdir)/config.charset 'x86_64-apple-darwin19.4.0' > t-$@
	@$(SHELL) $(top_srcdir)/tools/copy-if-change t-$@ $@
	@rm t-$@

charsetalias.h: charset.alias
	@$(ECHO) "cp =" > $@
	@ /usr/bin/sed -e 's/\([^ ]\{1,\}\)/"\1" "\\0"/g' charset.alias >> $@
	@$(ECHO) ";" >> $@

mostlyclean: clean
clean:
	@-rm -f Makedeps *.d *.o *.lo *.a
	@rm -f libgnuintl.h libintl.h charset.alias charsetalias.h ref-add.sed ref-del.sed
distclean: clean
	@-rm -f Makefile
maintainer-clean: distclean

install install-strip uninstall TAGS info dvi check:

distdir: $(DISTFILES)
	@for f in $(DISTFILES); do \
	  test -f $(distdir)/$${f} \
	    || ln $(srcdir)/$${f} $(distdir)/$${f} 2>/dev/null \
	    || cp -p $(srcdir)/$${f} $(distdir)/$${f}; \
	done
bindtextdom.o: bindtextdom.c ../../../src/include/config.h gettextP.h \
  lock.h libgnuintl.h loadinfo.h gmo.h
dcgettext.o: dcgettext.c ../../../src/include/config.h gettextP.h lock.h \
  libgnuintl.h loadinfo.h gmo.h
dgettext.o: dgettext.c ../../../src/include/config.h gettextP.h lock.h \
  libgnuintl.h loadinfo.h gmo.h
gettext.o: gettext.c ../../../src/include/config.h gettextP.h lock.h \
  libgnuintl.h loadinfo.h gmo.h
finddomain.o: finddomain.c ../../../src/include/config.h gettextP.h \
  lock.h libgnuintl.h loadinfo.h gmo.h
loadmsgcat.o: loadmsgcat.c ../../../src/include/config.h gmo.h gettextP.h \
  lock.h libgnuintl.h loadinfo.h hash-string.h plural-exp.h
textdomain.o: textdomain.c ../../../src/include/config.h gettextP.h \
  lock.h libgnuintl.h loadinfo.h gmo.h
l10nflist.o: l10nflist.c ../../../src/include/config.h loadinfo.h
explodename.o: explodename.c ../../../src/include/config.h loadinfo.h
dcigettext.o: dcigettext.c ../../../src/include/config.h localcharset.h \
  gettextP.h lock.h libgnuintl.h loadinfo.h gmo.h plural-exp.h \
  hash-string.h eval-plural.h
dcngettext.o: dcngettext.c ../../../src/include/config.h gettextP.h \
  lock.h libgnuintl.h loadinfo.h gmo.h
dngettext.o: dngettext.c ../../../src/include/config.h gettextP.h lock.h \
  libgnuintl.h loadinfo.h gmo.h
ngettext.o: ngettext.c ../../../src/include/config.h gettextP.h lock.h \
  libgnuintl.h loadinfo.h gmo.h
plural.o: plural.c ../../../src/include/config.h plural-exp.h
plural-exp.o: plural-exp.c ../../../src/include/config.h plural-exp.h
langprefs.o: langprefs.c ../../../src/include/config.h
localcharset.o: localcharset.c ../../../src/include/config.h \
  localcharset.h
localename.o: localename.c ../../../src/include/config.h gettextP.h \
  lock.h libgnuintl.h loadinfo.h gmo.h
printf.o: printf.c ../../../src/include/config.h
osdep.o: osdep.c
intl-compat.o: intl-compat.c ../../../src/include/config.h gettextP.h \
  lock.h libgnuintl.h loadinfo.h gmo.h
hash-string.o: hash-string.c ../../../src/include/config.h hash-string.h
lock.o: lock.c ../../../src/include/config.h lock.h
