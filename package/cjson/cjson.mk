################################################################################
#
# cjson
#
################################################################################

CJSON_VERSION = 1.7.18
CJSON_SITE = $(call github,DaveGamble,cjson,v$(CJSON_VERSION))
CJSON_INSTALL_STAGING = YES
CJSON_LICENSE = MIT
CJSON_LICENSE_FILES = LICENSE
CJSON_CPE_ID_VALID = YES
# Set ENABLE_CUSTOM_COMPILER_FLAGS to OFF in particular to disable
# -fstack-protector-strong which depends on LINGMO_TOOLCHAIN_HAS_SSP
CJSON_CONF_OPTS += \
	-DENABLE_CJSON_TEST=OFF \
	-DENABLE_CUSTOM_COMPILER_FLAGS=OFF

# If BUILD_SHARED_AND_STATIC_LIBS is set to OFF, cjson uses the
# standard BUILD_SHARED_LIBS option which is passed by the
# cmake-package infrastructure.
ifeq ($(LINGMO_SHARED_STATIC_LIBS),y)
CJSON_CONF_OPTS += -DBUILD_SHARED_AND_STATIC_LIBS=ON
else
CJSON_CONF_OPTS += -DBUILD_SHARED_AND_STATIC_LIBS=OFF
endif

ifeq ($(LINGMO_PACKAGE_CJSON_UTILS),y)
CJSON_CONF_OPTS += -DENABLE_CJSON_UTILS=ON
else
CJSON_CONF_OPTS += -DENABLE_CJSON_UTILS=OFF
endif

$(eval $(cmake-package))
