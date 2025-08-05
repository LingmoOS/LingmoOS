TESTS_DEB := pkg-dir-real pkg-dir-symlink-0 pkg-dir-symlink-1 pkg-file

include ../Test.mk

build-hook:
	ln -fs test-dir pkg-dir-symlink-0/test-symlink
	ln -fs test-dir pkg-dir-symlink-1/test-symlink

clean-hook:
	$(RM) pkg-dir-symlink-0/test-symlink
	$(RM) pkg-dir-symlink-1/test-symlink

test-case:
	$(DPKG_INSTALL) pkg-dir-real.deb
	$(DPKG_INSTALL) pkg-dir-symlink-0.deb
	$(DPKG_INSTALL) pkg-file.deb
	$(DPKG_INSTALL) pkg-dir-symlink-1.deb
	
	$(DPKG_PURGE) pkg-dir-symlink
	$(DPKG_PURGE) pkg-file
	$(DPKG_PURGE) pkg-dir-real
	
	test ! -L '$(DPKG_INSTDIR)/test-symlink'

test-clean:
	$(DPKG_PURGE) pkg-dir-symlink
	$(DPKG_PURGE) pkg-file
	$(DPKG_PURGE) pkg-dir-real
	$(BEROOT) $(RM) '$(DPKG_INSTDIR)/test-symlink'
