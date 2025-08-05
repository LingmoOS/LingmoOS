## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DLOCALEDIR=\"$(localedir)\" \
	-DCONFIGDIR=\"$(pkgconfdir)\" \
	-DADMINDIR=\"$(admindir)\" \
	-DDEFAULT_TEXT_DOMAIN=\"@PACKAGE@\" \
	-idirafter $(top_srcdir)/lib/compat \
	-I$(top_builddir) \
	-I$(top_srcdir)/lib \
	# EOL

# Needed for the test suite
LDADD = \
	libdpkg.la \
	$(LIBINTL) \
	../compat/libcompat.la \
	# EOL


EXTRA_DIST =
DISTCLEANFILES =

pkgconfig_DATA = libdpkg.pc

devlib_LTLIBRARIES = libdpkg.la

EXTRA_libdpkg_la_DEPENDENCIES = \
	libdpkg.map \
	# EOL

libdpkg_la_LDFLAGS = \
	-no-undefined \
	# EOL

if HAVE_LINKER_VERSION_SCRIPT
libdpkg_la_LDFLAGS += \
	-Wl,--version-script=$(srcdir)/libdpkg.map \
	# EOL
else
libdpkg_la_LDFLAGS += \
	-export-symbols libdpkg.sym \
	# EOL

EXTRA_libdpkg_la_DEPENDENCIES += \
	libdpkg.sym \
	# EOL
endif

libdpkg_la_LIBADD = \
	$(PS_LIBS) \
	$(Z_LIBS) \
	$(LZMA_LIBS) \
	$(ZSTD_LIBS) \
	$(BZ2_LIBS) \
	$(MD_LIBS) \
	$(LIBINTL) \
	../compat/libcompat.la \
	# EOL

libdpkg_la_SOURCES = \
	dlist.h \
	ar.c \
	arch.c \
	atomic-file.c \
	buffer.c \
	c-ctype.c \
	cleanup.c \
	color.c \
	command.c \
	compress.c \
	dbdir.c \
	dbmodify.c \
	db-ctrl-access.c \
	db-ctrl-format.c \
	db-ctrl-upgrade.c \
	db-fsys-digest.c \
	db-fsys-divert.c \
	db-fsys-files.c \
	db-fsys-load.c \
	db-fsys-override.c \
	deb-version.c \
	debug.c \
	depcon.c \
	dir.c \
	dump.c \
	ehandle.c \
	error.c \
	execname.c \
	fdio.c \
	file.c \
	fields.c \
	fsys-dir.c \
	fsys-iter.c \
	fsys-hash.c \
	glob.c \
	i18n.c i18n.h \
	log.c \
	meminfo.c \
	mustlib.c \
	namevalue.c \
	nfmalloc.c \
	options.c \
	options-dirs.c \
	options-parsers.c \
	pager.c \
	parse.c \
	parsehelp.c \
	path.c \
	path-remove.c \
	perf.h \
	pkg.c \
	pkg-array.c \
	pkg-files.c \
	pkg-format.c \
	pkg-hash.c \
	pkg-list.c \
	pkg-namevalue.c \
	pkg-queue.c \
	pkg-show.c \
	pkg-spec.c \
	progname.c \
	program.c \
	progress.c \
	report.c \
	string.c \
	strhash.c \
	strvec.c \
	strwide.c \
	subproc.c \
	sysuser.c \
	tarfn.c \
	test.h \
	treewalk.c \
	trigname.c \
	trignote.c \
	triglib.c \
	trigdeferred.c \
	utils.c \
	varbuf.c \
	version.c \
	# EOL

pkginclude_HEADERS = \
	ar.h \
	arch.h \
	atomic-file.h \
	buffer.h \
	c-ctype.h \
	color.h \
	command.h \
	compress.h \
	db-ctrl.h \
	db-fsys.h \
	deb-version.h \
	debug.h \
	dir.h \
	dpkg.h \
	dpkg-db.h \
	ehandle.h \
	error.h \
	execname.h \
	fdio.h \
	file.h \
	fsys.h \
	glob.h \
	macros.h \
	meminfo.h \
	namevalue.h \
	options.h \
	pager.h \
	parsedump.h \
	path.h \
	pkg.h \
	pkg-array.h \
	pkg-files.h \
	pkg-format.h \
	pkg-list.h \
	pkg-queue.h \
	pkg-show.h \
	pkg-spec.h \
	progname.h \
	program.h \
	progress.h \
	report.h \
	string.h \
	strvec.h \
	subproc.h \
	sysuser.h \
	tarfn.h \
	treewalk.h \
	trigdeferred.h \
	triglib.h \
	varbuf.h \
	version.h \
	# EOL

