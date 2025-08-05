TESTS_DEB := pkg-files pkg-depends pkg-remove pkg-trigger

include ../Test.mk

test-case:
	# This is what apt is doing currently. :(
	$(DPKG_INSTALL) pkg-files.deb
	$(DPKG_INSTALL) pkg-depends.deb
	$(DPKG_INSTALL) pkg-remove.deb
	$(DPKG_INSTALL) pkg-trigger.deb
	$(DPKG_REMOVE) --force-depends pkg-remove
	$(DPKG_INSTALL) pkg-files.deb
	$(DPKG_UNPACK) pkg-depends.deb

	$(DPKG) -s pkg-trigger pkg-files pkg-depends

	# Make sure everything is left in the expected state.
	$(call pkg_status_is,pkg-depends,install ok unpacked)
	$(call pkg_status_is,pkg-files,install ok triggers-awaited)
	$(call pkg_status_is,pkg-trigger,install ok triggers-pending)

	# Should fail due to unsatisfied dependencies.
	! $(DPKG_CONFIGURE) pkg-trigger pkg-depends

test-clean:
	-$(DPKG_PURGE) pkg-trigger pkg-depends pkg-files
