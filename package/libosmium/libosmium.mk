################################################################################
#
# libosmium
#
################################################################################

LIBOSMIUM_VERSION = 2.20.0
LIBOSMIUM_SITE = $(call github,osmcode,libosmium,v$(LIBOSMIUM_VERSION))
LIBOSMIUM_LICENSE = BSL-1.0
LIBOSMIUM_LICENSE_FILES = LICENSE
LIBOSMIUM_INSTALL_STAGING = YES
LIBOSMIUM_DEPENDENCIES = boost protozero lz4

ifeq ($(LINGMO_PACKAGE_BZIP2),y)
LIBOSMIUM_DEPENDENCIES += bzip2
endif

ifeq ($(LINGMO_PACKAGE_EXPAT),y)
LIBOSMIUM_DEPENDENCIES += expat
endif

ifeq ($(LINGMO_PACKAGE_LIBGEOS),y)
LIBOSMIUM_DEPENDENCIES += libgeos
endif

ifeq ($(LINGMO_PACKAGE_PROJ),y)
LIBOSMIUM_DEPENDENCIES += proj
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
LIBOSMIUM_DEPENDENCIES += zlib
endif

ifeq ($(LINGMO_PACKAGE_GDAL),y)
LIBOSMIUM_DEPENDENCIES += gdal
endif

$(eval $(cmake-package))
