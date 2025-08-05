TESTS_DEB := pkg-files pkg-divert pkg-replaces

include ../Test.mk

test-case:
	# Test failure to install a package overwriting a diversion (order 1).
	$(DPKG_INSTALL) pkg-divert.deb
	test -e $(DPKG_INSTDIR)/test-file
	! test -e $(DPKG_INSTDIR)/test-other
	$(DPKG_INSTALL) pkg-files.deb
	test -e $(DPKG_INSTDIR)/test-file
	test -e $(DPKG_INSTDIR)/test-other
	! $(DPKG_INSTALL) pkg-replaces.deb
	test -e $(DPKG_INSTDIR)/test-file
	test -e $(DPKG_INSTDIR)/test-other
	$(call pkg_is_installed,pkg-divert)
	$(call pkg_is_not_installed,pkg-replaces)
	$(DPKG_PURGE) pkg-files
	$(DPKG_PURGE) pkg-divert
	$(DPKG_PURGE) pkg-replaces
	! test -e $(DPKG_INSTDIR)/test-file
	! test -e $(DPKG_INSTDIR)/test-other
	:
	# Test failure to install a package overwriting a diversion (order 2).
	$(DPKG_INSTALL) pkg-files.deb
	test -e $(DPKG_INSTDIR)/test-file
	! test -e $(DPKG_INSTDIR)/test-other
	$(DPKG_INSTALL) pkg-divert.deb
	test -e $(DPKG_INSTDIR)/test-file
	test -e $(DPKG_INSTDIR)/test-other
	! $(DPKG_INSTALL) pkg-replaces.deb
	test -e $(DPKG_INSTDIR)/test-file
	test -e $(DPKG_INSTDIR)/test-other
	$(call pkg_is_installed,pkg-divert)
	$(call pkg_is_not_installed,pkg-replaces)
	$(DPKG_PURGE) pkg-files
	$(DPKG_PURGE) pkg-divert
	$(DPKG_PURGE) pkg-replaces
	! test -e $(DPKG_INSTDIR)/test-file
	! test -e $(DPKG_INSTDIR)/test-other
	:
	# Test success to install a package overwriting a diversion (forced).
	$(DPKG_INSTALL) pkg-files.deb
	test -e $(DPKG_INSTDIR)/test-file
	! test -e $(DPKG_INSTDIR)/test-other
	$(DPKG_INSTALL) pkg-divert.deb
	test -e $(DPKG_INSTDIR)/test-file
	test -e $(DPKG_INSTDIR)/test-other
	$(DPKG_INSTALL) --force-overwrite-diverted pkg-replaces.deb
	test -e $(DPKG_INSTDIR)/test-file
	test -e $(DPKG_INSTDIR)/test-other
	$(call pkg_is_installed,pkg-divert)
	$(call pkg_is_installed,pkg-replaces)
	$(DPKG_PURGE) pkg-replaces
	$(DPKG_PURGE) pkg-divert
	$(DPKG_PURGE) pkg-files
	! test -e $(DPKG_INSTDIR)/test-file
	! test -e $(DPKG_INSTDIR)/test-other

	# XXX: In the force case, if we purge pkg-replaces last, then it
	# will leave behind on the fsys an unowned file.

test-clean:
	$(DPKG_PURGE) pkg-files pkg-divert pkg-replaces
