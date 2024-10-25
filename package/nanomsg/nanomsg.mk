################################################################################
#
# nanomsg
#
################################################################################

NANOMSG_VERSION = 1.2
NANOMSG_SITE = $(call github,nanomsg,nanomsg,$(NANOMSG_VERSION))
NANOMSG_INSTALL_STAGING = YES
NANOMSG_LICENSE = MIT
NANOMSG_LICENSE_FILES = COPYING
NANOMSG_CONF_OPTS = -DNN_ENABLE_DOC=OFF -DNN_TESTS=OFF

ifeq ($(LINGMO_STATIC_LIBS),y)
NANOMSG_CONF_OPTS += -DNN_STATIC_LIB=ON
endif

ifeq ($(LINGMO_PACKAGE_NANOMSG_TOOLS),y)
NANOMSG_CONF_OPTS += -DNN_TOOLS=ON
else
NANOMSG_CONF_OPTS += -DNN_TOOLS=OFF
endif

$(eval $(cmake-package))
