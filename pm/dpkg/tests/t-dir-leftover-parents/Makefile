TESTS_DEB := pkg-dir-shared pkg-dir-shared-notempty

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-dir-shared.deb
	$(DPKG_INSTALL) pkg-dir-shared-notempty.deb
	$(DPKG_REMOVE) pkg-dir-shared-notempty
	$(DPKG_PURGE) pkg-dir-shared
	$(DPKG_PURGE) pkg-dir-shared-notempty
	test ! -d "$(DPKG_INSTDIR)/test-dir"

test-clean:
	$(DPKG_PURGE) pkg-dir-shared
	$(DPKG_PURGE) pkg-dir-shared-notempty
	$(BEROOT) rm -rf "$(DPKG_INSTDIR)/test-dir"
