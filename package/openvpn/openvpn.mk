################################################################################
#
# openvpn
#
################################################################################

OPENVPN_VERSION = 2.6.11
OPENVPN_SITE = https://swupdate.openvpn.net/community/releases
OPENVPN_DEPENDENCIES = host-pkgconf libcap-ng
OPENVPN_LICENSE = GPL-2.0
OPENVPN_LICENSE_FILES = COPYRIGHT.GPL
OPENVPN_CPE_ID_VENDOR = openvpn
OPENVPN_SELINUX_MODULES = openvpn
OPENVPN_CONF_OPTS = \
	--disable-unit-tests \
	$(if $(LINGMO_STATIC_LIBS),--disable-plugins)
OPENVPN_CONF_ENV = NETSTAT=/bin/netstat

ifeq ($(LINGMO_PACKAGE_LIBNL)$(LINGMO_TOOLCHAIN_HEADERS_AT_LEAST_4_16),yy)
OPENVPN_CONF_OPTS += --enable-dco
OPENVPN_DEPENDENCIES += libnl
else
OPENVPN_CONF_OPTS += --disable-dco
endif

ifeq ($(LINGMO_PACKAGE_OPENVPN_SMALL),y)
OPENVPN_CONF_OPTS += \
	--enable-small \
	--disable-plugins
endif

ifeq ($(LINGMO_PACKAGE_OPENVPN_LZ4),y)
OPENVPN_DEPENDENCIES += lz4
else
OPENVPN_CONF_OPTS += --disable-lz4
endif

ifeq ($(LINGMO_PACKAGE_OPENVPN_LZO),y)
OPENVPN_DEPENDENCIES += lzo
else
OPENVPN_CONF_OPTS += --disable-lzo
endif

ifeq ($(LINGMO_PACKAGE_LIBSELINUX),y)
OPENVPN_DEPENDENCIES += libselinux
OPENVPN_CONF_OPTS += --enable-selinux
else
OPENVPN_CONF_OPTS += --disable-selinux
endif

ifeq ($(LINGMO_PACKAGE_LINUX_PAM),y)
OPENVPN_DEPENDENCIES += linux-pam
OPENVPN_CONF_OPTS += --enable-plugin-auth-pam
else
OPENVPN_CONF_OPTS += --disable-plugin-auth-pam
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
OPENVPN_DEPENDENCIES += openssl
OPENVPN_CONF_OPTS += --with-crypto-library=openssl
else ifeq ($(LINGMO_PACKAGE_MBEDTLS),y)
OPENVPN_DEPENDENCIES += mbedtls
OPENVPN_CONF_OPTS += --with-crypto-library=mbedtls
endif

ifeq ($(LINGMO_PACKAGE_PKCS11_HELPER),y)
OPENVPN_DEPENDENCIES += pkcs11-helper
OPENVPN_CONF_OPTS += --enable-pkcs11
else
OPENVPN_CONF_OPTS += --disable-pkcs11
endif

ifeq ($(LINGMO_PACKAGE_SYSTEMD),y)
OPENVPN_DEPENDENCIES += systemd
OPENVPN_CONF_OPTS += --enable-systemd
else
OPENVPN_CONF_OPTS += --disable-systemd
endif

define OPENVPN_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D package/openvpn/S60openvpn \
		$(TARGET_DIR)/etc/init.d/S60openvpn
endef

$(eval $(autotools-package))
