TESTS_DEB := pkg-normal

include ../Test.mk

test-case:
	# Make sure the package is not there
	$(DPKG_PURGE) pkg-normal
	
	# Simulate unpacking a package
	$(DPKG_UNPACK) --dry-run pkg-normal.deb
	$(call pkg_is_not_installed,pkg-normal)
	$(DPKG_INSTALL) --dry-run pkg-normal.deb
	$(call pkg_is_not_installed,pkg-normal)
	
	$(DPKG_UNPACK) pkg-normal.deb
	
	# Simulate configuring a package
	$(DPKG_CONFIGURE) --dry-run pkg-normal
	$(call pkg_status_is,pkg-normal,install ok unpacked)
	
	$(DPKG_CONFIGURE) pkg-normal
	
	# Simulate removing and purging a package
	$(DPKG_REMOVE) --dry-run pkg-normal
	$(call pkg_is_installed,pkg-normal)
	$(DPKG_PURGE) --dry-run pkg-normal
	$(call pkg_is_installed,pkg-normal)
	
	# XXX: Complete

test-clean:
	$(DPKG_PURGE) pkg-normal