EXTRA_DIST += \
	libdpkg.map \
	libdpkg.pc.in \
	# EOL

DISTCLEANFILES += \
	libdpkg.sym \
	# EOL

# Generate a simple libtool symbol export list to be used as a fallback if
# there is no version script support.
libdpkg.sym: libdpkg.map
	$(AM_V_GEN) $(SED) -n \
	  -e 's/#.*$$//g' \
	  -e 's/^[[:space:]]\{1,\}\([A-Za-z0-9_]\{1,\}\);/\1/p' \
	  $(srcdir)/libdpkg.map \
	  >$@

t_t_headers_cpp_SOURCES = \
	t/t-headers-cpp.cc \
	# EOL

t_t_varbuf_cpp_SOURCES = \
	t/t-varbuf-cpp.cc \
	# EOL

# The tests are sorted in order of increasing complexity.
test_programs = \
	t/t-test \
	t/t-test-skip \
	t/t-macros \
	t/t-compat-getent \
	t/t-headers-cpp \
	t/t-c-ctype \
	t/t-namevalue \
	t/t-ehandle \
	t/t-error \
	t/t-string \
	t/t-strvec \
	t/t-sysuser \
	t/t-file \
	t/t-buffer \
	t/t-meminfo \
	t/t-path \
	t/t-progname \
	t/t-subproc \
	t/t-command \
	t/t-pager \
	t/t-varbuf \
	t/t-varbuf-cpp \
	t/t-ar \
	t/t-tar \
	t/t-deb-version \
	t/t-arch \
	t/t-version \
	t/t-pkginfo \
	t/t-pkg-list \
	t/t-pkg-queue \
	t/t-pkg-hash \
	t/t-pkg-show \
	t/t-pkg-format \
	t/t-fsys-dir \
	t/t-fsys-hash \
	t/t-trigger \
	t/t-mod-db \
	# EOL

test_scripts = \
	t/t-tarextract.t \
	t/t-treewalk.t \
	t/t-trigdeferred.t \
	# EOL

test_data = \
	t/data/command/path-a/cmd \
	t/data/command/path-a/cmd-a \
	t/data/command/path-b/cmd \
	t/data/command/path-b/cmd-b \
	t/data/command/path-noexec/cmd \
	t/data/command/path-noexec/cmd-noexec \
	t/data/meminfo-no-data \
	t/data/meminfo-no-info \
	t/data/meminfo-no-unit \
	t/data/meminfo-ok \
	t/data/sysuser/group.base \
	t/data/sysuser/group.nis \
	t/data/sysuser/passwd.base \
	t/data/sysuser/passwd.nis \
	# EOL

EXTRA_DIST += \
	$(test_scripts) \
	$(test_data) \
	# EOL

BENCHMARK_LDADD_FLAGS = \
	$(RT_LIBS) \
	$(LDADD) \
	# EOL

LIBCOMPAT_TEST_LDADD_FLAGS = \
	$(LDADD) \
	../compat/libcompat-test.la \
	# EOL

t_b_fsys_hash_LDADD = $(BENCHMARK_LDADD_FLAGS)
t_b_pkg_hash_LDADD = $(BENCHMARK_LDADD_FLAGS)
t_t_compat_getent_LDADD = $(LIBCOMPAT_TEST_LDADD_FLAGS)

check_PROGRAMS = \
	$(test_programs) \
	t/b-fsys-hash \
	t/b-pkg-hash \
	t/c-tarextract \
	t/c-treewalk \
	t/c-trigdeferred \
	# EOL

TEST_ENV_VARS = \
	DPKG_PROGTAR=$(TAR) \
	# EOL

test_tmpdir = t.tmp

include $(top_srcdir)/build-aux/tap.am

check-local: tap-check

clean-local: tap-clean
