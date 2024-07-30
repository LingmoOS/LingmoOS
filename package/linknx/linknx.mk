################################################################################
#
# linknx
#
################################################################################

LINKNX_VERSION = 0.0.1.38
LINKNX_SITE = $(call github,linknx,linknx,$(LINKNX_VERSION))
LINKNX_LICENSE = GPL-2.0+
LINKNX_LICENSE_FILES = LICENSE
LINKNX_INSTALL_STAGING = YES
# We're patching configure.ac
LINKNX_AUTORECONF = YES
LINKNX_CONF_OPTS = \
	--without-cppunit \
	--without-pth-test \
	--with-pth=$(STAGING_DIR)/usr \
	--disable-smtp

# add host-gettext for AM_ICONV macro
LINKNX_DEPENDENCIES = \
	host-gettext \
	host-pkgconf \
	libpthsem \
	$(if $(LINGMO_PACKAGE_ARGP_STANDALONE),argp-standalone) \
	$(if $(LINGMO_PACKAGE_LIBICONV),libiconv) \
	$(TARGET_NLS_DEPENDENCIES)

LINKNX_CONF_ENV = LIBS=$(TARGET_NLS_LIBS)

ifeq ($(LINGMO_PACKAGE_LIBCURL),y)
LINKNX_CONF_OPTS += --with-libcurl=$(STAGING_DIR)/usr
LINKNX_DEPENDENCIES += libcurl
else
LINKNX_CONF_OPTS += --without-libcurl
endif

ifeq ($(LINGMO_PACKAGE_LOG4CPP),y)
LINKNX_CONF_OPTS += --with-log4cpp
LINKNX_DEPENDENCIES += log4cpp
else
LINKNX_CONF_OPTS += --without-log4cpp
endif

ifeq ($(LINGMO_PACKAGE_LUA),y)
LINKNX_CONF_OPTS += --with-lua
LINKNX_DEPENDENCIES += lua
else
LINKNX_CONF_OPTS += --without-lua
endif

ifeq ($(LINGMO_PACKAGE_MARIADB),y)
LINKNX_CONF_OPTS += --with-mysql=$(STAGING_DIR)/usr
LINKNX_DEPENDENCIES += mariadb
else
LINKNX_CONF_OPTS += --without-mysql
endif

$(eval $(autotools-package))
