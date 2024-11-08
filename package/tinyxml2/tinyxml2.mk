################################################################################
#
# tinyxml2
#
################################################################################

TINYXML2_VERSION = 10.0.0
TINYXML2_SITE = $(call github,leethomason,tinyxml2,$(TINYXML2_VERSION))
TINYXML2_LICENSE = Zlib
TINYXML2_LICENSE_FILES = LICENSE.txt
TINYXML2_INSTALL_STAGING = YES
TINYXML2_CPE_ID_VALID = YES

ifeq ($(LINGMO_STATIC_LIBS),y)
TINYXML2_CONF_OPTS += -DBUILD_STATIC_LIBS=ON
endif

$(eval $(cmake-package))
$(eval $(host-cmake-package))
