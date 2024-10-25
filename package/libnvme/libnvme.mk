################################################################################
#
# libnvme
#
################################################################################

LIBNVME_VERSION = 1.7.1
LIBNVME_SITE = $(call github,linux-nvme,libnvme,v$(LIBNVME_VERSION))
LIBNVME_LICENSE = LGPL-2.1
LIBNVME_LICENSE_FILES = COPYING
LIBNVME_INSTALL_STAGING = YES

LIBNVME_CONF_OPTS += \
	-Dtests=false

ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
LIBNVME_DEPENDENCIES += python3 host-swig
LIBNVME_CONF_OPTS += -Dpython=enabled
else
LIBNVME_CONF_OPTS += -Dpython=disabled
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
LIBNVME_DEPENDENCIES += openssl
LIBNVME_CONF_OPTS += -Dopenssl=enabled
else
LIBNVME_CONF_OPTS += -Dopenssl=disabled
endif

ifeq ($(LINGMO_PACKAGE_JSON_C),y)
LIBNVME_DEPENDENCIES += json-c
LIBNVME_CONF_OPTS += -Djson-c=enabled
else
LIBNVME_CONF_OPTS += -Djson-c=disabled
endif

ifeq ($(LINGMO_PACKAGE_KEYUTILS),y)
LIBNVME_DEPENDENCIES += keyutils
LIBNVME_CONF_OPTS += -Dkeyutils=enabled
else
LIBNVME_CONF_OPTS += -Dkeyutils=disabled
endif

$(eval $(meson-package))
