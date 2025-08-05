TESTS_DEB := pkg-symlink

include ../Test.mk

build-hook:
	ln -fs test-file pkg-symlink/test-symlink

clean-hook:
	$(RM) pkg-symlink/test-symlink

test-case:
	# test unpacking symlinks
	$(DPKG_INSTALL) pkg-symlink.deb
	$(call pkg_is_installed,pkg-symlink)
	test -f "$(DPKG_INSTDIR)/test-file"
	test -L "$(DPKG_INSTDIR)/test-symlink"
	test "`readlink '$(DPKG_INSTDIR)/test-symlink'`" = "test-file"

test-clean:
	$(DPKG_PURGE) pkg-symlink
