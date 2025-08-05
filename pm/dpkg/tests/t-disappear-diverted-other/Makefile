TESTS_DEB := pkg-divert pkg-diverted pkg-replaces

include ../Test.mk

test-case:
	# Test disappearing diverted package with a not contended pathname.
	$(DPKG_INSTALL) pkg-divert.deb
	$(DPKG_INSTALL) pkg-diverted.deb
	$(DPKG_INSTALL) pkg-replaces.deb
	# Should disappear because there is no contended pathname.
	$(call pkg_is_not_installed,pkg-divert)
	$(DPKG_PURGE) pkg-divert
	$(DPKG_PURGE) pkg-diverted
	$(DPKG_PURGE) pkg-replaces
	:
	# Test disappearing diverted package with a not contended pathname.
	$(DPKG_INSTALL) pkg-divert.deb
	$(DPKG_INSTALL) pkg-replaces.deb
	# Should disappear because there is no contended pathname.
	$(call pkg_is_not_installed,pkg-divert)
	$(DPKG_PURGE) pkg-divert
	$(DPKG_PURGE) pkg-replaces

test-clean:
	$(DPKG_PURGE) pkg-divert
	$(DPKG_PURGE) pkg-diverted
	$(DPKG_PURGE) pkg-replaces

