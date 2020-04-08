#
# ${R_HOME}/src/library/utils/src/Makefile

pkg = utils


srcdir = .
top_srcdir = ../../../..

top_builddir = ../../../..
subdir = src/library/$(pkg)/src
R_HOME = $(top_builddir)
R_SHARE_DIR = $(R_HOME)/share
R_INCLUDE_DIR = $(R_HOME)/include

SOURCES_C = init.c io.c size.c sock.c stubs.c utils.c

DEPENDS = $(SOURCES_C:.c=.d)
OBJECTS = $(SOURCES_C:.c=.o)

PKG_CFLAGS = $(C_VISIBILITY)

SHLIB = $(pkg).so

# need Defn.h etc, and config.h
PKG_CPPFLAGS =-I../../../include -I$(top_srcdir)/src/include -DHAVE_CONFIG_H \
  -I$(top_srcdir)/src/main


all: Makedeps
	@$(MAKE) shlib

Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	@cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

Makedeps: Makefile $(DEPENDS)
	@cat $(DEPENDS) >> Makefile
	@touch $@

shlib: $(SHLIB)
	@$(MKINSTALLDIRS) \
	  $(top_builddir)/library/$(pkg)/libs
	@$(SHELL) $(top_srcdir)/tools/copy-if-change $(SHLIB) \
	  $(top_builddir)/library/$(pkg)/libs/$(SHLIB)

include $(R_HOME)/etc${R_ARCH}/Makeconf
include $(top_srcdir)/share/make/shlib.mk
LTO = 

mostlyclean: clean
clean:
	@-rm -Rf .libs _libs
	@-rm -f Makedeps *.d *.o *$(SHLIB_EXT)
distclean: clean
	@-rm -f Makefile
maintainer-clean: distclean

## Automagically generated dependencies:
init.o: init.c ../../../include/config.h ../../../../include/R.h \
  ../../../../include/Rconfig.h ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h \
  utils.h ../../../../include/libintl.h \
  ../../../../include/R_ext/Visibility.h
io.o: io.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h ../../../include/Fileio.h \
  ../../../include/Rconnections.h \
  ../../../../include/R_ext/Connections.h ../../../include/Print.h \
  ../../../../include/R_ext/PrtUtil.h ../../../include/rlocale.h \
  ../../../../src/main/RBufferUtils.h
size.o: size.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h
sock.o: sock.c ../../../../include/R.h ../../../../include/Rconfig.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h
stubs.o: stubs.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h ../../../include/Internal.h \
  ../../../../include/Rinterface.h ../../../../include/R_ext/RStartup.h \
  ../../../include/Rdynpriv.h ../../../include/Rmodules/RX11.h \
  ../../../include/Rconnections.h \
  ../../../../include/R_ext/Connections.h
utils.o: utils.c ../../../include/config.h ../../../../include/R.h \
  ../../../../include/Rconfig.h ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h \
  utils.h ../../../../include/libintl.h /usr/local/include/lzma.h \
  /usr/local/include/lzma/version.h /usr/local/include/lzma/base.h \
  /usr/local/include/lzma/vli.h /usr/local/include/lzma/check.h \
  /usr/local/include/lzma/filter.h /usr/local/include/lzma/bcj.h \
  /usr/local/include/lzma/delta.h /usr/local/include/lzma/lzma12.h \
  /usr/local/include/lzma/container.h \
  /usr/local/include/lzma/stream_flags.h /usr/local/include/lzma/block.h \
  /usr/local/include/lzma/index.h /usr/local/include/lzma/index_hash.h \
  /usr/local/include/lzma/hardware.h
init.o: init.c ../../../include/config.h ../../../../include/R.h \
  ../../../../include/Rconfig.h ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h \
  utils.h ../../../../include/libintl.h \
  ../../../../include/R_ext/Visibility.h
io.o: io.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h ../../../include/Fileio.h \
  ../../../include/Rconnections.h \
  ../../../../include/R_ext/Connections.h ../../../include/Print.h \
  ../../../../include/R_ext/PrtUtil.h ../../../include/rlocale.h \
  ../../../../src/main/RBufferUtils.h
