################################################################################
#
# libsrtp
#
################################################################################

LIBSRTP_VERSION = 2.5.0
LIBSRTP_SITE = $(call github,cisco,libsrtp,v$(LIBSRTP_VERSION))
LIBSRTP_INSTALL_STAGING = YES
LIBSRTP_LICENSE = BSD-3-Clause
LIBSRTP_LICENSE_FILES = LICENSE
LIBSRTP_CPE_ID_VENDOR = cisco

ifeq ($(LINGMO_STATIC_LIBS),y)
LIBSRTP_MAKE_OPTS = libsrtp2.a
else ifeq ($(LINGMO_SHARED_LIBS),y)
LIBSRTP_MAKE_OPTS = shared_library
else
LIBSRTP_MAKE_OPTS = libsrtp2.a shared_library
endif

LIBSRTP_DEPENDENCIES = host-pkgconf

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
LIBSRTP_DEPENDENCIES += openssl
LIBSRTP_CONF_OPTS += --disable-nss --enable-openssl
else ifeq ($(LINGMO_PACKAGE_LIBNSS),y)
LIBSRTP_DEPENDENCIES += libnss
LIBSRTP_CONF_OPTS += --enable-nss --disable-openssl
else
LIBSRTP_CONF_OPTS += --disable-nss --disable-openssl
endif

$(eval $(autotools-package))
