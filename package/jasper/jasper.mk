################################################################################
#
# jasper
#
################################################################################

JASPER_VERSION = 2.0.33
JASPER_SITE = https://github.com/jasper-software/jasper/releases/download/version-$(JASPER_VERSION)
JASPER_INSTALL_STAGING = YES
JASPER_LICENSE = JasPer-2.0
JASPER_LICENSE_FILES = LICENSE
JASPER_CPE_ID_VALID = YES
JASPER_SUPPORTS_IN_SOURCE_BUILD = NO
JASPER_CONF_OPTS = \
	-DJAS_ENABLE_DOC=OFF \
	-DJAS_ENABLE_PROGRAMS=OFF

ifeq ($(LINGMO_STATIC_LIBS),y)
JASPER_CONF_OPTS += -DJAS_ENABLE_SHARED=OFF
endif

ifeq ($(LINGMO_PACKAGE_JPEG),y)
JASPER_CONF_OPTS += -DJAS_ENABLE_LIBJPEG=ON
JASPER_DEPENDENCIES += jpeg
else
JASPER_CONF_OPTS += -DJAS_ENABLE_LIBJPEG=OFF
endif

JASPER_CFLAGS = $(TARGET_CFLAGS)

ifeq ($(LINGMO_TOOLCHAIN_HAS_GCC_BUG_85180),y)
JASPER_CFLAGS += -O0
endif

JASPER_CONF_OPTS += -DCMAKE_C_FLAGS="$(JASPER_CFLAGS)"

$(eval $(cmake-package))
