DEB_CPPFLAGS_MAINT_APPEND = -DTEST_MK=test-host
TEST_CPPFLAGS            += -DTEST_MK=test-host

DEB_CPPFLAGS_FOR_BUILD_MAINT_APPEND = -DTEST_MK=test-build
TEST_CPPFLAGS_FOR_BUILD            += -DTEST_MK=test-build

DEB_CXXFLAGS_MAINT_SET := set-host
TEST_CXXFLAGS          := set-host

DEB_CXXFLAGS_FOR_BUILD_MAINT_SET := set-build
TEST_CXXFLAGS_FOR_BUILD          := set-build

DEB_CFLAGS_MAINT_APPEND = -DTEST_MAKE_EXPANSION=$(expanded_on_demand)
expanded_on_demand := contents
TEST_CFLAGS += -DTEST_MAKE_EXPANSION=contents

DPKG_EXPORT_BUILDFLAGS := 1

include $(srcdir)/mk/buildflags.mk

vars := \
  ASFLAGS \
  CFLAGS \
  CPPFLAGS \
  CXXFLAGS \
  DFLAGS \
  FCFLAGS \
  FFLAGS \
  LDFLAGS \
  OBJCFLAGS \
  OBJCXXFLAGS \
  # EOL
loop_targets := $(vars) $(vars:=_FOR_BUILD)

test: $(loop_targets)

$(loop_targets):
	: # Test the $@ Make variable.
	test '$($@)' = '$(TEST_$@)'
	: # Test the $@ exported variable.
	test "$${$@}" = '$(TEST_$@)'
