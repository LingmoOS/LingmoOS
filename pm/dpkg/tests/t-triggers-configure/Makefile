TESTS_DEB := pkg-activate pkg-interest-0 pkg-interest-1

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-interest-0.deb

	# Activate the trigger, and leave pkg-interest in triggers-pending.
	$(DPKG_INSTALL) --no-triggers pkg-activate.deb
	$(call pkg_status_is,pkg-interest,install ok triggers-pending)

	# Test that an upgrade from triggers-pending passes the previous
	# configured version to "postinst configure".
	$(DPKG_INSTALL) pkg-interest-1.deb

	# Cleanup.
	$(DPKG_PURGE) pkg-interest
	$(DPKG_PURGE) pkg-activate

test-clean:
	-$(DPKG_PURGE) pkg-interest
	-$(DPKG_PURGE) pkg-activate
