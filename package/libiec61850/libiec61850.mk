################################################################################
#
# libiec61850
#
################################################################################

LIBIEC61850_VERSION = 1.5.3
LIBIEC61850_SITE = $(call github,mz-automation,libiec61850,v$(LIBIEC61850_VERSION))
LIBIEC61850_INSTALL_STAGING = YES
LIBIEC61850_LICENSE = GPL-3.0+
LIBIEC61850_LICENSE_FILES = COPYING
LIBIEC61850_CPE_ID_VENDOR = mz-automation
LIBIEC61850_CONF_OPTS = -DBUILD_PYTHON_BINDINGS=OFF

ifeq ($(LINGMO_PACKAGE_MBEDTLS),y)
LIBIEC61850_DEPENDENCIES += mbedtls
LIBIEC61850_CONF_OPTS += -DCONFIG_USE_EXTERNAL_MBEDTLS_DYNLIB=ON
else
LIBIEC61850_CONF_OPTS += -DCONFIG_USE_EXTERNAL_MBEDTLS_DYNLIB=OFF
endif

$(eval $(cmake-package))
