################################################################################
#
# trinity
#
################################################################################

TRINITY_VERSION = 1.9
TRINITY_SITE = http://codemonkey.org.uk/projects/trinity
TRINITY_SOURCE = trinity-$(TRINITY_VERSION).tar.xz
TRINITY_LICENSE = GPL-2.0
TRINITY_LICENSE_FILES = COPYING
TRINITY_CPE_ID_VALID = YES

TRINITY_LDFLAGS = $(TARGET_LDFLAGS)

ifeq ($(LINGMO_PACKAGE_LIBEXECINFO),y)
TRINITY_DEPENDENCIES += libexecinfo
TRINITY_LDFLAGS += -lexecinfo
endif

define TRINITY_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_CONFIGURE_OPTS) ./configure)
endef

define TRINITY_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) LDFLAGS="$(TRINITY_LDFLAGS)"
endef

define TRINITY_INSTALL_TARGET_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		LDFLAGS="$(TRINITY_LDFLAGS)" DESTDIR=$(TARGET_DIR)/usr install
endef

# Install helper scripts
define TRINITY_INSTALL_HELPER_SCRIPTS
	mkdir -p $(TARGET_DIR)/usr/libexec/trinity
	cp -p $(@D)/scripts/* $(TARGET_DIR)/usr/libexec/trinity/
endef
TRINITY_POST_INSTALL_TARGET_HOOKS += TRINITY_INSTALL_HELPER_SCRIPTS

$(eval $(generic-package))
