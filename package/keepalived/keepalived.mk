################################################################################
#
# keepalived
#
################################################################################

KEEPALIVED_VERSION = 2.2.8
KEEPALIVED_SITE = http://www.keepalived.org/software
KEEPALIVED_DEPENDENCIES = host-pkgconf openssl
KEEPALIVED_LICENSE = GPL-2.0+
KEEPALIVED_LICENSE_FILES = COPYING
KEEPALIVED_CPE_ID_VENDOR = keepalived
KEEPALIVED_CONF_OPTS = --disable-hardening

ifeq ($(LINGMO_PACKAGE_JSON_C),y)
KEEPALIVED_DEPENDENCIES += json-c
KEEPALIVED_CONF_OPTS += --enable-json
else
KEEPALIVED_CONF_OPTS += --disable-json
endif

ifeq ($(LINGMO_PACKAGE_LIBGLIB2),y)
KEEPALIVED_DEPENDENCIES += libglib2
KEEPALIVED_CONF_OPTS += --enable-dbus
else
KEEPALIVED_CONF_OPTS += --disable-dbus
endif

ifeq ($(LINGMO_PACKAGE_LIBNL)$(LINGMO_PACKAGE_LIBNFNETLINK),yy)
KEEPALIVED_DEPENDENCIES += libnl libnfnetlink
KEEPALIVED_CONF_OPTS += --enable-libnl
else
KEEPALIVED_CONF_OPTS += --disable-libnl
endif

ifeq ($(LINGMO_PACKAGE_IPTABLES),y)
KEEPALIVED_DEPENDENCIES += iptables
KEEPALIVED_CONF_OPTS += --enable-iptables
# ipset support only makes sense when iptables support is enabled.
ifeq ($(LINGMO_PACKAGE_IPSET),y)
KEEPALIVED_DEPENDENCIES += ipset
KEEPALIVED_CONF_OPTS += --enable-libipset
else
KEEPALIVED_CONF_OPTS += --disable-libipset
endif
else
KEEPALIVED_CONF_OPTS += --disable-iptables
endif

ifeq ($(LINGMO_PACKAGE_LIBNFTNL),y)
KEEPALIVED_DEPENDENCIES += libnftnl
KEEPALIVED_CONF_OPTS += --enable-nftables
else
KEEPALIVED_CONF_OPTS += --disable-nftables
endif

ifeq ($(LINGMO_TOOLCHAIN_GCC_AT_LEAST_4_9),)
KEEPALIVED_CONF_OPTS += --disable-track-process
endif

$(eval $(autotools-package))