size.o: size.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h
sock.o: sock.c ../../../../include/R.h ../../../../include/Rconfig.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h
stubs.o: stubs.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h ../../../include/Internal.h \
  ../../../../include/Rinterface.h ../../../../include/R_ext/RStartup.h \
  ../../../include/Rdynpriv.h ../../../include/Rmodules/RX11.h \
  ../../../include/Rconnections.h \
  ../../../../include/R_ext/Connections.h
utils.o: utils.c ../../../include/config.h ../../../../include/R.h \
  ../../../../include/Rconfig.h ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h \
  utils.h ../../../../include/libintl.h /usr/local/include/lzma.h \
  /usr/local/include/lzma/version.h /usr/local/include/lzma/base.h \
  /usr/local/include/lzma/vli.h /usr/local/include/lzma/check.h \
  /usr/local/include/lzma/filter.h /usr/local/include/lzma/bcj.h \
  /usr/local/include/lzma/delta.h /usr/local/include/lzma/lzma12.h \
  /usr/local/include/lzma/container.h \
  /usr/local/include/lzma/stream_flags.h /usr/local/include/lzma/block.h \
  /usr/local/include/lzma/index.h /usr/local/include/lzma/index_hash.h \
  /usr/local/include/lzma/hardware.h
init.o: init.c ../../../include/config.h ../../../../include/R.h \
  ../../../../include/Rconfig.h ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h \
  utils.h ../../../../include/libintl.h \
  ../../../../include/R_ext/Visibility.h
io.o: io.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h ../../../include/Fileio.h \
  ../../../include/Rconnections.h \
  ../../../../include/R_ext/Connections.h ../../../include/Print.h \
  ../../../../include/R_ext/PrtUtil.h ../../../include/rlocale.h \
  ../../../../src/main/RBufferUtils.h
size.o: size.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h
sock.o: sock.c ../../../../include/R.h ../../../../include/Rconfig.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h
stubs.o: stubs.c ../../../include/config.h ../../../include/Defn.h \
  ../../../../include/R_ext/Complex.h ../../../../include/Rinternals.h \
  ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h ../../../../include/R_ext/Error.h \
  ../../../../include/R_ext/Memory.h ../../../../include/R_ext/Utils.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Rdynload.h \
  ../../../include/Rinlinedfuns.h ../../../include/Errormsg.h \
  ../../../../include/libintl.h ../../../include/Internal.h \
  ../../../../include/Rinterface.h ../../../../include/R_ext/RStartup.h \
  ../../../include/Rdynpriv.h ../../../include/Rmodules/RX11.h \
  ../../../include/Rconnections.h \
  ../../../../include/R_ext/Connections.h
utils.o: utils.c ../../../include/config.h ../../../../include/R.h \
  ../../../../include/Rconfig.h ../../../../include/R_ext/Arith.h \
  ../../../../include/R_ext/libextern.h \
  ../../../../include/R_ext/Boolean.h \
  ../../../../include/R_ext/Complex.h \
  ../../../../include/R_ext/Constants.h \
  ../../../../include/R_ext/Error.h ../../../../include/R_ext/Memory.h \
  ../../../../include/R_ext/Print.h ../../../../include/R_ext/Random.h \
  ../../../../include/R_ext/Utils.h ../../../../include/R_ext/RS.h \
  ../../../../include/Rinternals.h ../../../../include/R_ext/Rdynload.h \
  utils.h ../../../../include/libintl.h /usr/local/include/lzma.h \
  /usr/local/include/lzma/version.h /usr/local/include/lzma/base.h \
  /usr/local/include/lzma/vli.h /usr/local/include/lzma/check.h \
  /usr/local/include/lzma/filter.h /usr/local/include/lzma/bcj.h \
  /usr/local/include/lzma/delta.h /usr/local/include/lzma/lzma12.h \
  /usr/local/include/lzma/container.h \
  /usr/local/include/lzma/stream_flags.h /usr/local/include/lzma/block.h \
  /usr/local/include/lzma/index.h /usr/local/include/lzma/index_hash.h \
  /usr/local/include/lzma/hardware.h
