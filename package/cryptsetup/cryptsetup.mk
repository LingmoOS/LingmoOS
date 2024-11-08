################################################################################
#
# cryptsetup
#
################################################################################

CRYPTSETUP_VERSION_MAJOR = 2.7
CRYPTSETUP_VERSION = $(CRYPTSETUP_VERSION_MAJOR).1
CRYPTSETUP_SOURCE = cryptsetup-$(CRYPTSETUP_VERSION).tar.xz
CRYPTSETUP_SITE = $(LINGMO_KERNEL_MIRROR)/linux/utils/cryptsetup/v$(CRYPTSETUP_VERSION_MAJOR)
CRYPTSETUP_DEPENDENCIES = \
	lvm2 popt host-pkgconf json-c libargon2 \
	$(if $(LINGMO_PACKAGE_LIBICONV),libiconv) \
	$(if $(LINGMO_PACKAGE_UTIL_LINUX_LIBS),util-linux-libs,util-linux) \
	$(TARGET_NLS_DEPENDENCIES)
CRYPTSETUP_LICENSE = GPL-2.0+ (programs), LGPL-2.1+ (library)
CRYPTSETUP_LICENSE_FILES = COPYING COPYING.LGPL
CRYPTSETUP_CPE_ID_VALID = YES
CRYPTSETUP_INSTALL_STAGING = YES

CRYPTSETUP_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) $(TARGET_NLS_LIBS)"
CRYPTSETUP_CONF_OPTS += --enable-blkid --enable-libargon2 --disable-asciidoc

# cryptsetup uses OpenSSL by default, but can be configured to use libgcrypt,
# nettle, libnss or kernel crypto modules instead
ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
CRYPTSETUP_DEPENDENCIES += openssl
CRYPTSETUP_CONF_OPTS += --with-crypto_backend=openssl
else ifeq ($(LINGMO_PACKAGE_LIBGCRYPT),y)
CRYPTSETUP_DEPENDENCIES += libgcrypt
CRYPTSETUP_CONF_ENV += LIBGCRYPT_CONFIG=$(STAGING_DIR)/usr/bin/libgcrypt-config
CRYPTSETUP_CONF_OPTS += --with-crypto_backend=gcrypt
else ifeq ($(LINGMO_PACKAGE_NETTLE),y)
CRYPTSETUP_DEPENDENCIES += nettle
CRYPTSETUP_CONF_OPTS += --with-crypto_backend=nettle
else ifeq ($(LINGMO_PACKAGE_LIBNSS),y)
CRYPTSETUP_DEPENDENCIES += libnss
CRYPTSETUP_CONF_OPTS += --with-crypto_backend=nss
else
CRYPTSETUP_CONF_OPTS += --with-crypto_backend=kernel
endif

ifeq ($(LINGMO_PACKAGE_LIBSSH),y)
CRYPTSETUP_DEPENDENCIES += \
	$(if $(LINGMO_PACKAGE_ARGP_STANDALONE),argp-standalone) \
	libssh
CRYPTSETUP_CONF_OPTS += --enable-ssh-token
else
CRYPTSETUP_CONF_OPTS += --disable-ssh-token
endif

ifeq ($(LINGMO_PACKAGE_SYSTEMD),y)
CRYPTSETUP_CONF_OPTS += --with-tmpfilesdir=/usr/lib/tmpfiles.d
endif

HOST_CRYPTSETUP_DEPENDENCIES = \
	host-pkgconf \
	host-lvm2 \
	host-popt \
	host-util-linux \
	host-json-c \
	host-openssl

HOST_CRYPTSETUP_CONF_OPTS = --with-crypto_backend=openssl \
	--disable-kernel_crypto \
	--disable-ssh-token \
	--enable-blkid \
	--with-tmpfilesdir=no \
	--disable-asciidoc

$(eval $(autotools-package))
$(eval $(host-autotools-package))
