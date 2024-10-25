################################################################################
#
# zlib-ng
#
################################################################################

ZLIB_NG_VERSION = 2.1.3
ZLIB_NG_SITE = $(call github,zlib-ng,zlib-ng,$(ZLIB_NG_VERSION))
ZLIB_NG_LICENSE = Zlib
ZLIB_NG_LICENSE_FILES = LICENSE.md
ZLIB_NG_INSTALL_STAGING = YES
ZLIB_NG_PROVIDES = zlib

# Build with zlib compatible API, gzFile support and optimizations on
ZLIB_NG_CONF_OPTS += \
	-DCMAKE_C_COMPILER_TARGET=$(LINGMO_ARCH) \
	-DWITH_GZFILEOP=1 \
	-DWITH_OPTIM=1 \
	-DZLIB_COMPAT=1 \
	-DZLIB_ENABLE_TESTS=OFF

# Enable ACLE on ARM
ifeq ($(LINGMO_arm),y)
ZLIB_NG_CONF_OPTS += -DWITH_ACLE=1
endif

ifeq ($(LINGMO_ARM_CPU_HAS_NEON)$(LINGMO_aarch64),y)
ZLIB_NG_CONF_OPTS += -DWITH_NEON=ON
else
ZLIB_NG_CONF_OPTS += -DWITH_NEON=OFF
endif

ifeq ($(LINGMO_powerpc_power8),y)
ZLIB_NG_CONF_OPTS += -DWITH_POWER8=ON
else
ZLIB_NG_CONF_OPTS += -DWITH_POWER8=OFF
endif

ifeq ($(LINGMO_powerpc_power9),y)
ZLIB_NG_CONF_OPTS += -DWITH_POWER9=ON
else
ZLIB_NG_CONF_OPTS += -DWITH_POWER9=OFF
endif

$(eval $(cmake-package))
