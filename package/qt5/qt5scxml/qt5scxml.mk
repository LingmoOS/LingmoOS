################################################################################
#
# qt5scxml
#
################################################################################

QT5SCXML_VERSION = e8727aabe55526956295407d27317ec15e12b283
QT5SCXML_SITE = $(QT5_SITE)/qtscxml/-/archive/$(QT5SCXML_VERSION)
QT5SCXML_SOURCE = qtscxml-$(QT5SCXML_VERSION).tar.bz2
QT5SCXML_DEPENDENCIES = qt5declarative
QT5SCXML_INSTALL_STAGING = YES
QT5SCXML_SYNC_QT_HEADERS = YES

QT5SCXML_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
QT5SCXML_LICENSE_FILES = LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
ifeq ($(LINGMO_PACKAGE_QT5BASE_EXAMPLES),y)
QT5SCXML_LICENSE += , BSD-3-Clause (examples)
endif

$(eval $(qmake-package))
