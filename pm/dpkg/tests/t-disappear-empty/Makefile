TESTS_DEB := pkg-normal pkg-disappear-0 pkg-disappear-1

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-disappear-0.deb
	# test that an upgraded empty package does not get disappeared
	$(DPKG_INSTALL) pkg-disappear-1.deb
	$(call pkg_is_installed,pkg-disappear)
	$(DPKG_INSTALL) pkg-normal.deb
	$(call pkg_is_installed,pkg-normal)

test-clean:
	$(DPKG_PURGE) pkg-disappear
	$(DPKG_PURGE) pkg-normal

