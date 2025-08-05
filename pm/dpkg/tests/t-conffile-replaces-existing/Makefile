TESTS_DEB := pkg-conff-original-0 pkg-conff-original-1 pkg-conff-takeover

include ../Test.mk

TEST_CASES += test-two-run-ordered
TEST_CASES += test-one-run-ordered
TEST_CASES += test-two-run-reverse
TEST_CASES += test-one-run-reverse

test-case: $(TEST_CASES)

define VERIFY
$(call pkg_is_installed,pkg-conff-original)
$(call pkg_is_installed,pkg-conff-takeover)
test "`$(DPKG_QUERY) -S /test-conffile`" = "pkg-conff-takeover: /test-conffile"
cmp ref-conffile "$(DPKG_INSTDIR)/test-conffile"
endef

test-two-run-ordered:
	# test-two-run-ordered
	$(DPKG_INSTALL) pkg-conff-original-0.deb
	$(DPKG_INSTALL) pkg-conff-original-1.deb
	$(DPKG_INSTALL) pkg-conff-takeover.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

test-one-run-ordered:
	# test-one-run-ordered
	$(DPKG_INSTALL) pkg-conff-original-0.deb
	$(DPKG_INSTALL) pkg-conff-original-1.deb pkg-conff-takeover.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

test-two-run-reverse:
	# test-two-run-reverse
	$(DPKG_INSTALL) pkg-conff-original-0.deb
	$(DPKG_INSTALL) pkg-conff-takeover.deb
	$(DPKG_INSTALL) pkg-conff-original-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

test-one-run-reverse:
	# test-one-run-reverse
	$(DPKG_INSTALL) pkg-conff-original-0.deb
	$(DPKG_INSTALL) pkg-conff-takeover.deb pkg-conff-original-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

