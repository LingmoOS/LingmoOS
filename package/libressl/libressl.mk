################################################################################
#
# libressl
#
################################################################################

LIBRESSL_VERSION = 3.8.4
LIBRESSL_SITE = https://ftp.openbsd.org/pub/OpenBSD/LibreSSL
LIBRESSL_LICENSE = ISC (new additions), OpenSSL or SSLeay (original OpenSSL code)
LIBRESSL_LICENSE_FILES = COPYING
LIBRESSL_CPE_ID_VENDOR = openbsd
LIBRESSL_INSTALL_STAGING = YES
LIBRESSL_CONF_OPTS = -DLIBRESSL_TESTS=OFF -DOPENSSLDIR="/etc/ssl"

ifeq ($(LINGMO_PACKAGE_LIBRESSL_BIN),)
define LIBRESSL_REMOVE_BIN
	$(RM) -f $(TARGET_DIR)/usr/bin/openssl
endef
LIBRESSL_POST_INSTALL_TARGET_HOOKS += LIBRESSL_REMOVE_BIN
endif

$(eval $(cmake-package))
