TESTS_DEB := pkg-conff-rename-0 pkg-conff-rename-1

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-conff-rename-0.deb
	$(DPKG_INSTALL) pkg-conff-rename-1.deb
	test -f '$(DPKG_INSTDIR)/test-conffile-new'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old.dpkg-remove'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old.dpkg-bak'
	$(DPKG_PURGE) pkg-conff-rename
	# Same upgrade but with modified conffile, should still not prompt
	$(DPKG_INSTALL) pkg-conff-rename-0.deb
	$(BEROOT) sh -c "echo foo >>'$(DPKG_INSTDIR)/test-conffile-old'"
	$(DPKG_INSTALL) pkg-conff-rename-1.deb
	test -f '$(DPKG_INSTDIR)/test-conffile-new'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old.dpkg-remove'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old.dpkg-bak'
	$(DPKG_PURGE) pkg-conff-rename
	# Abort-upgrade restores the to-be-renamed conffile
	$(DPKG_INSTALL) pkg-conff-rename-0.deb
	$(BEROOT) touch '$(DPKG_INSTDIR)/fail-preinst'
	$(DPKG_INSTALL) pkg-conff-rename-1.deb || true
	$(BEROOT) rm -f '$(DPKG_INSTDIR)/fail-preinst'
	test -f '$(DPKG_INSTDIR)/test-conffile-old'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-new'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old.dpkg-remove'
	test ! -f '$(DPKG_INSTDIR)/test-conffile-old.dpkg-bak'
	$(DPKG_PURGE) pkg-conff-rename

test-clean:

