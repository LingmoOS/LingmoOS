TESTS_DEB := pkg-conff-original pkg-conff-takeover-0 pkg-conff-takeover-1

include ../Test.mk

TEST_CASES += test-two-run-ordered
TEST_CASES += test-one-run-ordered
TEST_CASES += test-two-run-reverse
TEST_CASES += test-one-run-reverse

test-case: $(TEST_CASES)

define CONFFILES
$(DPKG_QUERY) -f '$${Package} @ $${Conffiles}\n'
endef

define VERIFY
$(call pkg_is_installed,pkg-conff-original)
$(call pkg_is_installed,pkg-conff-takeover)
test "`$(DPKG_QUERY) -S /test-conffile`" = "pkg-conff-takeover: /test-conffile"
test "`$(DPKG_QUERY) -S /test-confpath`" = "pkg-conff-takeover: /test-confpath"
test "`$(CONFFILES) -W pkg-conff-original | grep obsolete | wc -l`" = "2"
test "`$(CONFFILES) -W pkg-conff-takeover | grep -v obsolete | wc -l`" = "2"
cmp ref-conffile "$(DPKG_INSTDIR)/test-conffile"
cmp ref-conffile "$(DPKG_INSTDIR)/test-confpath"
endef

test-two-run-ordered:
	# test-two-run-ordered
	$(DPKG_INSTALL) pkg-conff-original.deb
	$(DPKG_INSTALL) pkg-conff-takeover-0.deb
	$(DPKG_INSTALL) pkg-conff-takeover-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

test-one-run-ordered:
	# test-one-run-ordered
	$(DPKG_INSTALL) pkg-conff-original.deb pkg-conff-takeover-0.deb
	$(DPKG_INSTALL) pkg-conff-takeover-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

test-two-run-reverse:
	# test-two-run-reverse
	$(DPKG_INSTALL) pkg-conff-takeover-0.deb
	$(DPKG_INSTALL) pkg-conff-original.deb
	$(DPKG_INSTALL) pkg-conff-takeover-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

test-one-run-reverse:
	# test-one-run-reverse
	$(DPKG_INSTALL) pkg-conff-takeover-0.deb pkg-conff-original.deb
	$(DPKG_INSTALL) pkg-conff-takeover-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

test-clean:
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover
