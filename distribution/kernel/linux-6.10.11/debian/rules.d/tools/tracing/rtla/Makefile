include $(top_rulesdir)/Makefile.inc

all:
# Out-of-tree build is not supported, so copy source to the build dir
	echo '$(UPSTREAMVERSION)' >VERSION
	rsync -a $(top_srcdir)/tools/tracing/rtla/ .
	mkdir -p ../../../Documentation/tools/rtla
	rsync -a $(top_srcdir)/Documentation/tools/rtla/ ../../../Documentation/tools/rtla/
	rsync -a $(top_srcdir)/tools/build/ ../../../tools/build/
	rsync -a $(top_srcdir)/tools/scripts/ ../../../tools/scripts/
	$(MAKE) EXTRA_CFLAGS='$(CFLAGS) $(CPPFLAGS)' EXTRA_LDFLAGS='$(LDFLAGS)' PKG_CONFIG='$(PKG_CONFIG)'

install:
	$(MAKE) install
ifneq (,$(filter nodoc,$(DEB_BUILD_PROFILES)))
	rm -rf $(DESTDIR)/usr/share/man
endif

clean:
	rm -rf *
