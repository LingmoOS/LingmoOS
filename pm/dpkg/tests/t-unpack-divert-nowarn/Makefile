TESTS_DEB := pkg-files pkg-divert-unpacking

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-files.deb
	$(DPKG_UNPACK) pkg-divert-unpacking.deb

test-clean:
	$(DPKG_PURGE) pkg-files pkg-divert-unpacking
