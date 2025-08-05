TESTS_DEB := pkg-dbg-dir

include ../Test.mk

test-case:
	$(DPKG_INSTALL) -D10 pkg-dbg-dir.deb

test-clean:
	$(DPKG_PURGE) pkg-dbg-dir

