################################################################################
#
# pngquant
#
################################################################################

PNGQUANT_VERSION = 2.18.0
PNGQUANT_SOURCE = pngquant-$(PNGQUANT_VERSION)-src.tar.gz
PNGQUANT_SITE = https://pngquant.org
PNGQUANT_LICENSE = GPL-3.0+
PNGQUANT_LICENSE_FILES = COPYRIGHT
PNGQUANT_CPE_ID_VENDOR = pngquant
HOST_PNGQUANT_DEPENDENCIES = host-libpng
PNGQUANT_DEPENDENCIES = libpng

ifeq ($(LINGMO_PACKAGE_LCMS2),y)
PNGQUANT_DEPENDENCIES += lcms2
endif

define PNGQUANT_CONFIGURE_CMDS
	(cd $(@D) && \
		$(TARGET_CONFIGURE_OPTS) \
		./configure --prefix=/usr \
		$(if $(LINGMO_PACKAGE_LCMS2),--with-lcms2,--without-lcms2) \
		$(if $(LINGMO_X86_CPU_HAS_SSE),--enable-sse,--disable-sse) \
		$(TARGET_CONFIGURE_OPTS) \
	)
endef

define PNGQUANT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define PNGQUANT_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR="$(TARGET_DIR)" -C $(@D) install
endef

define HOST_PNGQUANT_CONFIGURE_CMDS
	(cd $(@D) && \
		$(HOST_CONFIGURE_OPTS) \
		CC=$(HOSTCC_NOCCACHE) \
		./configure --prefix=$(HOST_DIR) \
		--without-lcms2 \
	)
endef

define HOST_PNGQUANT_BUILD_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D)
endef

define HOST_PNGQUANT_INSTALL_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D) install
endef

$(eval $(generic-package))
$(eval $(host-generic-package))
