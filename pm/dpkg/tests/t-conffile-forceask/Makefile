TESTS_DEB := pkg-conff-forceask-0 pkg-conff-forceask-1

include ../Test.mk

test-case:

	# Test that we get the new conffile after a modified one.
	$(DPKG_INSTALL) pkg-conff-forceask-0.deb
	$(BEROOT) cp mod-conffile "$(DPKG_INSTDIR)/test-conffile"
	$(DPKG_INSTALL) --force-confask --force-confnew pkg-conff-forceask-1.deb
	cmp mod-conffile "$(DPKG_INSTDIR)/test-conffile.dpkg-old"
	cmp ref-conffile "$(DPKG_INSTDIR)/test-conffile"
	$(DPKG_PURGE) pkg-conff-forceask

test-clean:
	$(DPKG_PURGE) pkg-conff-forceask
