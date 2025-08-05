TESTS_DEB := pkg-conff-a pkg-conff-b pkg-conff-c

include ../Test.mk

TEST_CASES += test-conflict-installed-implicit
TEST_CASES += test-conflict-removed-implicit
TEST_CASES += test-conflict-removed-explicit

test-case: $(TEST_CASES)

test-conflict-installed-implicit:
	# test if packages fail on install due to conflicting conffile
	$(DPKG_INSTALL) pkg-conff-a.deb
	! $(DPKG_INSTALL) pkg-conff-b.deb
	-$(DPKG_PURGE) pkg-conff-b
	$(DPKG_PURGE) pkg-conff-a

test-conflict-removed-implicit:
	# test that conffile of removed package can be taken over
	$(DPKG_INSTALL) pkg-conff-a.deb
	$(DPKG_REMOVE) pkg-conff-a
	$(DPKG_INSTALL) pkg-conff-b.deb
	# Verify the new conffile has been installed
	grep -q pkg-conff-b "$(DPKG_INSTDIR)/test-conffile"
	# Verify it's kept after purge of old package
	$(DPKG_PURGE) pkg-conff-a
	test -e "$(DPKG_INSTDIR)/test-conffile"
	$(DPKG_PURGE) pkg-conff-b

test-conflict-removed-explicit:
	# test that conffile of removed package can be taken over even if
	# the new package is conflicting with the old one
	$(DPKG_INSTALL) pkg-conff-a.deb
	$(DPKG_REMOVE) pkg-conff-a
	$(DPKG_INSTALL) pkg-conff-c.deb
	# Verify the new conffile has been installed
	grep -q pkg-conff-c "$(DPKG_INSTDIR)/test-conffile"
	# Verify it's kept after purge of old package
	$(DPKG_PURGE) pkg-conff-a
	test -e "$(DPKG_INSTDIR)/test-conffile"
	$(DPKG_PURGE) pkg-conff-c

test-clean:
	-$(DPKG_PURGE) pkg-conff-a pkg-conff-b pkg-conff-c
