TESTS_DEB := pkg-normal pkg-depend pkg-disappear

include ../Test.mk

test-case:
	# Test disappearing depended package.
	$(DPKG_INSTALL) pkg-disappear.deb
	$(DPKG_INSTALL) pkg-depend.deb
	$(DPKG_INSTALL) pkg-normal.deb
	# Should not disappear while it is being depended on.
	$(call pkg_is_installed,pkg-disappear)
	$(DPKG_PURGE) pkg-depend
	$(DPKG_PURGE) pkg-normal
	:
	# Should disappear now that it is not being depended on.
	$(DPKG_INSTALL) pkg-disappear.deb
	$(DPKG_INSTALL) pkg-normal.deb
	# Should have been disappeared now.
	$(call pkg_is_not_installed,pkg-disappear)

test-clean:
	-$(DPKG_PURGE) pkg-depend
	-$(DPKG_PURGE) pkg-disappear
	$(DPKG_PURGE) pkg-normal
