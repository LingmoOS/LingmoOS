## Process this file with automake to produce Makefile.in

SUBDIRS = methods po

AM_CPPFLAGS = \
	-DLOCALEDIR=\"$(localedir)\" \
	-DADMINDIR=\"$(admindir)\" \
	-DLIBDIR=\"$(pkglibexecdir)\" \
	-DLOCALLIBDIR=\"/usr/local/lib/dpkg\" \
	-idirafter $(top_srcdir)/lib/compat \
	-iquote $(builddir) \
	-I$(top_builddir) \
	-I$(top_srcdir)/lib \
	# EOL
AM_CXXFLAGS = \
	-fno-rtti \
	-fno-exceptions \
	# EOL
if HAVE_LINKER_AS_NEEDED
AM_LDFLAGS = \
	-Wl,--as-needed \
	# EOL
endif


bin_PROGRAMS = dselect

dselect_SOURCES = \
	cxx-support.cc \
	dselect.h \
	dselect-curses.h \
	basecmds.cc \
	baselist.cc \
	basetop.cc \
	bindings.cc bindings.h \
	curkeys.cc \
	helpmsgs.cc helpmsgs.h \
	main.cc \
	methkeys.cc \
	methlist.cc \
	method.cc method.h \
	methparse.cc \
	pkgcmds.cc \
	pkgdepcon.cc \
	pkgdisplay.cc \
	pkginfo.cc \
	pkgkeys.cc \
	pkglist.cc pkglist.h \
	pkgsublist.cc \
	pkgtop.cc \
	# EOL

dselect_LDADD = \
	$(CURSES_LIBS) \
	../lib/dpkg/libdpkg.la \
	$(LIBINTL) \
	../lib/compat/libcompat.la \
	# EOL

dist_bashcompletions_DATA = \
	# EOL

EXTRA_DIST = \
	keyoverride \
	mkcurkeys.pl \
	# EOL

CLEANFILES = \
	curkeys.hpp \
	curkeys.h \
	# EOL

curkeys.$(OBJEXT): curkeys.h
curkeys.hpp: dselect-curses.h
	$(AM_V_GEN) echo '#include "dselect-curses.h"' | \
	  $(CPP) -dD $(CPPFLAGS) -I$(top_builddir) -I $(srcdir) - >$@
curkeys.h: $(srcdir)/keyoverride $(srcdir)/mkcurkeys.pl curkeys.hpp
	$(AM_V_GEN) $(PERL) $(srcdir)/mkcurkeys.pl $< curkeys.hpp >$@

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(pkgconfdir)/dselect.cfg.d
