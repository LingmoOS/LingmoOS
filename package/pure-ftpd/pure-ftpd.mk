################################################################################
#
# pure-ftpd
#
################################################################################

PURE_FTPD_VERSION = 1.0.51
PURE_FTPD_SITE = https://download.pureftpd.org/pub/pure-ftpd/releases
PURE_FTPD_SOURCE = pure-ftpd-$(PURE_FTPD_VERSION).tar.bz2
PURE_FTPD_LICENSE = ISC
PURE_FTPD_LICENSE_FILES = COPYING
PURE_FTPD_CPE_ID_VENDOR = pureftpd

PURE_FTPD_CONF_OPTS = \
	--with-altlog \
	--with-puredb

ifeq ($(LINGMO_PACKAGE_ELFUTILS),y)
PURE_FTPD_DEPENDENCIES += elfutils
endif

ifeq ($(LINGMO_PACKAGE_LIBCAP),y)
PURE_FTPD_CONF_OPTS += --with-capabilities
PURE_FTPD_DEPENDENCIES += libcap
else
PURE_FTPD_CONF_OPTS += --without-capabilities
endif

ifeq ($(LINGMO_PACKAGE_LIBSODIUM),y)
PURE_FTPD_DEPENDENCIES += libsodium
endif

ifeq ($(LINGMO_PACKAGE_MARIADB),y)
PURE_FTPD_CONF_OPTS += --with-mysql=$(STAGING_DIR)/usr
PURE_FTPD_DEPENDENCIES += mariadb
else
PURE_FTPD_CONF_OPTS += --without-mysql
endif

ifeq ($(LINGMO_PACKAGE_OPENLDAP),y)
PURE_FTPD_CONF_OPTS += --with-ldap
PURE_FTPD_DEPENDENCIES += openldap
else
PURE_FTPD_CONF_OPTS += --without-ldap
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
PURE_FTPD_CONF_OPTS += --with-tls
PURE_FTPD_DEPENDENCIES += host-pkgconf openssl
PURE_FTPD_CONF_ENV += LIBS=`$(PKG_CONFIG_HOST_BINARY) --libs openssl`
else
PURE_FTPD_CONF_OPTS += --without-tls
endif

ifeq ($(LINGMO_PACKAGE_POSTGRESQL),y)
PURE_FTPD_CONF_OPTS += --with-pgsql=$(STAGING_DIR)/usr
PURE_FTPD_DEPENDENCIES += postgresql
else
PURE_FTPD_CONF_OPTS += --without-pgsql
endif

ifeq ($(LINGMO_TOOLCHAIN_SUPPORTS_PIE),)
PURE_FTPD_CONF_ENV += ax_cv_check_cflags___fPIE=no ax_cv_check_ldflags___fPIE=no
endif

ifeq ($(LINGMO_PACKAGE_PURE_FTPD_FTPWHO),y)
PURE_FTPD_CONF_OPTS += --with-ftpwho
endif

ifeq ($(LINGMO_PACKAGE_PURE_FTPD_QUOTAS),y)
PURE_FTPD_CONF_OPTS += --with-quotas
endif

ifeq ($(LINGMO_PACKAGE_PURE_FTPD_UPLOADSCRIPT),y)
PURE_FTPD_CONF_OPTS += --with-uploadscript
endif

ifeq ($(LINGMO_PACKAGE_LINUX_PAM),y)
PURE_FTPD_CONF_OPTS += --with-pam
PURE_FTPD_DEPENDENCIES += linux-pam
else
PURE_FTPD_CONF_OPTS += --without-pam
endif

$(eval $(autotools-package))
