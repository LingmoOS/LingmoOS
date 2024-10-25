################################################################################
#
# wavpack
#
################################################################################

WAVPACK_VERSION = 5.6.0
WAVPACK_SITE = \
	https://github.com/dbry/WavPack/releases/download/$(WAVPACK_VERSION)
WAVPACK_SOURCE = wavpack-$(WAVPACK_VERSION).tar.xz
WAVPACK_INSTALL_STAGING = YES
WAVPACK_DEPENDENCIES = $(if $(LINGMO_ENABLE_LOCALE),,libiconv)
WAVPACK_LICENSE = BSD-3-Clause
WAVPACK_LICENSE_FILES = COPYING
WAVPACK_CPE_ID_VENDOR = wavpack

ifeq ($(LINGMO_PACKAGE_LIBICONV),y)
WAVPACK_CONF_OPTS += LIBS=-liconv
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
WAVPACK_DEPENDENCIES += openssl
WAVPACK_CONF_OPTS += --enable-libcrypto
else
WAVPACK_CONF_OPTS += --disable-libcrypto
endif

# WavPack "autodetects" CPU type to enable ASM code. However, the assembly code
# for ARM is written for ARMv7 only and building WavPack for an ARM-non-v7
# architecture will fail. We explicitly enable ASM for the supported
# architectures x86, x64 and ARMv7 and disable it for all others.
ifeq ($(LINGMO_i386)$(LINGMO_x86_64)$(LINGMO_ARM_CPU_ARMV7A),y)
WAVPACK_CONF_OPTS += --enable-asm
else
WAVPACK_CONF_OPTS += --disable-asm
endif

$(eval $(autotools-package))
