TESTS_DEB := pkg-conff-obsolete-0 pkg-conff-obsolete-1 pkg-conff-obsolete-2

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-conff-obsolete-0.deb
	$(DPKG_INSTALL) pkg-conff-obsolete-1.deb
	test -f '$(DPKG_INSTDIR)/test-conffile'
	$(DPKG_PURGE) pkg-conff-obsolete
	test ! -f '$(DPKG_INSTDIR)/test-conffile'
	# Unmodified conffile is autoremoved for the user
	$(DPKG_INSTALL) pkg-conff-obsolete-0.deb
	$(DPKG_UNPACK) pkg-conff-obsolete-2.deb
	test -f '$(DPKG_INSTDIR)/test-conffile.dpkg-remove'
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-backup'
	$(DPKG_CONFIGURE) pkg-conff-obsolete
	test ! -f '$(DPKG_INSTDIR)/test-conffile'
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-bak'
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-backup'
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-remove'
	$(DPKG_PURGE) pkg-conff-obsolete
	# Modified conffile is moved away in .dpkg-bak for the user
	$(DPKG_INSTALL) pkg-conff-obsolete-0.deb
	$(BEROOT) sh -c "echo foo >>'$(DPKG_INSTDIR)/test-conffile'"
	$(DPKG_INSTALL) pkg-conff-obsolete-2.deb
	test ! -f '$(DPKG_INSTDIR)/test-conffile'
	test -f '$(DPKG_INSTDIR)/test-conffile.dpkg-bak'
	$(DPKG_PURGE) pkg-conff-obsolete
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-bak'
	# Abort-upgrade restores the to-be-removed conffile
	$(DPKG_INSTALL) pkg-conff-obsolete-0.deb
	$(BEROOT) touch '$(DPKG_INSTDIR)/fail-preinst'
	$(DPKG_INSTALL) pkg-conff-obsolete-2.deb || true
	$(BEROOT) rm -f '$(DPKG_INSTDIR)/fail-preinst'
	test -f '$(DPKG_INSTDIR)/test-conffile'
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-remove'
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-bak'
	test ! -f '$(DPKG_INSTDIR)/test-conffile.dpkg-backup'

test-clean:
	$(DPKG_PURGE) pkg-conff-obsolete

