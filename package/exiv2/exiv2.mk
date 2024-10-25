################################################################################
#
# exiv2
#
################################################################################

EXIV2_VERSION = 0.27.6
EXIV2_SOURCE = exiv2-$(EXIV2_VERSION)-Source.tar.gz
EXIV2_SITE = https://github.com/Exiv2/exiv2/releases/download/v$(EXIV2_VERSION)
EXIV2_INSTALL_STAGING = YES
EXIV2_LICENSE = GPL-2.0+
EXIV2_LICENSE_FILES = COPYING
EXIV2_CPE_ID_VENDOR = exiv2

EXIV2_CONF_OPTS += \
	-DBUILD_WITH_STACK_PROTECTOR=OFF \
	-DEXIV2_BUILD_SAMPLES=OFF

ifeq ($(LINGMO_PACKAGE_EXIV2_LENSDATA),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_LENSDATA=ON
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_LENSDATA=OFF
endif

ifeq ($(LINGMO_PACKAGE_EXIV2_PNG),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_PNG=ON
EXIV2_DEPENDENCIES += zlib
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_PNG=OFF
endif

ifeq ($(LINGMO_PACKAGE_EXIV2_XMP),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_XMP=ON
EXIV2_DEPENDENCIES += expat
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_XMP=OFF
endif

EXIV2_DEPENDENCIES += $(TARGET_NLS_DEPENDENCIES)

ifeq ($(LINGMO_SYSTEM_ENABLE_NLS),y)
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_NLS=ON
else
EXIV2_CONF_OPTS += -DEXIV2_ENABLE_NLS=OFF
endif

$(eval $(cmake-package))
