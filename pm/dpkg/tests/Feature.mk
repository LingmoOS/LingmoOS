#
# Dpkg functional testsuite (kind of)
#
# Copyright © 2015 Guillem Jover <guillem@debian.org>
#

## Feature checks setup ##

ifneq ($(DPKG_FEATURE_CHECKS),yes)
export DPKG_FEATURE_CHECKS := yes

# XXX: once apt is fixed:
#export DPKG_HAS_CONFIGURE_WITH_IMPLICIT_TRIGGER_PENDING ?= 1

ifneq (,$(filter as-root,$(DPKG_TESTSUITE_OPTIONS)))
export DPKG_AS_ROOT = 1
endif

endif
