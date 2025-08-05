TESTS_DEB := pkg-real pkg-predepends pkg-trigger

include ../Test.mk

test-case:
	# Install the file trigger
	$(DPKG_INSTALL) pkg-trigger.deb
	# Install the pre-dependency with --no-triggers
	$(DPKG_INSTALL) --no-triggers pkg-real.deb
	$(call pkg_status_is,pkg-real,install ok triggers-awaited)
	# Should pass even if not yet configured...
	$(DPKG_UNPACK) --no-triggers pkg-predepends.deb
	# ...by running the required triggers
	$(call pkg_status_is,pkg-real,install ok installed)
	# Retry again, this time the trigger processing is not required
	# because pkg-real has already been configured once
	$(DPKG_PURGE) pkg-predepends
	$(DPKG_INSTALL) --no-triggers pkg-real.deb
	$(call pkg_status_is,pkg-real,install ok triggers-awaited)
	$(DPKG_UNPACK) --no-triggers pkg-predepends.deb
	$(call pkg_status_is,pkg-real,install ok triggers-awaited)
	# Cleanup
	$(DPKG_PURGE) pkg-predepends
	$(DPKG_PURGE) pkg-real
	$(DPKG_PURGE) pkg-trigger

test-clean:
	-$(DPKG_PURGE) pkg-predepends pkg-real pkg-trigger
