TESTS_DEB := dir-a/pkg-a dir-b/dir-bb/pkg-b pkg-c

include ../Test.mk

test-case:
	# Try recursive directory unpacking of packages
	$(DPKG_INSTALL) -R .
	$(call pkg_is_unpacked,pkg-a)
	$(call pkg_is_unpacked,pkg-b)
	$(call pkg_is_unpacked,pkg-c)
	
	$(DPKG_PURGE) pkg-a
	$(DPKG_PURGE) pkg-b
	$(DPKG_PURGE) pkg-c
	
	# Try recursive directory installation of packages
	$(DPKG_INSTALL) -R .
	$(call pkg_is_installed,pkg-a)
	$(call pkg_is_installed,pkg-b)
	$(call pkg_is_installed,pkg-c)

test-clean:
	$(DPKG_PURGE) pkg-a
	$(DPKG_PURGE) pkg-b
	$(DPKG_PURGE) pkg-c
