TESTS_DEB := pkg-trigger-unpack pkg-trigger-activate

include ../Test.mk

test-case:
	# Self trigger cycles cannot happen with unpacked files.
	$(DPKG_INSTALL) pkg-trigger-unpack.deb
	$(DPKG_UNPACK) pkg-trigger-unpack.deb
	$(DPKG_CONFIGURE) pkg-trigger-unpack
	$(DPKG_CONFIGURE) --pending
	$(DPKG_PURGE) pkg-trigger-unpack

	# Self trigger cycles cannot happen with activate directives.
	$(DPKG_INSTALL) pkg-trigger-activate.deb
	$(DPKG_UNPACK) pkg-trigger-activate.deb
	$(DPKG_CONFIGURE) pkg-trigger-activate
	$(DPKG_CONFIGURE) --pending
	$(DPKG_PURGE) pkg-trigger-activate

	# Cleanup.
	$(DPKG_PURGE) pkg-trigger-unpack
	$(DPKG_PURGE) pkg-trigger-activate

test-clean:
	-$(DPKG_PURGE) pkg-trigger-unpack pkg-trigger-activate
