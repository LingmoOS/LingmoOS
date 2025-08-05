#
# Dpkg functional testsuite (kind of)
#
# Copyright © 2008-2012 Guillem Jover <guillem@debian.org>
#

-include ../.pkg-tests.conf

## Feature checks setup ##

include ../Feature.mk

## Test case support ##

ifneq (,$(filter debug,$(DPKG_TESTSUITE_OPTIONS)))
DPKG_MAINTSCRIPT_DEBUG = DPKG_DEBUG=1
endif

DPKG_PATH := $(PATH):/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
ifdef DPKG_BUILDTREE
DPKG_PATH := $(DPKG_BUILDTREE)/src:$(DPKG_BUILDTREE)/utils:$(DPKG_BUILDTREE)/scripts:$(DPKG_PATH)
endif

DPKG_ENV = \
  PATH=$(DPKG_PATH) \
  $(DPKG_MAINTSCRIPT_DEBUG)

# eatmydata confuses ASAN link order check.
export ASAN_OPTIONS = verify_asan_link_order=0
# Do not fail due to leaks, as the code is still using lots of
# static variables and error variables.
export LSAN_OPTIONS = exitcode=0

ifdef DPKG_BUILDTREE
export DPKG_DATADIR := $(DPKG_BUILDTREE)/src
DPKG_ENV += \
  DPKG_DATADIR="$(DPKG_DATADIR)"
endif

export PATH
PATH = $(DPKG_PATH)

DPKG_OPTIONS =
DPKG_DIVERT_OPTIONS =

ifdef DPKG_AS_ROOT
DPKG_INSTDIR = /
ifeq ($(shell id -u),0)
BEROOT := env $(DPKG_ENV)
else
DPKG_ENV += \
  LD_PRELOAD="$(LD_PRELOAD)" \
  LD_LIBRARY_PATH="$(LD_LIBRARY_PATH)"
BEROOT := sudo -E env $(DPKG_ENV)
endif
else
DPKG_INSTDIR = $(CURDIR)/../dpkginst
DPKG_OPTIONS += \
 --force-script-chrootless \
 --force-not-root \
 # EOL
BEROOT := env $(DPKG_ENV)

DPKG_DIVERT_OPTIONS += \
  --instdir="$(DPKG_INSTDIR)" \
  # EOL
endif

DPKG_OPTIONS += \
  --force-unsafe-io \
  --instdir="$(DPKG_INSTDIR)" \
  --no-debsig --log=/dev/null \
  # EOL

ifneq (,$(filter debug,$(DPKG_TESTSUITE_OPTIONS)))
DPKG_OPTIONS += -D77777
endif

# Always use a local db.
DPKG_ADMINDIR = $(CURDIR)/../dpkgdb
DPKG_COMMON_OPTIONS = --admindir="$(DPKG_ADMINDIR)"

DPKG = dpkg $(DPKG_COMMON_OPTIONS) $(DPKG_OPTIONS)
DPKG_INSTALL = $(BEROOT) $(DPKG) -i
DPKG_UNPACK = $(BEROOT) $(DPKG) --unpack
DPKG_CONFIGURE = $(BEROOT) $(DPKG) --configure
DPKG_REMOVE = $(BEROOT) $(DPKG) -r
DPKG_PURGE = $(BEROOT) $(DPKG) -P
DPKG_VERIFY = $(DPKG) -V
DPKG_DEB = dpkg-deb $(DPKG_DEB_OPTIONS)
DPKG_DIVERT = dpkg-divert $(DPKG_COMMON_OPTIONS) $(DPKG_DIVERT_OPTIONS)
DPKG_DIVERT_ADD = $(BEROOT) $(DPKG_DIVERT) --add
DPKG_DIVERT_DEL = $(BEROOT) $(DPKG_DIVERT) --remove
DPKG_SPLIT = dpkg-split $(DPKG_SPLIT_OPTIONS)
DPKG_BUILD_DEB = $(DPKG_DEB) --root-owner-group -b
DPKG_QUERY = dpkg-query $(DPKG_COMMON_OPTIONS) $(DPKG_QUERY_OPTIONS)
DPKG_TRIGGER = dpkg-trigger $(DPKG_COMMON_OPTIONS) $(DPKG_TRIGGER_OPTIONS)

PKG_STATUS = $(DPKG_QUERY) -f '$${Status}' -W

DEB = $(addsuffix .deb,$(TESTS_DEB))

# Common test patterns to use with $(call foo,args...)
stdout_is = test "`$(1)`" = "$(2)"
stdout_has = $(1) | grep -qE "$(2)"
stderr_is = test "`$(1) 2>&1 1>/dev/null`" = "$(2)"
stderr_has = $(1) 2>&1 1>/dev/null | grep -qE "$(2)"
pkg_is_installed = $(call stdout_is,$(PKG_STATUS) $(1),install ok installed)
pkg_is_not_installed = $(call stdout_has,$(PKG_STATUS) $(1) 2>/dev/null, ok not-installed) || ! $(PKG_STATUS) $(1) >/dev/null 2>&1
pkg_status_is = $(call stdout_is,$(PKG_STATUS) $(1),$(2))
pkg_field_is = $(call stdout_is,$(DPKG_QUERY) -f '$${$(2)}' -W $(1),$(3))

%.deb: %
	$(DPKG_BUILD_DEB) $< $@

TEST_CASES :=

build: build-hook $(DEB)

test: build test-case test-clean

clean: clean-hook
	$(RM) $(DEB)

.PHONY: build-hook build test test-case test-clean clean-hook clean

# Most of the tests are serial in nature, as they perform package database
# changes, and the Makefile are written with that in mind. Avoid any
# surprises by explicitly disallowing parallel executions.
.NOTPARALLEL:
