################################################################################
#
# lightning
#
################################################################################

LIGHTNING_VERSION = 2.2.3
LIGHTNING_SITE = $(LINGMO_GNU_MIRROR)/lightning
LIGHTNING_LICENSE = LGPL-3.0+
LIGHTNING_LICENSE_FILES = COPYING.LESSER
LIGHTNING_INSTALL_STAGING = YES
LIGHTNING_CPE_ID_VENDOR = gnu

ifeq ($(LINGMO_PACKAGE_LIGHTNING_DISASSEMBLER),y)
LIGHTNING_DEPENDENCIES += binutils zlib
LIGHTNING_CONF_OPTS += --enable-disassembler
endif

$(eval $(autotools-package))
