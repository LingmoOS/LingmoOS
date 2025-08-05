TESTS_DEB := pkg-file-original pkg-file-takeover

include ../Test.mk

TEST_CASES += test-two-run-ordered
TEST_CASES += test-one-run-ordered
TEST_CASES += test-two-run-reverse
TEST_CASES += test-one-run-reverse

test-case: $(TEST_CASES)

define VERIFY
$(call pkg_is_installed,pkg-file-takeover)
test "`$(DPKG_QUERY) -S /test-file`" = "pkg-file-takeover: /test-file"
test "`$(DPKG_QUERY) -S /test-other`" = "pkg-file-takeover: /test-other"
cmp ref-file "$(DPKG_INSTDIR)/test-file"
cmp ref-file "$(DPKG_INSTDIR)/test-other"
endef

test-two-run-ordered:
	# test-two-run-ordered
	$(DPKG_INSTALL) pkg-file-original.deb
	$(DPKG_INSTALL) pkg-file-takeover.deb
	$(call pkg_is_not_installed,pkg-file-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover
	test ! -f "$(DPKG_INSTDIR)/test-file"
	test ! -f "$(DPKG_INSTDIR)/test-other"

test-one-run-ordered:
	# test-one-run-ordered
	$(DPKG_INSTALL) pkg-file-original.deb pkg-file-takeover.deb
	$(call pkg_is_not_installed,pkg-file-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover
	test ! -f "$(DPKG_INSTDIR)/test-file"
	test ! -f "$(DPKG_INSTDIR)/test-other"

test-two-run-reverse:
	# test-two-run-reverse
	$(DPKG_INSTALL) pkg-file-takeover.deb
	$(DPKG_INSTALL) pkg-file-original.deb
	$(call pkg_is_installed,pkg-file-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original
	test -f "$(DPKG_INSTDIR)/test-file"
	test -f "$(DPKG_INSTDIR)/test-other"
	$(DPKG_PURGE) pkg-file-takeover
	test ! -f "$(DPKG_INSTDIR)/test-file"
	test ! -f "$(DPKG_INSTDIR)/test-other"

test-one-run-reverse:
	# test-one-run-reverse
	$(DPKG_INSTALL) pkg-file-takeover.deb pkg-file-original.deb
	$(call pkg_is_installed,pkg-file-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original
	test -f "$(DPKG_INSTDIR)/test-file"
	test -f "$(DPKG_INSTDIR)/test-other"
	$(DPKG_PURGE) pkg-file-takeover
	test ! -f "$(DPKG_INSTDIR)/test-file"
	test ! -f "$(DPKG_INSTDIR)/test-other"

test-clean:
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover
