################################################################################
#
# casync
#
################################################################################

CASYNC_VERSION = 0efa7abffe5fffbde8c457d3c8fafbdde0bb6e4f
CASYNC_SITE = $(call github,systemd,casync,$(CASYNC_VERSION))
CASYNC_LICENSE = LGPL-2.1+
CASYNC_LICENSE_FILES = LICENSE.LGPL2.1
CASYNC_DEPENDENCIES = acl libcurl openssl

CASYNC_CONF_OPTS = -Dman=false -Doss-fuzz=false -Dllvm-fuzz=false

ifeq ($(LINGMO_PACKAGE_BASH_COMPLETION),y)
CASYNC_DEPENDENCIES += bash-completion
# The default path is correct, leave it as-is
else
CASYNC_CONF_OPTS += -Dbashcompletiondir=no
endif

ifeq ($(LINGMO_PACKAGE_LIBFUSE),y)
CASYNC_DEPENDENCIES += libfuse
CASYNC_CONF_OPTS += -Dfuse=true
else
CASYNC_CONF_OPTS += -Dfuse=false
endif

ifeq ($(LINGMO_PACKAGE_LIBSELINUX),y)
CASYNC_DEPENDENCIES += libselinux
CASYNC_CONF_OPTS += -Dselinux=true
else
CASYNC_CONF_OPTS += -Dselinux=false
endif

ifeq ($(LINGMO_PACKAGE_HAS_UDEV),y)
CASYNC_DEPENDENCIES += udev
CASYNC_CONF_OPTS += -Dudev=true
else
CASYNC_CONF_OPTS += -Dudev=false
endif

ifeq ($(LINGMO_PACKAGE_ZSTD),y)
CASYNC_DEPENDENCIES += zstd
CASYNC_CONF_OPTS += -Dlibzstd=enabled
else
CASYNC_CONF_OPTS += -Dlibzstd=disabled
endif

ifeq ($(LINGMO_PACKAGE_XZ),y)
CASYNC_DEPENDENCIES += xz
CASYNC_CONF_OPTS += -Dliblzma=enabled
else
CASYNC_CONF_OPTS += -Dliblzma=disabled
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
CASYNC_DEPENDENCIES += zlib
CASYNC_CONF_OPTS += -Dlibz=enabled
else
CASYNC_CONF_OPTS += -Dlibz=disabled
endif

$(eval $(meson-package))
