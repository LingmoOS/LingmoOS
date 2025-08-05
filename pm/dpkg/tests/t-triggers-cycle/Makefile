TESTS_DEB := pkg-depends pkg-trigger-a pkg-trigger-b

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-depends.deb
	# Should fail due to a trigger cycle.
	! $(DPKG_INSTALL) pkg-trigger-a.deb pkg-trigger-b.deb
	! $(DPKG_CONFIGURE) --pending

	# Make sure everything is left in a correct state.
	$(call pkg_is_installed,pkg-depends)
	$(call pkg_is_installed,pkg-trigger-a)
	$(call pkg_status_is,pkg-trigger-b,install ok half-configured)

	# Cleanup.
	$(DPKG_PURGE) pkg-trigger-a pkg-trigger-b
	$(DPKG_PURGE) pkg-depends

test-clean:
	-$(DPKG_PURGE) pkg-trigger-a pkg-trigger-b pkg-depends
