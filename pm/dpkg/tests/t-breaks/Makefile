TESTS_DEB := lib-a-0 lib-a-1 lib-b-0 lib-b-1 pkg-c

include ../Test.mk

DPKG_OPTIONS += --auto-deconfigure

test-case:
	$(DPKG_INSTALL) lib-a-0.deb lib-b-0.deb
	$(DPKG_INSTALL) pkg-c.deb lib-a-1.deb lib-b-1.deb

test-clean:
	$(DPKG_PURGE) lib-a lib-b pkg-c
