################################################################################
#
# fastd
#
################################################################################

FASTD_VERSION = 22
FASTD_SITE = https://github.com/NeoRaider/fastd/releases/download/v$(FASTD_VERSION)
FASTD_SOURCE = fastd-$(FASTD_VERSION).tar.xz
FASTD_LICENSE = BSD-2-Clause
FASTD_LICENSE_FILES = COPYRIGHT
FASTD_CPE_ID_VERSION = $(FASTD_VERSION).0
FASTD_DEPENDENCIES = host-bison host-pkgconf libuecc libsodium

ifeq ($(LINGMO_PACKAGE_LIBCAP),y)
FASTD_CONF_OPTS += -Dcapabilities=enabled
FASTD_DEPENDENCIES += libcap
else
FASTD_CONF_OPTS += -Dcapabilities=disabled
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
FASTD_CONF_OPTS += -Dcipher_aes128-ctr=enabled
FASTD_DEPENDENCIES += openssl
else
FASTD_CONF_OPTS += -Dcipher_aes128-ctr=disabled
endif

ifeq ($(LINGMO_PACKAGE_FASTD_STATUS_SOCKET),y)
FASTD_CONF_OPTS += -Dstatus_socket=enabled
FASTD_DEPENDENCIES += json-c
else
FASTD_CONF_OPTS += -Dstatus_socket=disabled
endif

ifeq ($(LINGMO_PACKAGE_FASTD_OFFLOAD_L2TP),y)
FASTD_CONF_OPTS += -Doffload_l2tp=enabled -Dlibmnl_builtin=false
FASTD_DEPENDENCIES += libmnl
else
FASTD_CONF_OPTS += -Doffload_l2tp=disabled
endif

ifeq ($(LINGMO_INIT_SYSTEMD),y)
FASTD_CONF_OPTS += -Dsystemd=enabled
else
FASTD_CONF_OPTS += -Dsystemd=disabled
endif

ifeq ($(LINGMO_ENABLE_LTO),y)
FASTD_CONF_OPTS += -Db_lto=true
else
FASTD_CONF_OPTS += -Db_lto=false
endif

$(eval $(meson-package))
