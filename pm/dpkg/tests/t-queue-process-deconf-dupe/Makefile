TESTS_DEB := pkg-a-0 pkg-a-1 pkg-b

include ../Test.mk

DPKG_OPTIONS += --auto-deconfigure

test-case:
	$(DPKG_INSTALL) pkg-a-0.deb
	$(DPKG_INSTALL) pkg-b.deb pkg-a-1.deb

test-clean:
	$(DPKG_PURGE) pkg-b pkg-a
