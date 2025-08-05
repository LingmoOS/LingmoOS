TESTS_DEB := pkg-normal pkg-divert pkg-disappear

include ../Test.mk

test-case:
	# Test not disappearing diverted package.
	$(DPKG_INSTALL) pkg-disappear.deb
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_INSTALL) pkg-divert.deb
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	# Should not disappear while it's being diverted.
	$(call pkg_is_installed,pkg-disappear)
	$(DPKG_PURGE) pkg-divert
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_PURGE) pkg-disappear
	:
	# Test disappearing package not being diverted.
	$(DPKG_INSTALL) pkg-disappear.deb
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_INSTALL) pkg-normal.deb
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	# Should have been disappeared now.
	$(call pkg_is_not_installed,pkg-disappear)
	$(DPKG_PURGE) pkg-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_PURGE) pkg-disappear
	:
	# Test disappearing package diverted but replaced by a 3rd package.
	$(DPKG_INSTALL) pkg-disappear.deb
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_INSTALL) pkg-divert.deb
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_INSTALL) pkg-normal.deb
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	# Should have been disappeared now, as it is diverted by a 3rd package.
	$(call pkg_is_not_installed,pkg-disappear)
	$(DPKG_PURGE) pkg-normal
	test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_PURGE) pkg-divert
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal
	! test -e $(DPKG_INSTDIR)/test-dir/test-normal.distrib
	$(DPKG_PURGE) pkg-disappear

test-clean:
	$(DPKG_PURGE) pkg-divert
	$(DPKG_PURGE) pkg-disappear
	$(DPKG_PURGE) pkg-normal

