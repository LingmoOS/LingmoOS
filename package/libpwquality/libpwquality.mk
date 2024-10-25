################################################################################
#
# libpwquality
#
################################################################################

LIBPWQUALITY_VERSION = 1.4.5
LIBPWQUALITY_SOURCE = libpwquality-$(LIBPWQUALITY_VERSION).tar.bz2
LIBPWQUALITY_SITE = https://github.com/libpwquality/libpwquality/releases/download/libpwquality-$(LIBPWQUALITY_VERSION)
LIBPWQUALITY_LICENSE = BSD-3-Clause or GPL-2.0+
LIBPWQUALITY_INSTALL_STAGING = YES
LIBPWQUALITY_DEPENDENCIES = cracklib
LIBPWQUALITY_LICENSE_FILES = COPYING
# We're patching python/Makefile.am
LIBPWQUALITY_AUTORECONF = YES

ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
LIBPWQUALITY_CONF_OPTS += --enable-python-bindings
LIBPWQUALITY_DEPENDENCIES += python3 host-python-setuptools
LIBPWQUALITY_MAKE_ENV += $(PKG_PYTHON_SETUPTOOLS_ENV)
else
LIBPWQUALITY_CONF_OPTS += --disable-python-bindings
endif

ifeq ($(LINGMO_PACKAGE_LINUX_PAM),y)
LIBPWQUALITY_CONF_OPTS += --enable-pam
LIBPWQUALITY_DEPENDENCIES += linux-pam
else
LIBPWQUALITY_CONF_OPTS += --disable-pam
endif

# Static link with cracklib requires -lz if zlib is enabled
ifeq ($(LINGMO_STATIC_LIBS)$(LINGMO_PACKAGE_ZLIB),yy)
LIBPWQUALITY_CONF_ENV += LIBS="-lz"
endif

ifeq ($(LINGMO_PACKAGE_LIBPWQUALITY_TOOLS),)
define LIBPWQUALITY_REMOVE_TOOLS
	rm -f $(TARGET_DIR)/usr/bin/pwmake
	rm -f $(TARGET_DIR)/usr/bin/pwscore
endef

LIBPWQUALITY_POST_INSTALL_TARGET_HOOKS += LIBPWQUALITY_REMOVE_TOOLS
endif

$(eval $(autotools-package))
