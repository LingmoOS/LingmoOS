################################################################################
#
# jemalloc
#
################################################################################

JEMALLOC_VERSION = 5.3.0
JEMALLOC_SOURCE = jemalloc-$(JEMALLOC_VERSION).tar.bz2
JEMALLOC_SITE = https://github.com/jemalloc/jemalloc/releases/download/$(JEMALLOC_VERSION)
JEMALLOC_LICENSE = BSD-2-Clause
JEMALLOC_LICENSE_FILES = COPYING
JEMALLOC_INSTALL_STAGING = YES
JEMALLOC_CONFIG_SCRIPTS = jemalloc-config

# gcc bug internal compiler error: in merge_overlapping_regs, at
# regrename.c:304. This bug is fixed since gcc 6.
ifeq ($(LINGMO_or1k):$(LINGMO_TOOLCHAIN_GCC_AT_LEAST_6),y:)
JEMALLOC_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -O0"
endif

$(eval $(autotools-package))
$(eval $(host-autotools-package))
