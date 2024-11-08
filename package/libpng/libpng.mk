################################################################################
#
# libpng
#
################################################################################

LIBPNG_VERSION = 1.6.43
LIBPNG_SERIES = 16
LIBPNG_SOURCE = libpng-$(LIBPNG_VERSION).tar.xz
LIBPNG_SITE = http://downloads.sourceforge.net/project/libpng/libpng$(LIBPNG_SERIES)/$(LIBPNG_VERSION)
LIBPNG_LICENSE = Libpng-2.0
LIBPNG_LICENSE_FILES = LICENSE
LIBPNG_CPE_ID_VENDOR = libpng
LIBPNG_INSTALL_STAGING = YES
LIBPNG_DEPENDENCIES = host-pkgconf zlib
HOST_LIBPNG_DEPENDENCIES = host-pkgconf host-zlib
LIBPNG_CONFIG_SCRIPTS = libpng$(LIBPNG_SERIES)-config libpng-config
LIBPNG_CONF_OPTS = --disable-tools

ifeq ($(LINGMO_ARM_CPU_HAS_NEON)$(LINGMO_aarch64),y)
LIBPNG_CONF_OPTS += --enable-arm-neon
else
LIBPNG_CONF_OPTS += --disable-arm-neon
endif

ifeq ($(LINGMO_X86_CPU_HAS_SSE2),y)
LIBPNG_CONF_OPTS += --enable-intel-sse
else
LIBPNG_CONF_OPTS += --disable-intel-sse
endif

$(eval $(autotools-package))
$(eval $(host-autotools-package))
