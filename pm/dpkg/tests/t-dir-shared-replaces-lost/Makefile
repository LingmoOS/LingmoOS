TESTS_DEB := pkg-dir-original pkg-dir-takeover

include ../Test.mk

TEST_CASES += test-ordered
TEST_CASES += test-reverse

test-case: $(TEST_CASES)

define VERIFY
$(call pkg_is_installed,pkg-dir-original)
$(call pkg_is_installed,pkg-dir-takeover)
test -d '$(DPKG_INSTDIR)/test-dir'
test "`$(DPKG_QUERY) -S /test-dir/test-original`" = "pkg-dir-original: /test-dir/test-original"
test "`$(DPKG_QUERY) -S /test-dir/test-takeover`" = "pkg-dir-takeover: /test-dir/test-takeover"
endef

test-ordered:
	# test-ordered
	$(DPKG_INSTALL) pkg-dir-original.deb
	$(BEROOT) rm -rf '$(DPKG_INSTDIR)/test-dir'
	$(DPKG_INSTALL) pkg-dir-takeover.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-dir-original pkg-dir-takeover

test-reverse:
	# test-reverse
	$(DPKG_INSTALL) pkg-dir-takeover.deb
	$(BEROOT) rm -rf '$(DPKG_INSTDIR)/test-dir'
	$(DPKG_INSTALL) pkg-dir-original.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-dir-original pkg-dir-takeover

test-clean:
	-$(DPKG_PURGE) pkg-dir-original pkg-dir-takeover
