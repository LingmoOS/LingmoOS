################################################################################
#
# giflib
#
################################################################################

GIFLIB_VERSION = 5.2.2
GIFLIB_SITE = http://downloads.sourceforge.net/project/giflib
GIFLIB_INSTALL_STAGING = YES
GIFLIB_LICENSE = MIT
GIFLIB_LICENSE_FILES = COPYING
GIFLIB_CPE_ID_VALID = YES

ifeq ($(LINGMO_STATIC_LIBS),y)
GIFLIB_BUILD_LIBS = static-lib
GIFLIB_INSTALL_LIBS = install-static-lib
else ifeq ($(LINGMO_SHARED_LIBS),y)
GIFLIB_BUILD_LIBS = shared-lib
GIFLIB_INSTALL_LIBS = install-shared-lib
else
GIFLIB_BUILD_LIBS = static-lib shared-lib
GIFLIB_INSTALL_LIBS = install-lib
endif

define GIFLIB_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) $(GIFLIB_BUILD_LIBS)
endef

define HOST_GIFLIB_BUILD_CMDS
	$(HOST_CONFIGURE_OPTS) $(MAKE) -C $(@D) shared-lib
endef

define GIFLIB_INSTALL_STAGING_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) DESTDIR=$(STAGING_DIR) \
		PREFIX=/usr install-include $(GIFLIB_INSTALL_LIBS)
endef

define GIFLIB_INSTALL_TARGET_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) \
		PREFIX=/usr install-include $(GIFLIB_INSTALL_LIBS)
endef

define HOST_GIFLIB_INSTALL_CMDS
	$(HOST_CONFIGURE_OPTS) $(MAKE) -C $(@D) DESTDIR=$(HOST_DIR) \
		PREFIX=/usr install-include install-shared-lib
endef

$(eval $(generic-package))
$(eval $(host-generic-package))
