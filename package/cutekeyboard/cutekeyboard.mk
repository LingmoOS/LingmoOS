################################################################################
#
# cutekeyboard
#
################################################################################

CUTEKEYBOARD_VERSION = afacc3210b75d7e0de27dcc4c0f2bed0212cc4c7
CUTEKEYBOARD_SITE = $(call github,amarula,cutekeyboard,$(CUTEKEYBOARD_VERSION))
CUTEKEYBOARD_DEPENDENCIES = qt5declarative qt5quickcontrols2
CUTEKEYBOARD_INSTALL_STAGING = YES
CUTEKEYBOARD_LICENSE = MIT
CUTEKEYBOARD_LICENSE_FILES = LICENSE

ifeq ($(LINGMO_PACKAGE_CUTEKEYBOARD_EXAMPLES),y)
CUTEKEYBOARD_CONF_OPTS += CONFIG+=BUILD_EXAMPLES
endif

$(eval $(qmake-package))
