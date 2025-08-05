TESTS_DEB := lib-a-0-amd64 lib-a-0-i386 \
             lib-a-1-amd64 lib-a-1-i386 \
             lib-b-0-amd64 lib-b-0-i386 \
             lib-b-1-amd64 lib-b-1-i386 \
             pkg-c

include ../Test.mk

DPKG_OPTIONS += --auto-deconfigure

test-case:
	$(DPKG) --add-architecture i386
	$(DPKG_INSTALL) lib-a-0-amd64.deb lib-a-0-i386.deb
	$(DPKG_INSTALL) lib-b-0-amd64.deb lib-b-0-i386.deb
	$(DPKG_INSTALL) pkg-c.deb \
	                lib-a-1-amd64.deb lib-a-1-i386.deb \
	                lib-b-1-amd64.deb lib-b-1-i386.deb

test-clean:
	$(DPKG_PURGE) lib-a:amd64 lib-a:i386 lib-b:amd64 lib-b:i386 pkg-c
	$(DPKG) --remove-architecture i386
	rm -f $(DPKG_ADMINDIR)/arch
