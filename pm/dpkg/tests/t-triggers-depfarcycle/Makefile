TESTS_DEB := pkg-depends-a pkg-depends-b pkg-files pkg-trigger

include ../Test.mk

test-case:
	# The trigger cycle does not affect us here.
	$(DPKG_INSTALL) pkg-depends-a.deb pkg-files.deb \
	                pkg-depends-b.deb pkg-trigger.deb
	$(DPKG_UNPACK) pkg-files.deb
	$(DPKG_CONFIGURE) --pending

	# Make sure everything is left in a correct state.
	$(call pkg_is_installed,pkg-files)
	$(call pkg_is_installed,pkg-depends-a)
	$(call pkg_is_installed,pkg-depends-b)
	$(call pkg_is_installed,pkg-trigger)

	# Provoke the trigger cycle.
	$(DPKG_UNPACK) pkg-depends-a.deb
	$(DPKG_UNPACK) pkg-depends-b.deb
	$(DPKG_UNPACK) pkg-files.deb
	! $(DPKG_CONFIGURE) pkg-trigger
	$(DPKG) -s pkg-depends-a pkg-depends-b pkg-files pkg-trigger
	! $(DPKG_CONFIGURE) --pending

	# Make sure everything is left in a correct state.
	$(call pkg_is_installed,pkg-files)
	$(call pkg_is_installed,pkg-depends-a)
	$(call pkg_is_installed,pkg-depends-b)
	$(call pkg_status_is,pkg-trigger,install ok half-configured)

	# Cleanup.
	$(DPKG_PURGE) pkg-trigger pkg-files pkg-depends-a pkg-depends-b

test-clean:
	-$(DPKG_PURGE) pkg-trigger pkg-files pkg-depends-a pkg-depends-b
