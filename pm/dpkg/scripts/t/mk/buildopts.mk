include $(srcdir)/mk/buildopts.mk

test_vars := \
  DEB_BUILD_OPTION_PARALLEL \
  # EOL

test: $(test_vars)

$(test_vars):
	: # Test the $@ Make variable.
	test '$($@)' = '$(TEST_$@)'
