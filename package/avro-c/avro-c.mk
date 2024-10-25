################################################################################
#
# avro-c
#
################################################################################

# When updating the version, please also update python-avro
AVRO_C_VERSION = 1.11.3
AVRO_C_SITE = https://www-eu.apache.org/dist/avro/avro-$(AVRO_C_VERSION)/c
AVRO_C_LICENSE = Apache-2.0
AVRO_C_LICENSE_FILES = LICENSE
AVRO_C_INSTALL_STAGING = YES
AVRO_C_DEPENDENCIES = host-pkgconf jansson

ifeq ($(LINGMO_PACKAGE_SNAPPY),y)
AVRO_C_DEPENDENCIES += snappy
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
AVRO_C_DEPENDENCIES += zlib
endif

ifeq ($(LINGMO_PACKAGE_XZ),y)
AVRO_C_DEPENDENCIES += xz
endif

$(eval $(cmake-package))
