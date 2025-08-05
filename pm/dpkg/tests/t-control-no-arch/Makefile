include ../Test.mk

test-case:
	# test building package with missing Architecture field.
	! $(DPKG_BUILD_DEB) pkg-no-arch

	# build bogus packages
	$(DPKG_BUILD_DEB) --no-check pkg-no-arch

	# test handling of package without the Architecture field
	! $(DPKG_INSTALL) pkg-no-arch.deb
	$(call pkg_is_not_installed,pkg-no-arch)
	# these ancient/bogus packages need to be forced now
	$(DPKG_INSTALL) --force-architecture pkg-no-arch.deb
	$(call pkg_is_installed,pkg-no-arch)
	! $(call stdout_has,$(DPKG_QUERY) --status pkg-no-arch,^Architecture:)
	$(DPKG_PURGE) pkg-no-arch
	# Clear the available file to avoid missing Architecture warnings
	$(BEROOT) $(DPKG) --clear-avail

test-clean:
	rm -f pkg-no-arch.deb
