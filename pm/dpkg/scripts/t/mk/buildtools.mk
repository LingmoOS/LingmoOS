AR                := overridden
TEST_AR           := overridden
TEST_AR_FOR_BUILD := overridden

DPKG_EXPORT_BUILDTOOLS := 1

include $(srcdir)/mk/buildtools.mk

tools := \
  AR \
  AS \
  CC \
  CPP \
  CXX \
  F77 \
  FC \
  LD \
  NM \
  OBJC \
  OBJCOPY \
  OBJCXX \
  OBJDUMP \
  PKG_CONFIG \
  RANLIB \
  STRIP \
  # EOL
loop_targets := $(tools) $(tools:=_FOR_BUILD)

test: $(loop_targets)

$(loop_targets):
	: # Test the $@ Make variable.
	test '$($@)' = '$(TEST_$@)'
	: # Test the $@ exported variable.
	test "$${$@}" = '$(TEST_$@)'
