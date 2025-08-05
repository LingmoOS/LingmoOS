TESTS_DEB := pkg-dir-real pkg-dir-symlink-0 pkg-dir-symlink-1

include ../Test.mk

build-hook:
	ln -sf test-dir pkg-dir-symlink-0/test-symlink

clean-hook:
	$(RM) pkg-dir-symlink-0/test-symlink

test-case:
	$(DPKG_INSTALL) pkg-dir-real.deb

	$(DPKG_INSTALL) pkg-dir-symlink-0.deb
	test -h "$(DPKG_INSTDIR)/test-symlink"

	$(DPKG_INSTALL) pkg-dir-symlink-1.deb
	! test -h "$(DPKG_INSTDIR)/test-symlink"
	test -d "$(DPKG_INSTDIR)/test-symlink"

	$(DPKG_PURGE) pkg-dir-symlink
	! test -e "$(DPKG_INSTDIR)/test-symlink"

	$(DPKG_PURGE) pkg-dir-real

test-clean:
	$(DPKG_PURGE) pkg-dir-symlink
	$(DPKG_PURGE) pkg-dir-real
