TESTS_DEB := pkg-normal pkg-disappear-0 pkg-disappear-1

include ../Test.mk

test-case:
	# test disappearing package
	$(DPKG_INSTALL) pkg-disappear-0.deb
	$(DPKG_INSTALL) pkg-disappear-1.deb
	$(DPKG_INSTALL) pkg-normal.deb

test-clean:
	-$(DPKG_PURGE) pkg-disappear
	$(DPKG_PURGE) pkg-normal

