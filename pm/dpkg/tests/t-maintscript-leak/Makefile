TESTS_DEB := pkg-maintscript-leak

include ../Test.mk

test-case:
	# Check for leaks on install
	$(DPKG_INSTALL) pkg-maintscript-leak.deb
	# Check for leaks on upgrade
	$(DPKG_INSTALL) pkg-maintscript-leak.deb
	# Check for leaks on removal
	$(DPKG_PURGE) pkg-maintscript-leak

test-clean:
	$(DPKG_PURGE) pkg-maintscript-leak
