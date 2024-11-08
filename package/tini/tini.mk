################################################################################
#
# tini
#
################################################################################

TINI_VERSION = 0.19.0
TINI_SITE = $(call github,krallin,tini,v$(TINI_VERSION))
TINI_LICENSE = MIT
TINI_LICENSE_FILES = LICENSE
TINI_CPE_ID_VALID = YES

TINI_CFLAGS = $(TARGET_CFLAGS) \
	-static \
	-DTINI_VERSION=\"$(TINI_VERSION)\" \
	-DTINI_GIT=\"\"

ifeq ($(LINGMO_PACKAGE_TINI_MINIMAL),y)
TINI_CFLAGS += -DTINI_MINIMAL
endif

define TINI_CONFIGURE_CMDS
	printf "#pragma once\n" > $(@D)/src/tiniConfig.h
endef

define TINI_BUILD_CMDS
	mkdir -p $(@D)/bin
	$(TARGET_CC) $(TINI_CFLAGS) \
		-o $(@D)/bin/tini $(@D)/src/tini.c
endef

define TINI_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/bin/tini $(TARGET_DIR)/usr/bin/tini
	mkdir -p $(TARGET_DIR)/usr/libexec/docker
	ln -sf ../../bin/tini $(TARGET_DIR)/usr/libexec/docker/docker-init
endef

# Tini's CMakeLists.txt is not suitable for Buildroot.
$(eval $(generic-package))
