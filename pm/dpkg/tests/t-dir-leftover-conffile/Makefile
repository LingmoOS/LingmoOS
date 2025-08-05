TESTS_DEB := pkg-dir-leftover-conffile

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-dir-leftover-conffile.deb
	$(DPKG_REMOVE) pkg-dir-leftover-conffile
	$(DPKG_PURGE) pkg-dir-leftover-conffile
	test ! -d '$(DPKG_INSTDIR)/test-dir'

test-clean:
	$(DPKG_PURGE) pkg-dir-leftover-conffile
	$(BEROOT) rm -rf '$(DPKG_INSTDIR)/test-dir'
