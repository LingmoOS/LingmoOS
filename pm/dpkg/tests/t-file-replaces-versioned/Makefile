TESTS_DEB := pkg-file-original-0 pkg-file-original-1 \
             pkg-file-takeover-0 pkg-file-takeover-1

include ../Test.mk

TEST_CASES += test-two-run-ordered
TEST_CASES += test-one-run-ordered
TEST_CASES += test-two-run-reverse
TEST_CASES += test-one-run-reverse

test-case: $(TEST_CASES)

define VERIFY
$(call pkg_is_installed,pkg-file-original)
$(call pkg_is_installed,pkg-file-takeover)
test "`$(DPKG_QUERY) -S /test-file`" = "pkg-file-takeover: /test-file"
cmp ref-file "$(DPKG_INSTDIR)/test-file"
endef

test-two-run-ordered:
	# test-two-run-ordered
	$(DPKG_INSTALL) pkg-file-original-0.deb pkg-file-takeover-0.deb
	$(DPKG_INSTALL) pkg-file-original-1.deb
	$(DPKG_INSTALL) pkg-file-takeover-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover

test-one-run-ordered:
	# test-one-run-ordered
	$(DPKG_INSTALL) pkg-file-original-0.deb pkg-file-takeover-0.deb
	$(DPKG_INSTALL) pkg-file-original-1.deb pkg-file-takeover-1.deb
	$(DPKG_CONFIGURE) -a
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover

test-two-run-reverse:
	# test-two-run-reverse
	$(DPKG_INSTALL) pkg-file-original-0.deb pkg-file-takeover-0.deb
	$(DPKG_INSTALL) pkg-file-takeover-1.deb
	$(DPKG_INSTALL) pkg-file-original-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover

test-one-run-reverse:
	# test-one-run-reverse
	$(DPKG_INSTALL) pkg-file-original-0.deb pkg-file-takeover-0.deb
	$(DPKG_INSTALL) pkg-file-takeover-1.deb pkg-file-original-1.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover

test-clean:
	$(DPKG_PURGE) pkg-file-original pkg-file-takeover
