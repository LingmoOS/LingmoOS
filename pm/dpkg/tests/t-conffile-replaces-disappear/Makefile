TESTS_DEB := pkg-conff-original pkg-conff-takeover

include ../Test.mk

TEST_CASES += test-two-run-ordered
TEST_CASES += test-one-run-ordered
TEST_CASES += test-two-run-reverse
TEST_CASES += test-one-run-reverse

test-case: $(TEST_CASES)

define VERIFY
$(call pkg_is_installed,pkg-conff-takeover)
test "`$(DPKG_QUERY) -S /test-conffile`" = "pkg-conff-takeover: /test-conffile"
cmp ref-conffile "$(DPKG_INSTDIR)/test-conffile"
endef

test-two-run-ordered:
	# test-two-run-ordered
	$(DPKG_INSTALL) pkg-conff-original.deb
	$(DPKG_INSTALL) pkg-conff-takeover.deb
	$(call pkg_is_not_installed,pkg-conff-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover
	test ! -f "$(DPKG_INSTDIR)/test-conffile"

test-one-run-ordered:
	# test-one-run-ordered
	$(DPKG_INSTALL) pkg-conff-original.deb pkg-conff-takeover.deb
	$(call pkg_is_not_installed,pkg-conff-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover
	test ! -f "$(DPKG_INSTDIR)/test-conffile"

test-two-run-reverse:
	# test-two-run-reverse
	$(DPKG_INSTALL) pkg-conff-takeover.deb
	$(DPKG_INSTALL) pkg-conff-original.deb
	$(call pkg_is_installed,pkg-conff-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover
	test ! -f "$(DPKG_INSTDIR)/test-conffile"

test-one-run-reverse:
	# test-one-run-reverse
	$(DPKG_INSTALL) pkg-conff-takeover.deb pkg-conff-original.deb
	$(call pkg_is_installed,pkg-conff-original)
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover
	test ! -f "$(DPKG_INSTDIR)/test-conffile"

