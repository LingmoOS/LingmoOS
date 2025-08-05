TESTS_DEB := pkg-failinst-failrm-0 pkg-failinst-failrm-1

include ../Test.mk

test-case:
	-$(DPKG_INSTALL) pkg-failinst-failrm-0.deb
	$(call pkg_status_is,pkg-failinst-failrm,install ok half-configured)

test-clean:
	# Upgrade to a working version to allow the purge.
	-$(DPKG_INSTALL) pkg-failinst-failrm-1.deb
	$(DPKG_PURGE) pkg-failinst-failrm
