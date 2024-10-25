################################################################################
#
# snort3
#
################################################################################

SNORT3_VERSION = 3.1.40.0
SNORT3_SITE = $(call github,snort3,snort3,$(SNORT3_VERSION))
SNORT3_LICENSE = GPL-2.0
SNORT3_LICENSE_FILES = COPYING LICENSE

SNORT3_DEPENDENCIES = \
	host-pkgconf daq3 flex hwloc libdnet libpcap luajit openssl pcre zlib

SNORT3_CONF_OPTS = \
	-DENABLE_GDB=OFF \
	-DENABLE_STATIC_DAQ=OFF \
	-DMAKE_DOC=OFF

ifeq ($(LINGMO_PACKAGE_LIBICONV),y)
SNORT3_DEPENDENCIES += libiconv
SNORT3_CONF_OPTS += -DHAVE_ICONV=ON
endif

ifeq ($(LINGMO_PACKAGE_LIBTIRPC),y)
SNORT3_DEPENDENCIES += libtirpc
endif

ifeq ($(LINGMO_PACKAGE_SAFECLIB),y)
SNORT3_DEPENDENCIES += safeclib
SNORT3_CONF_OPTS += -DENABLE_SAFEC=ON
else
SNORT3_CONF_OPTS += -DENABLE_SAFEC=OFF
endif

ifeq ($(LINGMO_PACKAGE_UTIL_LINUX_LIBUUID),y)
SNORT3_DEPENDENCIES += util-linux
SNORT3_CONF_OPTS += -DHAVE_UUID=ON
else
SNORT3_CONF_OPTS += -DHAVE_UUID=OFF
endif

ifeq ($(LINGMO_PACKAGE_XZ),y)
SNORT3_DEPENDENCIES += xz
SNORT3_CONF_OPTS += -DHAVE_LZMA=ON
else
SNORT3_CONF_OPTS += -DHAVE_LZMA=OFF
endif

# Uses __atomic_load_8
ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
SNORT3_CONF_OPTS += -DCMAKE_EXE_LINKER_FLAGS=-latomic
endif

$(eval $(cmake-package))
