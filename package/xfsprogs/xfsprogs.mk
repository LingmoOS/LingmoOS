################################################################################
#
# xfsprogs
#
################################################################################

XFSPROGS_VERSION = 6.4.0
XFSPROGS_SITE = $(LINGMO_KERNEL_MIRROR)/linux/utils/fs/xfs/xfsprogs
XFSPROGS_SOURCE = xfsprogs-$(XFSPROGS_VERSION).tar.xz
XFSPROGS_LICENSE = GPL-2.0, GPL-2.0+, LGPL-2.1 (libhandle, few headers)
XFSPROGS_LICENSE_FILES = LICENSES/GPL-2.0 LICENSES/LGPL-2.1

XFSPROGS_DEPENDENCIES = inih liburcu util-linux

XFSPROGS_CONF_ENV = ac_cv_header_aio_h=yes ac_cv_lib_rt_lio_listio=yes PLATFORM="linux"
XFSPROGS_CONF_OPTS = \
	--enable-lib64=no \
	--enable-gettext=no \
	INSTALL_USER=root \
	INSTALL_GROUP=root \
	--enable-static

ifeq ($(LINGMO_PACKAGE_ICU),y)
XFSPROGS_DEPENDENCIES += icu
XFSPROGS_CONF_OPTS += --enable-libicu
else
XFSPROGS_CONF_OPTS += --disable-libicu
endif

ifeq ($(LINGMO_OPTIMIZE_0),y)
XFSPROGS_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -O1"
endif

XFSPROGS_INSTALL_TARGET_OPTS = DIST_ROOT=$(TARGET_DIR) install

$(eval $(autotools-package))
