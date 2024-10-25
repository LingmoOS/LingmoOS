################################################################################
#
# liblinear
#
################################################################################

LIBLINEAR_VERSION = 2.45
LIBLINEAR_SITE = http://www.csie.ntu.edu.tw/~cjlin/liblinear
LIBLINEAR_LICENSE = BSD-3-Clause
LIBLINEAR_LICENSE_FILES = COPYRIGHT
LIBLINEAR_INSTALL_STAGING = YES
LIBLINEAR_CFLAGS = $(TARGET_CFLAGS)

ifeq ($(LINGMO_SHARED_LIBS)$(LINGMO_SHARED_STATIC_LIBS),y)
# $1: destination directory
define LIBLINEAR_INSTALL_SHARED
	$(INSTALL) -m 0644 -D $(@D)/liblinear.so.5 $(1)/usr/lib/liblinear.so.5
	ln -sf liblinear.so.5 $(1)/usr/lib/liblinear.so
endef
LIBLINEAR_CFLAGS += -fPIC
endif

ifeq ($(LINGMO_STATIC_LIBS)$(LINGMO_SHARED_STATIC_LIBS),y)
# $1: destination directory
define LIBLINEAR_INSTALL_STATIC
	$(INSTALL) -m 0644 -D $(@D)/liblinear.a $(1)/usr/lib/liblinear.a
endef
endif

define LIBLINEAR_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) CFLAGS="$(LIBLINEAR_CFLAGS)" -C $(@D) \
		$(if $(LINGMO_SHARED_LIBS)$(LINGMO_SHARED_STATIC_LIBS),lib) \
		$(if $(LINGMO_STATIC_LIBS)$(LINGMO_SHARED_STATIC_LIBS),static-lib)
endef

define LIBLINEAR_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0644 -D $(@D)/linear.h $(STAGING_DIR)/usr/include/linear.h
	$(call LIBLINEAR_INSTALL_SHARED,$(STAGING_DIR))
	$(call LIBLINEAR_INSTALL_STATIC,$(STAGING_DIR))
endef

define LIBLINEAR_INSTALL_TARGET_CMDS
	$(call LIBLINEAR_INSTALL_SHARED,$(TARGET_DIR))
endef

$(eval $(generic-package))
