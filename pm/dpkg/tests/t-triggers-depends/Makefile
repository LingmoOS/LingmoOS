TESTS_DEB := pkg-depended pkg-interest pkg-activate

include ../Test.mk

test-case:
	# Should pass due to satisfied dependencies.
	$(DPKG_INSTALL) pkg-depended.deb pkg-interest.deb
	$(DPKG_INSTALL) pkg-activate.deb
	$(DPKG_CONFIGURE) --pending

	# Should fail due to unsatisfied dependencies.
	$(DPKG_UNPACK) --no-triggers pkg-depended.deb
	$(DPKG_INSTALL) --no-triggers pkg-activate.deb
	! $(DPKG_CONFIGURE) pkg-interest

	# Should pass now due to satisfied dependencies.
	$(DPKG_CONFIGURE) --no-triggers pkg-depended
	$(DPKG_CONFIGURE) pkg-interest

	# Cleanup.
	$(DPKG_PURGE) pkg-trigger-a pkg-trigger-b
	$(DPKG_PURGE) pkg-depends

test-clean:
	-$(DPKG_PURGE) pkg-activate pkg-interest pkg-depended
