QT -= gui
QT += dbus

TEMPLATE = lib
DEFINES += LIBLINGMOAPPWIDGETMANAGER_LIBRARY
TARGET = lingmo-appwidget-manager
VERSION = 0.0.0
CONFIG += c++11
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
include(../../liblingmo-appwidget-common/liblingmo-appwidget-common.pri)
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
    kappwidgetmanager.cpp

HEADERS += \
    liblingmo-appwidget-manager_global.h \
    kappwidgetmanager.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
    apifile.files = *.h
    apifile.path  = /usr/include/lingmo/
}
!isEmpty(target.path): INSTALLS += target apifile
