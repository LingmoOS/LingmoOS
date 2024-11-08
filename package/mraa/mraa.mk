################################################################################
#
# mraa
#
################################################################################

MRAA_VERSION = 2.2.0
MRAA_SITE = $(call github,eclipse,mraa,v$(MRAA_VERSION))
MRAA_LICENSE = MIT
MRAA_LICENSE_FILES = COPYING
MRAA_INSTALL_STAGING = YES

ifeq ($(LINGMO_i386),y)
MRAA_ARCH = i386
else ifeq ($(LINGMO_x86_64),y)
MRAA_ARCH = x86_64
else ifeq ($(LINGMO_arm)$(LINGMO_armeb),y)
MRAA_ARCH = arm
else ifeq ($(LINGMO_aarch64)$(LINGMO_aarch64_be),y)
MRAA_ARCH = aarch64
else ifeq ($(LINGMO_mips)$(LINGMO_mipsel)$(LINGMO_mips64)$(LINGMO_mips64el),y)
MRAA_ARCH = mips
endif

# USBPLAT only makes sense with FTDI4222, which requires the ftd2xx library,
# which doesn't exist in buildroot
# Disable C++ as it is used only by FTDI4222 and tests
MRAA_CONF_OPTS += \
	-DBUILDARCH=$(MRAA_ARCH) \
	-DBUILDCPP=OFF \
	-DBUILDSWIG=OFF \
	-DUSBPLAT=OFF \
	-DFTDI4222=OFF \
	-DENABLEEXAMPLES=OFF \
	-DBUILDTESTS=OFF

ifeq ($(LINGMO_PACKAGE_JSON_C),y)
MRAA_CONF_OPTS += -DJSONPLAT=ON
MRAA_DEPENDENCIES += json-c
else
MRAA_CONF_OPTS += -DJSONPLAT=OFF
endif

$(eval $(cmake-package))
