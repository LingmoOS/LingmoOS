TESTS_DEB := pkg-conff-0 pkg-conff-1 pkg-conff-move

include ../Test.mk

DPKG_DIVERT_OPTIONS += --local --rename --divert /test/conffile.moved

test-case:
	$(DPKG_INSTALL) pkg-conff-0.deb
	$(DPKG_DIVERT_ADD) /test/conffile
	test -f '$(DPKG_INSTDIR)/test/conffile.moved'
	# Test if the other package can takeover a diverted conffile w/o error.
	$(DPKG_INSTALL) pkg-conff-1.deb
	$(DPKG_INSTALL) pkg-conff-move.deb

test-clean:
	$(DPKG_DIVERT_DEL) /test/conffile
	$(DPKG_PURGE) pkg-conff
	$(DPKG_PURGE) pkg-conff-move
