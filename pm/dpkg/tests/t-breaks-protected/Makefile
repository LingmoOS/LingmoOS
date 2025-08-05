TESTS_DEB := lib-a-0 lib-a-1 pkg-b

include ../Test.mk

DPKG_OPTIONS += --auto-deconfigure

test-case:
	$(DPKG_INSTALL) lib-a-0.deb
	$(DPKG_INSTALL) pkg-b.deb lib-a-1.deb

test-clean:
	$(DPKG_PURGE) pkg-b
	$(DPKG_PURGE) --force-remove-protected lib-a
