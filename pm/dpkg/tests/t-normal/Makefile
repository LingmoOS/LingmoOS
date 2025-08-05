TESTS_DEB := pkg-normal

include ../Test.mk

test-case:
	# test normal working packages
	$(DPKG_INSTALL) pkg-normal.deb
	$(call pkg_is_installed,pkg-normal)

test-clean:
	$(DPKG_PURGE) pkg-normal

