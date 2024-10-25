################################################################################
#
# qt5xmlpatterns
#
################################################################################

QT5XMLPATTERNS_VERSION = 5a1948ddc05bf44017ac12bd5c2b9bc79fbcb9a2
QT5XMLPATTERNS_SITE = $(QT5_SITE)/qtxmlpatterns/-/archive/$(QT5XMLPATTERNS_VERSION)
QT5XMLPATTERNS_SOURCE = qtxmlpatterns-$(QT5XMLPATTERNS_VERSION).tar.bz2
QT5XMLPATTERNS_INSTALL_STAGING = YES
QT5XMLPATTERNS_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
QT5XMLPATTERNS_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
QT5XMLPATTERNS_SYNC_QT_HEADERS = YES

ifeq ($(LINGMO_PACKAGE_QT5DECLARATIVE),y)
QT5XMLPATTERNS_DEPENDENCIES += qt5declarative
endif

ifeq ($(LINGMO_PACKAGE_QT5BASE_EXAMPLES),y)
QT5XMLPATTERNS_LICENSE += , BSD-3-Clause (examples)
endif

ifeq ($(LINGMO_TOOLCHAIN_HAS_GCC_BUG_90620),y)
QT5XMLPATTERNS_CONF_OPTS += "QMAKE_CXXFLAGS+=-O0"
endif

$(eval $(qmake-package))
