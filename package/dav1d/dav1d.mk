################################################################################
#
# dav1d
#
################################################################################

DAV1D_VERSION = 1.3.0
DAV1D_SOURCE = dav1d-$(DAV1D_VERSION).tar.xz
DAV1D_SITE = https://download.videolan.org/pub/videolan/dav1d/$(DAV1D_VERSION)
DAV1D_LICENSE = BSD-2-Clause
DAV1D_LICENSE_FILES = COPYING
DAV1D_CPE_ID_VENDOR = videolan
DAV1D_INSTALL_STAGING = YES
DAV1D_CONF_OPTS = \
	-Denable_tests=false \
	-Denable_tools=false

ifeq ($(LINGMO_i386)$(LINGMO_x86_64),y)
DAV1D_DEPENDENCIES += host-nasm
endif

# ARM assembly requires v6+ ISA
ifeq ($(LINGMO_ARM_CPU_ARMV4)$(LINGMO_ARM_CPU_ARMV5)$(LINGMO_ARM_CPU_ARMV7M),y)
DAV1D_CONF_OPTS += -Denable_asm=false
endif

# Uses __atomic_fetch_add_4
ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
DAV1D_LDFLAGS += $(TARGET_LDFLAGS) -latomic
endif

$(eval $(meson-package))
