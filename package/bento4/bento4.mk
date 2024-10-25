################################################################################
#
# bento4
#
################################################################################

BENTO4_VERSION = 1.6.0-639-7-Omega
BENTO4_SITE = $(call github,xbmc,Bento4,$(BENTO4_VERSION))
BENTO4_INSTALL_STAGING = YES
BENTO4_LICENSE = GPL-2.0+
BENTO4_LICENSE_FILES = Documents/LICENSE.txt
BENTO4_CPE_ID_VENDOR = axiosys

# Source/C++/Core/Ap4Config.h
ifeq ($(LINGMO_ENDIAN),"BIG")
BENTO4_BYTE_ORDER = 0
else
BENTO4_BYTE_ORDER = 1
endif

BENTO4_CONF_OPTS += \
	-DBUILD_APPS=OFF \
	-DCMAKE_CXX_FLAGS="$(TARGET_CXXFLAGS) -std=c++11 -fPIC -DAP4_PLATFORM_BYTE_ORDER=$(BENTO4_BYTE_ORDER)"

$(eval $(cmake-package))
