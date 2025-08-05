TESTS_DEB := pkg-dir-shared-0 pkg-dir-shared-1

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-dir-shared-0.deb
	$(DPKG_INSTALL) pkg-dir-shared-1.deb
	$(DPKG_REMOVE) pkg-dir-shared-1
	$(DPKG_REMOVE) pkg-dir-shared-0
	$(DPKG_PURGE) pkg-dir-shared-0
	$(DPKG_PURGE) pkg-dir-shared-1
	test ! -d '$(DPKG_INSTDIR)/test-dir'

test-clean:
	$(DPKG_PURGE) pkg-dir-shared-0
	$(DPKG_PURGE) pkg-dir-shared-1
	$(BEROOT) rm -rf '$(DPKG_INSTDIR)/test-dir'
