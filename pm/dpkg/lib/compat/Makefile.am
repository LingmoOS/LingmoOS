## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-idirafter $(top_srcdir)/lib/compat \
	-I$(top_builddir) \
	# EOL


noinst_LTLIBRARIES = \
	libcompat-test.la \
	libcompat.la \
	# EOL

libcompat_test_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DTEST_LIBCOMPAT=1 \
	# EOL
libcompat_test_la_SOURCES = \
	compat.h \
	strchrnul.c \
	strnlen.c \
	strndup.c \
	strsignal.c \
	snprintf.c vsnprintf.c \
	asprintf.c vasprintf.c \
	getent.c \
	alphasort.c \
	scandir.c \
	unsetenv.c \
	# EOL

if HAVE_SYS_ERRLIST
libcompat_test_la_SOURCES += strerror.c
endif

libcompat_la_SOURCES = \
	empty.c \
	compat.h \
	compat-zlib.h \
	gettext.h \
	# EOL

if !HAVE_GETOPT
libcompat_la_SOURCES += getopt.c getopt.h getopt_int.h
else
if !HAVE_GETOPT_LONG
libcompat_la_SOURCES += getopt.c getopt.h getopt_int.h
endif
endif

if !HAVE_GETOPT_LONG
libcompat_la_SOURCES += getopt1.c
endif

if !HAVE_OBSTACK_FREE
libcompat_la_SOURCES += obstack.c obstack.h
endif

if !HAVE_STRNLEN
libcompat_la_SOURCES += strnlen.c
endif

if !HAVE_STRCHRNUL
libcompat_la_SOURCES += strchrnul.c
endif

if !HAVE_STRNDUP
libcompat_la_SOURCES += strndup.c
endif

if !HAVE_STRERROR
libcompat_la_SOURCES += strerror.c
endif

if !HAVE_STRSIGNAL
libcompat_la_SOURCES += strsignal.c
endif

if !HAVE_C99_SNPRINTF
libcompat_la_SOURCES += snprintf.c vsnprintf.c
endif

if !HAVE_ASPRINTF
libcompat_la_SOURCES += asprintf.c vasprintf.c
endif

if !HAVE_FGETPWENT
libcompat_la_SOURCES += getent.c
else
if !HAVE_FGETGRENT
libcompat_la_SOURCES += getent.c
endif
endif

if !HAVE_ALPHASORT
libcompat_la_SOURCES += alphasort.c
endif

if !HAVE_SCANDIR
libcompat_la_SOURCES += scandir.c
endif

if !HAVE_UNSETENV
libcompat_la_SOURCES += unsetenv.c
endif
