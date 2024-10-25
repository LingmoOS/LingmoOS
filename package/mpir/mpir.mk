################################################################################
#
# mpir
#
################################################################################

MPIR_VERSION = 3.0.0
MPIR_SITE = http://www.mpir.org
MPIR_SOURCE = mpir-$(MPIR_VERSION).tar.bz2
MPIR_LICENSE = LGPL-3.0+
MPIR_LICENSE_FILES = COPYING.LIB
MPIR_INSTALL_STAGING = YES
MPIR_DEPENDENCIES = gmp host-yasm

ifeq ($(LINGMO_MIPS_NABI32),y)
MPIR_CONF_OPTS += ABI=n32
endif

ifeq ($(LINGMO_MIPS_NABI64),y)
MPIR_CONF_OPTS += ABI=64
endif

# The optimized ARM assembly code uses ARM-only (i.e not Thumb1/2
# compatible) instructions.
ifeq ($(LINGMO_arm)$(LINGMO_armeb):$(LINGMO_ARM_CPU_HAS_ARM),y:)
MPIR_CONF_ENV += MPN_PATH="generic"
endif

# Optimized mips32/mips64 code not suitable for mips32r6/mips64r6
ifeq ($(LINGMO_MIPS_CPU_MIPS32R6)$(LINGMO_MIPS_CPU_MIPS64R6),y)
MPIR_CONF_ENV += MPN_PATH="generic"
endif

# Optimized powerpc64 code is not suitable for powerpc64le
ifeq ($(LINGMO_powerpc64le),y)
MPIR_CONF_ENV += MPN_PATH="generic"
endif

ifeq ($(LINGMO_ARM_INSTRUCTIONS_THUMB),y)
MPIR_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -marm"
endif

$(eval $(autotools-package))
