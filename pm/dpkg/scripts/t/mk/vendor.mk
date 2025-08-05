include $(srcdir)/mk/vendor.mk

TEST_DEB_VENDOR = Debian
TEST_DEB_PARENT_VENDOR =

test_vars := \
  DEB_VENDOR \
  DEB_PARENT_VENDOR \
  # EOL

test: $(test_vars)
	: # Test the dpkg_vendor_derives_from macro.
	test "$(shell $(call dpkg_vendor_derives_from,debian))" = "yes"

$(test_vars):
	: # Test $@ Make variable.
	test '$($@)' = '$(TEST_$@)'
