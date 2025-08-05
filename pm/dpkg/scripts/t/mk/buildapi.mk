include $(srcdir)/mk/buildapi.mk

TEST_DPKG_BUILD_API = 0

test_vars := \
  DPKG_BUILD_API \
  # EOL

test: $(test_vars)

$(test_vars):
	: # Test $@ Make variable.
	test '$($@)' = '$(TEST_$@)'
