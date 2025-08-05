TESTS_DEB := pkg-conffile

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-conffile.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile"
	$(DPKG_REMOVE) pkg-conffile
	$(DPKG_PURGE) pkg-conffile
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile"
	test ! -d "$(DPKG_INSTDIR)/test-dir"

test-clean:
	$(DPKG_PURGE) pkg-conffile
	$(BEROOT) $(RM) -rf "$(DPKG_INSTDIR)/test-dir"
