################################################################################
#
# highway
#
################################################################################

HIGHWAY_VERSION = 1.1.0
HIGHWAY_SITE = $(call github,google,highway,$(HIGHWAY_VERSION))
HIGHWAY_LICENSE = Apache-2.0 or BSD-3-Clause
HIGHWAY_LICENSE_FILES = LICENSE LICENSE-BSD3
HIGHWAY_INSTALL_STAGING = YES

HIGHWAY_CXXFLAGS = $(TARGET_CXXFLAGS)

ifeq ($(LINGMO_PACKAGE_HIGHWAY_CONTRIB),y)
HIGHWAY_CONF_OPTS += -DHWY_ENABLE_CONTRIB=ON
else
HIGHWAY_CONF_OPTS += -DHWY_ENABLE_CONTRIB=OFF
endif

ifeq ($(LINGMO_PACKAGE_HIGHWAY_EXAMPLES),y)
HIGHWAY_CONF_OPTS += -DHWY_ENABLE_EXAMPLES=ON
# Examples are not installed by cmake. This binary can be useful for
# quick testing and debug.
define HIGHWAY_INSTALL_EXAMPLES
	$(INSTALL) -m 0755 \
		$(@D)/examples/hwy_benchmark \
		$(TARGET_DIR)/usr/bin/hwy_benchmark
endef
HIGHWAY_POST_INSTALL_TARGET_HOOKS += HIGHWAY_INSTALL_EXAMPLES
else
HIGHWAY_CONF_OPTS += -DHWY_ENABLE_EXAMPLES=OFF
endif

ifeq ($(LINGMO_ARM_FPU_VFPV4),y)
HIGHWAY_CONF_OPTS += -DHWY_CMAKE_ARM7=ON
else
# Highway Armv7 Neon support requires in fact vfpv4 / neon v2. When we
# are in a vfpv3 case (e.g. Cortex-A8, Cortex-A9) this flag need to be
# set to off.
HIGHWAY_CONF_OPTS += -DHWY_CMAKE_ARM7=OFF
endif

ifeq ($(LINGMO_RISCV_32),y)
HIGHWAY_CONF_OPTS += -DHWY_CMAKE_RVV=OFF
endif

# Workaround for gcc bug 104028 on m68k.
# See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=104028
ifeq ($(LINGMO_TOOLCHAIN_HAS_GCC_BUG_104028),y)
HIGHWAY_CXXFLAGS += -O0
endif

HIGHWAY_CONF_OPTS += \
	-DCMAKE_CXX_FLAGS="$(HIGHWAY_CXXFLAGS)"

$(eval $(cmake-package))
