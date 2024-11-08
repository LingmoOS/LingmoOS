################################################################################
#
# openjpeg
#
################################################################################

OPENJPEG_VERSION = 2.5.2
OPENJPEG_SITE = $(call github,uclouvain,openjpeg,v$(OPENJPEG_VERSION))
OPENJPEG_LICENSE = BSD-2-Clause
OPENJPEG_LICENSE_FILES = LICENSE
OPENJPEG_CPE_ID_VENDOR = uclouvain
OPENJPEG_INSTALL_STAGING = YES

OPENJPEG_DEPENDENCIES += $(if $(LINGMO_PACKAGE_ZLIB),zlib)
OPENJPEG_DEPENDENCIES += $(if $(LINGMO_PACKAGE_LIBPNG),libpng)
OPENJPEG_DEPENDENCIES += $(if $(LINGMO_PACKAGE_TIFF),tiff)
OPENJPEG_DEPENDENCIES += $(if $(LINGMO_PACKAGE_LCMS2),lcms2)

ifeq ($(LINGMO_TOOLCHAIN_HAS_THREADS),y)
OPENJPEG_CONF_OPTS += -DOPJ_USE_THREAD=ON
else
OPENJPEG_CONF_OPTS += -DOPJ_USE_THREAD=OFF
endif

$(eval $(cmake-package))
