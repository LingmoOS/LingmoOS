################################################################################
#
# libzip
#
################################################################################

LIBZIP_VERSION = 1.10.1
LIBZIP_SITE = https://libzip.org/download
LIBZIP_SOURCE = libzip-$(LIBZIP_VERSION).tar.xz
LIBZIP_LICENSE = BSD-3-Clause
LIBZIP_LICENSE_FILES = LICENSE
LIBZIP_CPE_ID_VENDOR = libzip
LIBZIP_INSTALL_STAGING = YES
LIBZIP_DEPENDENCIES = zlib
HOST_LIBZIP_DEPENDENCIES = host-zlib

ifeq ($(LINGMO_PACKAGE_BZIP2),y)
LIBZIP_DEPENDENCIES += bzip2
LIBZIP_CONF_OPTS += -DENABLE_BZIP2=ON
else
LIBZIP_CONF_OPTS += -DENABLE_BZIP2=OFF
endif

ifeq ($(LINGMO_PACKAGE_GNUTLS),y)
LIBZIP_DEPENDENCIES += gnutls
LIBZIP_CONF_OPTS += -DENABLE_GNUTLS=ON
else
LIBZIP_CONF_OPTS += -DENABLE_GNUTLS=OFF
endif

ifeq ($(LINGMO_PACKAGE_MBEDTLS),y)
LIBZIP_DEPENDENCIES += mbedtls
LIBZIP_CONF_OPTS += -DENABLE_MBEDTLS=ON
else
LIBZIP_CONF_OPTS += -DENABLE_MBEDTLS=OFF
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
LIBZIP_DEPENDENCIES += openssl
LIBZIP_CONF_OPTS += -DENABLE_OPENSSL=ON
else
LIBZIP_CONF_OPTS += -DENABLE_OPENSSL=OFF
endif

ifeq ($(LINGMO_TOOLCHAIN_USES_GLIBC),)
LIBZIP_DEPENDENCIES += musl-fts
LIBZIP_CONF_OPTS += -DCMAKE_EXE_LINKER_FLAGS=-lfts
endif

ifeq ($(LINGMO_PACKAGE_XZ),y)
LIBZIP_DEPENDENCIES += xz
LIBZIP_CONF_OPTS += -DENABLE_LZMA=ON
else
LIBZIP_CONF_OPTS += -DENABLE_LZMA=OFF
endif

ifeq ($(LINGMO_PACKAGE_ZSTD),y)
LIBZIP_DEPENDENCIES += zstd
LIBZIP_CONF_OPTS += -DENABLE_ZSTD=ON
else
LIBZIP_CONF_OPTS += -DENABLE_ZSTD=OFF
endif

$(eval $(cmake-package))
$(eval $(host-cmake-package))
