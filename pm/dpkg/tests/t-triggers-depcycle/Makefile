TESTS_DEB := \
	pkg-depends-0 pkg-depends-1 \
	pkg-files-0 pkg-files-1 \
	pkg-trigger

include ../Test.mk

test-case:
	# Now provoke the trigger cycle
	$(DPKG_INSTALL) pkg-depends-0.deb
	$(DPKG_INSTALL) pkg-trigger.deb pkg-files-0.deb
	$(DPKG_UNPACK) pkg-files-1.deb
	! $(DPKG_CONFIGURE) --pending

	# Make sure everything is left in the expected state.
	$(call pkg_is_installed,pkg-files)
	$(call pkg_is_installed,pkg-depends)
	$(call pkg_status_is,pkg-trigger,install ok half-configured)

	# Cleanup.
	$(DPKG_PURGE) pkg-trigger pkg-files
	$(DPKG_PURGE) pkg-depends

test-clean:
	-$(DPKG_PURGE) --force-all pkg-trigger pkg-files pkg-depends
