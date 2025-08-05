TESTS_DEB := pkg-conffile-v1 pkg-conffile-v2 \
             pkg-conffile-owns-filename pkg-conffile-owns-symlink

include ../Test.mk

build-hook:
	ln -sf test-conffile-owned \
	       pkg-conffile-owns-symlink/test-dir/test-conffile-to-be-removed

clean-hook:
	$(RM) pkg-conffile-owns-symlink/test-dir/test-conffile-to-be-removed
	$(RM) pkg-conffile-removal-present.deb

test-case:
	# Conffile marked for removal but present, cannot be installed.
	$(DPKG_BUILD_DEB) --no-check pkg-conffile-removal-present
	! $(DPKG_INSTALL) pkg-conffile-removal-present.deb
	$(call pkg_is_not_installed,pkg-conffile-removal-present)
	$(DPKG_PURGE) pkg-conffile-removal-present

	# Unmodified case, one invocation.
	$(DPKG_INSTALL) pkg-conffile-v1.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"
	$(DPKG_INSTALL) pkg-conffile-v2.deb
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Reset.
	$(DPKG_PURGE) pkg-conffile
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Unmodified case; but force it to happen via two invocations.
	$(DPKG_INSTALL) pkg-conffile-v1.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"
	$(DPKG_UNPACK) pkg-conffile-v2.deb
	# The old conffile is removed during unpack.
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"
	# After configure, the file is still removed.
	$(DPKG_CONFIGURE) pkg-conffile
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Reset.
	$(DPKG_PURGE) pkg-conffile
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Modified; should store .dpkg-old.
	$(DPKG_INSTALL) pkg-conffile-v1.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	echo "modify this" >"$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	$(DPKG_INSTALL) pkg-conffile-v2.deb
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Reset.
	$(DPKG_PURGE) pkg-conffile
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	$(RM) "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Queue a remove on upgrade, and move ownership.
	$(DPKG_INSTALL) pkg-conffile-v1.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	$(DPKG_INSTALL) pkg-conffile-v2.deb
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	$(DPKG_INSTALL) pkg-conffile-owns-filename.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	# Upgrade the package again, should not remove unowned conffile.
	$(DPKG_INSTALL) pkg-conffile-v2.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Reset.
	$(DPKG_PURGE) pkg-conffile
	$(DPKG_PURGE) pkg-conffile-owns
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"

	# Queue a remove on upgrade, and move ownership via symlink.
	$(DPKG_INSTALL) pkg-conffile-v1.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	$(DPKG_INSTALL) pkg-conffile-v2.deb
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	test ! -f "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed.dpkg-old"
	$(DPKG_INSTALL) pkg-conffile-owns-symlink.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-owned"
	test -L "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"
	# Upgrade the package again, should not remove unowned conffile.
	$(DPKG_INSTALL) pkg-conffile-v2.deb
	test -f "$(DPKG_INSTDIR)/test-dir/test-conffile-owned"
	test -L "$(DPKG_INSTDIR)/test-dir/test-conffile-to-be-removed"

test-clean:
	$(DPKG_PURGE) pkg-conffile-removal-present
	$(DPKG_PURGE) pkg-conffile-owns
	$(DPKG_PURGE) pkg-conffile
	$(BEROOT) $(RM) -rf "$(DPKG_INSTDIR)/test-dir"
