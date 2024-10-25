################################################################################
#
# gdbm
#
################################################################################

GDBM_VERSION = 1.23
GDBM_SITE = $(LINGMO_GNU_MIRROR)/gdbm
GDBM_LICENSE = GPL-3.0+
GDBM_LICENSE_FILES = COPYING
GDBM_INSTALL_STAGING = YES
GDBM_DEPENDENCIES = $(TARGET_NLS_DEPENDENCIES)

ifeq ($(LINGMO_PACKAGE_READLINE),y)
GDBM_CONF_OPTS += --with-readline
GDBM_DEPENDENCIES += readline
else
GDBM_CONF_OPTS += --without-readline
endif

$(eval $(autotools-package))
