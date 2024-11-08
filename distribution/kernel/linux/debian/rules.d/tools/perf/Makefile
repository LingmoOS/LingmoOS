include $(top_rulesdir)/Makefile.inc

# gas only understands the -fdebug-prefix-map option, and gcc doesn't
# convert the -ffile-prefix-map option to that when invoking gas.  So
# use both options.
ifneq ($(filter -ffile-prefix-map=%,$(CFLAGS)),)
CFLAGS += -fdebug-prefix-map=$(DEB_BUILD_PATH)=.
endif

MAKE_PERF := $(MAKE) O=$(CURDIR) prefix=/usr V=1 VF=1 ARCH=$(KERNEL_ARCH) WERROR=0 EXTRA_CFLAGS='$(CFLAGS) $(CPPFLAGS)' EXTRA_CXXFLAGS='$(CXXFLAGS) $(CPPFLAGS)' LDFLAGS='$(LDFLAGS)'

# Disable Gtk UI until it's more usable
MAKE_PERF += NO_GTK2=1

# Do not build the unversioned jvmti library
MAKE_PERF += feature-jvmti=0

MAKE_PERF += perfexecdir=lib/perf-core plugindir=/usr/lib/traceevent/plugins tipdir=share/doc/linux-perf perf_include_dir=include/perf perf_examples_dir=share/doc/linux-perf/examples

# perf can link against libcrypto if available, but the result is
# undistributable as GPL v2 and Apache 2.0 are not compatible without
# an explicit exception.  Override detection of libcrypto.
MAKE_PERF += NO_LIBCRYPTO=1

# perf only links against libopencsd (coresight) if specifically enabled
MAKE_PERF += CORESIGHT=1

# Currently babeltrace support for `perf data' is not automatically detected.
MAKE_PERF += LIBBABELTRACE=1

# Build with asciidoctor, not asciidoc
MAKE_PERF += USE_ASCIIDOCTOR=1

# Build with Python 3, not Python 2
MAKE_PERF += PYTHON=/usr/bin/python3

# Ensure the right options are picked up for libperl and libpython
export PERL5LIB=/usr/lib/$(DEB_HOST_MULTIARCH)/perl/cross-config
MAKE_PERF += PYTHON_CONFIG=$(CROSS_COMPILE)python3-config

# Explicitly set DEBUG=0.  This is the default for perf itself, but
# it's not passed down to libapi and it uses the wrong compiler
# options unless it's explicitly set.
MAKE_PERF += DEBUG=0

all:
# perf changes some default directories depending on whether DESTDIR is
# set.  We must define it even when building to avoid a rebuild when we
# run 'make install'.
	+$(MAKE_PERF) -C $(top_srcdir)/tools/perf -f Makefile.perf all DESTDIR=dummy
ifeq (,$(filter nodoc,$(DEB_BUILD_PROFILES)))
	+$(MAKE_PERF) -C $(top_srcdir)/tools/perf/Documentation man
endif
# Check that perf didn't get linked against incompatibly-licensed libraries
	@if readelf -d $(CURDIR)/perf | sed -rne 's/.*NEEDED.*\[(.*)\]/\1/p' | grep -E '\blib(bfd|crypto)'; then \
		echo; \
		echo 'perf linked against incompatibly-licensed libraries'; \
		echo; \
		exit 1; \
	fi
# Check that it links against abi::__cxa_demangle from libstdc++
	grep __cxa_demangle $(CURDIR)/perf

install:
	+$(MAKE_PERF) -C $(top_srcdir)/tools/perf -f Makefile.perf install install-python_ext
# Don't install a 'trace' alias yet:
# - We need a wrapper for it anyway, so there's little point adding a
#   versioned link
# - It doesn't work out-of-the-box as non-root (it depends on debugfs),
#   so it's less widely useful than strace
# - 'perf trace' doesn't take much more typing
	rm -f $(DESTDIR)/usr/bin/trace
	mkdir -p $(DESTDIR)/usr/share/bash-completion/
	mv $(DESTDIR)/etc/bash_completion.d \
		$(DESTDIR)/usr/share/bash-completion/completions
	rmdir --ignore-fail-on-non-empty $(DESTDIR)/etc
