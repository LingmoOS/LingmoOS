################################################################################
#
# nut
#
################################################################################

NUT_VERSION = 2.8.1
NUT_SITE = https://github.com/networkupstools/nut/releases/download/v$(NUT_VERSION)
NUT_LICENSE = GPL-2.0+, GPL-3.0+ (python scripts), GPL/Artistic (perl client)
NUT_LICENSE_FILES = COPYING LICENSE-GPL2 LICENSE-GPL3
NUT_SELINUX_MODULES = apache nut
NUT_INSTALL_STAGING = YES
NUT_DEPENDENCIES = host-pkgconf
# We're patching m4/nut_compiler_family.m4
# We're patching m4/nut_check_python.m4
NUT_AUTORECONF = YES

# Put the PID files in a read-write place (/var/run is a tmpfs)
# since the default location (/var/state/ups) maybe readonly.
NUT_CONF_OPTS = \
	--with-altpidpath=/var/run/upsd \
	--with-dev \
	--without-doc \
	--without-python \
	--without-python2 \
	--with-user=nut \
	--with-group=nut

NUT_CONF_ENV = \
	ax_cv_check_cflags__Werror__Wno_unknown_warning_option=no \
	ax_cv_check_cxxflags__Werror__Wno_unknown_warning_option=no \
	ac_cv_func_strcasecmp=yes \
	ac_cv_func_strdup=yes \
	ac_cv_func_strncasecmp=yes \
	ax_cv__printf_string_null=yes

define NUT_USERS
	nut -1 nut -1 * - - - NUT user
endef

ifeq ($(call qstrip,$(LINGMO_PACKAGE_NUT_DRIVERS)),)
NUT_CONF_OPTS += --with-drivers=auto
else
NUT_CONF_OPTS += --with-drivers=$(LINGMO_PACKAGE_NUT_DRIVERS)
endif

ifeq ($(LINGMO_PACKAGE_AVAHI)$(LINGMO_PACKAGE_DBUS),yy)
NUT_DEPENDENCIES += avahi dbus
NUT_CONF_OPTS += --with-avahi
else
NUT_CONF_OPTS += --without-avahi
endif

ifeq ($(LINGMO_PACKAGE_FREEIPMI),y)
NUT_CONF_OPTS += --with-freeipmi
NUT_DEPENDENCIES += freeipmi
else
NUT_CONF_OPTS += --without-freeipmi
endif

# gd with support for png is required for the CGI
ifeq ($(LINGMO_PACKAGE_GD)$(LINGMO_PACKAGE_LIBPNG),yy)
NUT_DEPENDENCIES += gd libpng
NUT_CONF_OPTS += --with-cgi
else
NUT_CONF_OPTS += --without-cgi
endif

# nut-scanner needs libltdl, which is a wrapper around dlopen/dlsym,
# so is not available for static-only builds.
# There is no flag to directly enable/disable nut-scanner, it's done
# via the --enable/disable-libltdl flag.
ifeq ($(LINGMO_STATIC_LIBS):$(LINGMO_PACKAGE_LIBTOOL),:y)
NUT_DEPENDENCIES += libtool
NUT_CONF_OPTS += --with-libltdl
else
NUT_CONF_OPTS += --without-libltdl
endif

ifeq ($(LINGMO_PACKAGE_LIBUSB),y)
NUT_DEPENDENCIES += libusb
NUT_CONF_OPTS += --with-usb
else ifeq ($(LINGMO_PACKAGE_LIBUSB_COMPAT),y)
NUT_DEPENDENCIES += libusb-compat
NUT_CONF_OPTS += --with-usb
else
NUT_CONF_OPTS += --without-usb
endif

ifeq ($(LINGMO_PACKAGE_NEON_EXPAT)$(LINGMO_PACKAGE_NEON_LIBXML2),y)
NUT_DEPENDENCIES += neon
NUT_CONF_OPTS += --with-neon
else
NUT_CONF_OPTS += --without-neon
endif

ifeq ($(LINGMO_PACKAGE_NETSNMP),y)
NUT_DEPENDENCIES += netsnmp
NUT_CONF_OPTS += \
	--with-snmp \
	--with-net-snmp-config=$(STAGING_DIR)/usr/bin/net-snmp-config
else
NUT_CONF_OPTS += --without-snmp
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
NUT_DEPENDENCIES += openssl
NUT_CONF_OPTS += --with-ssl
else
NUT_CONF_OPTS += --without-ssl
endif

ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
NUT_DEPENDENCIES += python3
NUT_CONF_ENV += nut_cv_PYTHON3_SITE_PACKAGES=/usr/lib/python$(PYTHON3_VERSION_MAJOR)/site-packages
NUT_CONF_OPTS += \
	--with-pynut \
	--with-python3
else
NUT_CONF_OPTS += \
	--without-pynut \
	--without-python3
endif

$(eval $(autotools-package))
