DEB_BUILD_ARCH      := overridden
TEST_DEB_BUILD_ARCH := overridden

include $(srcdir)/mk/architecture.mk

vars := \
  ARCH \
  ARCH_ABI \
  ARCH_BITS \
  ARCH_CPU \
  ARCH_ENDIAN \
  ARCH_LIBC \
  ARCH_OS \
  GNU_CPU \
  GNU_SYSTEM \
  GNU_TYPE \
  MULTIARCH \
  # EOL
loop_targets := $(foreach machine,BUILD HOST TARGET,\
                  $(foreach var,$(vars),DEB_$(machine)_$(var)))

test: $(loop_targets)

$(loop_targets):
	: # Test the $@ Make variable.
	test '$($@)' = '$(TEST_$@)'
	: # Test the $@ exported variable.
	test "$${$@}" = '$(TEST_$@)'
