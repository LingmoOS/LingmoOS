################################################################################
#
# pkcs11-helper
#
################################################################################

PKCS11_HELPER_VERSION = 1.30.0
PKCS11_HELPER_SOURCE = pkcs11-helper-$(PKCS11_HELPER_VERSION).tar.bz2
PKCS11_HELPER_SITE = https://github.com/OpenSC/pkcs11-helper/releases/download/pkcs11-helper-$(PKCS11_HELPER_VERSION)
PKCS11_HELPER_LICENSE = GPL-2.0 or BSD-3-Clause
PKCS11_HELPER_LICENSE_FILES = COPYING COPYING.BSD COPYING.GPL
PKCS11_HELPER_DEPENDENCIES = host-pkgconf
PKCS11_HELPER_INSTALL_STAGING = YES

PKCS11_HELPER_CONF_OPTS = \
	--disable-crypto-engine-polarssl \
	--disable-crypto-engine-cryptoapi

ifeq ($(LINGMO_TOOLCHAIN_HAS_THREADS),y)
PKCS11_HELPER_CONF_OPTS += --enable-slotevent --enable-threading
else
PKCS11_HELPER_CONF_OPTS += --disable-slotevent --disable-threading
endif

ifeq ($(LINGMO_PACKAGE_GNUTLS),y)
PKCS11_HELPER_DEPENDENCIES += gnutls
PKCS11_HELPER_CONF_OPTS += --enable-crypto-engine-gnutls
else
PKCS11_HELPER_CONF_OPTS += --disable-crypto-engine-gnutls
endif

ifeq ($(LINGMO_PACKAGE_LIBNSS),y)
PKCS11_HELPER_DEPENDENCIES += libnss
PKCS11_HELPER_CONF_OPTS += --enable-crypto-engine-nss
else
PKCS11_HELPER_CONF_OPTS += --disable-crypto-engine-nss
endif

ifeq ($(LINGMO_PACKAGE_MBEDTLS),y)
PKCS11_HELPER_DEPENDENCIES += mbedtls
PKCS11_HELPER_CONF_OPTS += --enable-crypto-engine-mbedtls
else
PKCS11_HELPER_CONF_OPTS += --disable-crypto-engine-mbedtls
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
PKCS11_HELPER_DEPENDENCIES += openssl
PKCS11_HELPER_CONF_OPTS += \
	--enable-openssl \
	--enable-crypto-engine-openssl
else
PKCS11_HELPER_CONF_OPTS += \
	--disable-openssl \
	--disable-crypto-engine-openssl
endif

$(eval $(autotools-package))
