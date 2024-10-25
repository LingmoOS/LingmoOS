################################################################################
#
# axel
#
################################################################################

AXEL_VERSION = 2.17.13
AXEL_SITE = https://github.com/axel-download-accelerator/axel/releases/download/v$(AXEL_VERSION)
AXEL_SOURCE = axel-$(AXEL_VERSION).tar.xz
AXEL_LICENSE = GPL-2.0+
AXEL_LICENSE_FILES = COPYING
AXEL_CPE_ID_VALID = YES
AXEL_DEPENDENCIES = host-pkgconf $(TARGET_NLS_DEPENDENCIES)

# ac_cv_prog_cc_c99 is required for LINGMO_USE_WCHAR=n because the C99 test
# provided by autoconf relies on wchar_t.
AXEL_CONF_OPTS = \
	ac_cv_prog_cc_c99=-std=c99 \
	CFLAGS="$(TARGET_CFLAGS)"

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
AXEL_CONF_OPTS += --with-ssl=openssl
AXEL_DEPENDENCIES += openssl
else ifeq ($(LINGMO_PACKAGE_WOLFSSL_ALL),y)
AXEL_CONF_OPTS += --with-ssl=wolfssl
AXEL_DEPENDENCIES += wolfssl
else
AXEL_CONF_OPTS += --without-ssl
endif

$(eval $(autotools-package))
