################################################################################
#
# nettle
#
################################################################################

NETTLE_VERSION = 3.9.1
NETTLE_SITE = https://ftp.gnu.org/gnu/nettle
NETTLE_DEPENDENCIES = host-m4 gmp
NETTLE_INSTALL_STAGING = YES
NETTLE_LICENSE = Dual GPL-2.0+/LGPL-3.0+
NETTLE_LICENSE_FILES = COPYING.LESSERv3 COPYINGv2
NETTLE_CPE_ID_VALID = YES
# don't include openssl support for (unused) examples as it has problems
# with static linking
NETTLE_CONF_OPTS = --disable-openssl

HOST_NETTLE_DEPENDENCIES = host-m4 host-gmp

# ARM assembly requires v6+ ISA
ifeq ($(LINGMO_ARM_CPU_ARMV4)$(LINGMO_ARM_CPU_ARMV5)$(LINGMO_ARM_CPU_ARMV7M),y)
NETTLE_CONF_OPTS += --disable-assembler
endif

ifeq ($(LINGMO_ARM_CPU_HAS_NEON),y)
NETTLE_CONF_OPTS += --enable-arm-neon
else
NETTLE_CONF_OPTS += --disable-arm-neon
endif

$(eval $(autotools-package))
$(eval $(host-autotools-package))
