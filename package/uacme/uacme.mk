################################################################################
#
# uacme
#
################################################################################

UACME_VERSION = 1.7.5
# Released versions are on branch upstream/latest, tagged as
# upstream/X.Y.Z Do not use vX.Y.Z tags from master, as they do not
# include .tarball-version
UACME_SITE = $(call github,ndilieto,uacme,upstream/$(UACME_VERSION))
UACME_LICENSE = GPL-3.0+
UACME_LICENSE_FILES = COPYING
UACME_DEPENDENCIES = libcurl

# 0001-Fix-cross-compilation.patch touches configure.ac
UACME_AUTORECONF = YES

UACME_CONF_ENV = ac_cv_prog_cc_c99='-std=gnu99' LIBS="$(UACME_LIBS)"

ifeq ($(LINGMO_PACKAGE_GNUTLS),y)
UACME_CONF_OPTS += --with-gnutls
UACME_DEPENDENCIES += gnutls
else ifeq ($(LINGMO_PACKAGE_MBEDTLS),y)
ifeq ($(LINGMO_PACKAGE_MBEDTLS_COMPRESSION),y)
    UACME_LIBS += -lz
endif
UACME_CONF_OPTS += --with-mbedtls
UACME_DEPENDENCIES += mbedtls
else ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
UACME_CONF_OPTS += --with-openssl
UACME_DEPENDENCIES += openssl
endif

ifeq ($(LINGMO_PACKAGE_UACME_UALPN),y)
UACME_DEPENDENCIES += libev
UACME_CONF_OPTS += --with-ualpn
else
UACME_CONF_OPTS += --without-ualpn
endif

$(eval $(autotools-package))
