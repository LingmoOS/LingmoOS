TESTS_DEB := pkg-conff-forcenew-0 pkg-conff-forcenew-1

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-conff-forcenew-0.deb
	# Modify the conffile.
	$(BEROOT) cp mod-conffile "$(DPKG_INSTDIR)/test-conffile"
	# Test that we do not get the new conffile.
	$(DPKG_INSTALL) pkg-conff-forcenew-0.deb
	cmp mod-conffile "$(DPKG_INSTDIR)/test-conffile"
	# Test that we get the new conffile.
	$(DPKG_INSTALL) --force-confnew pkg-conff-forcenew-1.deb
	cmp ref-conffile "$(DPKG_INSTDIR)/test-conffile"

test-clean:
	$(DPKG_PURGE) pkg-conff-forcenew
