################################################################################
#
# erofs-utils
#
################################################################################

EROFS_UTILS_VERSION = 1.5
EROFS_UTILS_SITE = https://git.kernel.org/pub/scm/linux/kernel/git/xiang/erofs-utils.git/snapshot
EROFS_UTILS_LICENSE = GPL-2.0+, GPL-2.0+ or Apache-2.0 (liberofs)
EROFS_UTILS_LICENSE_FILES = COPYING LICENSES/Apache-2.0 LICENSES/GPL-2.0

# From a git tree: no generated autotools files
EROFS_UTILS_AUTORECONF = YES

EROFS_UTILS_DEPENDENCIES = host-pkgconf util-linux

ifeq ($(LINGMO_PACKAGE_EROFS_UTILS_LZ4),y)
EROFS_UTILS_DEPENDENCIES += lz4
EROFS_UTILS_CONF_OPTS += --enable-lz4
else
EROFS_UTILS_CONF_OPTS += --disable-lz4
endif

ifeq ($(LINGMO_PACKAGE_LIBSELINUX),y)
EROFS_UTILS_CONF_OPTS += --with-selinux
EROFS_UTILS_DEPENDENCIES += libselinux
else
EROFS_UTILS_CONF_OPTS += --without-selinux
endif

ifeq ($(LINGMO_PACKAGE_EROFS_UTILS_EROFSFUSE),y)
EROFS_UTILS_CONF_OPTS += --enable-fuse
EROFS_UTILS_DEPENDENCIES += libfuse
else
EROFS_UTILS_CONF_OPTS += --disable-fuse
endif

HOST_EROFS_UTILS_DEPENDENCIES = host-pkgconf host-util-linux host-lz4
HOST_EROFS_UTILS_CONF_OPTS += --enable-lz4 --disable-fuse --without-selinux

$(eval $(autotools-package))
$(eval $(host-autotools-package))
