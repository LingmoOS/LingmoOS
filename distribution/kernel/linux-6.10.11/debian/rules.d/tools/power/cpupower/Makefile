include $(top_rulesdir)/Makefile.inc

MAKE_CPUPOWER := CFLAGS='$(CFLAGS) $(CPPFLAGS)' LDFLAGS='$(LDFLAGS)' $(MAKE) O=$(CURDIR) CPUFREQ_BENCH=false V=true mandir=/usr/share/man

MAKE_CPUPOWER += DEBUG=$(if $(filter noopt,$(DEB_BUILD_OPTIONS)),true,)

# Don't strip binaries here; let dh_strip determine what to do
MAKE_CPUPOWER += STRIP=true

MAKE_CPUPOWER += CROSS='$(CROSS_COMPILE)'

MAKE_CPUPOWER += PACKAGE_BUGREPORT='Debian\ \(reportbug\ linux-cpupower\)'

MAKE_CPUPOWER += libdir=/usr/lib/$(DEB_HOST_MULTIARCH)

all:
	+$(MAKE_CPUPOWER) -C $(top_srcdir)/tools/power/cpupower

install:
	+$(MAKE_CPUPOWER) -C $(top_srcdir)/tools/power/cpupower install DESTDIR=$(DESTDIR)
ifneq (,$(filter nodoc,$(DEB_BUILD_PROFILES)))
	rm -rf $(DESTDIR)/usr/share/man
endif

clean:
	+$(MAKE_CPUPOWER) -C $(top_srcdir)/tools/power/cpupower clean
