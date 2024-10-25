################################################################################
#
# openipmi
#
################################################################################

OPENIPMI_VERSION = 2.0.34
OPENIPMI_SITE = https://sourceforge.net/projects/openipmi/files/OpenIPMI%202.0%20Library
OPENIPMI_SOURCE = OpenIPMI-$(OPENIPMI_VERSION).tar.gz
OPENIPMI_LICENSE = LGPL-2.0+, GPL-2.0+, BSD-3-Clause
OPENIPMI_LICENSE_FILES = COPYING.LIB COPYING COPYING.BSD
OPENIPMI_DEPENDENCIES = popt ncurses readline host-pkgconf
OPENIPMI_INSTALL_STAGING = YES
OPENIPMI_CONF_ENV = ac_cv_path_pkgprog="$(PKG_CONFIG_HOST_BINARY)"
OPENIPMI_CONF_OPTS = \
	--with-glib=no \
	--with-tcl=no \
	--with-perl=no \
	--with-python=no \
	--with-swig=no

ifeq ($(LINGMO_PACKAGE_GDBM),y)
OPENIPMI_DEPENDENCIES += gdbm
endif

ifeq ($(LINGMO_PACKAGE_LIBEXECINFO),y)
OPENIPMI_DEPENDENCIES += libexecinfo
OPENIPMI_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -lexecinfo"
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
OPENIPMI_DEPENDENCIES += openssl
OPENIPMI_CONF_OPTS += --with-openssl=yes
else
OPENIPMI_CONF_OPTS += --with-openssl=no
endif

ifeq ($(LINGMO_PACKAGE_NETSNMP),y)
OPENIPMI_DEPENDENCIES += netsnmp
OPENIPMI_CONF_OPTS += --with-ucdsnmp=yes
else
OPENIPMI_CONF_OPTS += --with-ucdsnmp=no
endif

$(eval $(autotools-package))
