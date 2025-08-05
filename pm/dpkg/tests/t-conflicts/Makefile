TESTS_DEB := pkg-a pkg-b

include ../Test.mk

test-case:
	# test if the conflicting packages fail on install
	! $(DPKG_INSTALL) pkg-b.deb pkg-a.deb
	$(call pkg_is_not_installed,pkg-a)
	-$(DPKG_PURGE) pkg-a pkg-b
	! $(DPKG_INSTALL) pkg-a.deb pkg-b.deb
	$(call pkg_is_not_installed,pkg-b)
	-$(DPKG_PURGE) pkg-a pkg-b

	# Test installation generating removal of conflicts.
	$(DPKG_INSTALL) pkg-b.deb
	# Set up the desired transaction.
	echo pkg-b deinstall | $(BEROOT) $(DPKG) --set-selections
	$(DPKG_INSTALL) pkg-a.deb
	$(call pkg_is_installed,pkg-a)
	$(call pkg_is_not_installed,pkg-b)
	-$(DPKG_PURGE) pkg-a pkg-b

test-clean:
	-$(DPKG_PURGE) pkg-a pkg-b
