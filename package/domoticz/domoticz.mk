################################################################################
#
# domoticz
#
################################################################################

DOMOTICZ_VERSION = 2024.4
DOMOTICZ_SITE = $(call github,domoticz,domoticz,$(DOMOTICZ_VERSION))
DOMOTICZ_LICENSE = GPL-3.0
DOMOTICZ_LICENSE_FILES = License.txt
DOMOTICZ_CPE_ID_VENDOR = domoticz
DOMOTICZ_DEPENDENCIES = \
	boost \
	cereal \
	host-pkgconf \
	jsoncpp \
	libcurl \
	lua \
	minizip-zlib \
	mosquitto \
	openssl \
	sqlite \
	zlib

# Disable precompiled header as it needs cmake >= 3.16
DOMOTICZ_CONF_OPTS = -DUSE_PRECOMPILED_HEADER=OFF

# Due to the dependency on mosquitto, domoticz depends on
# !LINGMO_STATIC_LIBS so set USE_STATIC_BOOST and USE_OPENSSL_STATIC to OFF
DOMOTICZ_CONF_OPTS += \
	-DUSE_STATIC_BOOST=OFF \
	-DUSE_OPENSSL_STATIC=OFF

# Do not use any built-in libraries which are enabled by default for
# jsoncpp, fmt, minizip, sqlite and mqtt
DOMOTICZ_CONF_OPTS += \
	-DUSE_BUILTIN_JSONCPP=OFF \
	-DUSE_BUILTIN_MINIZIP=OFF \
	-DUSE_BUILTIN_SQLITE=OFF \
	-DUSE_BUILTIN_MQTT=OFF

ifeq ($(LINGMO_PACKAGE_LIBEXECINFO),y)
DOMOTICZ_DEPENDENCIES += libexecinfo
DOMOTICZ_CONF_OPTS += -DEXECINFO_LIBRARIES=-lexecinfo
endif

ifeq ($(LINGMO_PACKAGE_LIBUSB),y)
DOMOTICZ_DEPENDENCIES += libusb
DOMOTICZ_CONF_OPTS += -DWITH_LIBUSB=ON
else
DOMOTICZ_CONF_OPTS += -DWITH_LIBUSB=OFF
endif

ifeq ($(LINGMO_PACKAGE_OPENZWAVE),y)
DOMOTICZ_DEPENDENCIES += openzwave

# Due to the dependency on mosquitto, domoticz depends on
# !LINGMO_STATIC_LIBS so set USE_STATIC_OPENZWAVE to OFF otherwise
# domoticz will not find the openzwave library as it searches by
# default a static library.
DOMOTICZ_CONF_OPTS += -DUSE_STATIC_OPENZWAVE=OFF
endif

ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
DOMOTICZ_DEPENDENCIES += python3
DOMOTICZ_CONF_OPTS += -DUSE_PYTHON=ON
else
DOMOTICZ_CONF_OPTS += -DUSE_PYTHON=OFF
endif

# Install domoticz in a dedicated directory (/opt/domoticz) as
# domoticz expects by default that all its subdirectories (www,
# Config, scripts, ...) are in the binary directory.
DOMOTICZ_TARGET_DIR = /opt/domoticz
DOMOTICZ_CONF_OPTS += -DCMAKE_INSTALL_PREFIX=$(DOMOTICZ_TARGET_DIR)

# Delete License.txt and updatedomo files installed by domoticz in target
# directory
# Do not delete History.txt as it is used in source code
define DOMOTICZ_REMOVE_UNNEEDED_FILES
	$(RM) $(TARGET_DIR)/$(DOMOTICZ_TARGET_DIR)/License.txt
	$(RM) $(TARGET_DIR)/$(DOMOTICZ_TARGET_DIR)/updatedomo
endef

DOMOTICZ_POST_INSTALL_TARGET_HOOKS += DOMOTICZ_REMOVE_UNNEEDED_FILES

# Use dedicated init scripts for systemV and systemd instead of using
# domoticz.sh as it is not compatible with buildroot init system
define DOMOTICZ_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/domoticz/S99domoticz \
		$(TARGET_DIR)/etc/init.d/S99domoticz
endef

define DOMOTICZ_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 644 package/domoticz/domoticz.service \
		$(TARGET_DIR)/usr/lib/systemd/system/domoticz.service
endef

$(eval $(cmake-package))
