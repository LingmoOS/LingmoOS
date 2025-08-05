TESTS_DEB := pkg-conff-orphan-0 pkg-conff-orphan-1 pkg-conff-takeover

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-conff-orphan-0.deb
	$(DPKG_INSTALL) pkg-conff-orphan-1.deb
	# test if the other package can takeover the orphan conffile
	$(DPKG_INSTALL) pkg-conff-takeover.deb

test-clean:
	$(DPKG_PURGE) pkg-conff-orphan
	$(DPKG_PURGE) pkg-conff-takeover

