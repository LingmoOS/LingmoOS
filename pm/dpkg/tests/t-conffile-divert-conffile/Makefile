TESTS_DEB := pkg-conff pkg-conff-divert

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-conff.deb
	# test if the other package can divert the conffile w/o prompt
	$(DPKG_INSTALL) pkg-conff-divert.deb

test-clean:
	$(DPKG_PURGE) pkg-conff-divert
	$(DPKG_PURGE) pkg-conff

