TARGET = udisks2-qt5
QT += core dbus
QT -= gui
TEMPLATE = lib

isEmpty(VERSION): VERSION = 0.0.1

SOURCES += \
    $$PWD/ddiskdevice.cpp \
    $$PWD/ddiskmanager.cpp \
    $$PWD/udisks2_dbus_common.cpp \
    $$PWD/dblockdevice.cpp \
    $$PWD/dblockpartition.cpp \
    $$PWD/dudisksjob.cpp

udisk2.files = $$PWD/org.freedesktop.UDisks2.xml
udisk2.header_flags = -i $$PWD/udisks2_dbus_common.h -N

DBUS_INTERFACES += udisk2 $$PWD/org.freedesktop.UDisks2.ObjectManager.xml

HEADERS += \
    $$PWD/ddiskdevice.h \
    $$PWD/udisks2_dbus_common.h \
    $$PWD/ddiskmanager.h \
    $$PWD/dblockdevice.h \
    $$PWD/dblockpartition.h \
    $$PWD/dudisksjob.h

include($$PWD/private/private.pri)

INCLUDEPATH += $$PWD

OTHER_FILES += $$PWD/*.xml

isEmpty(PREFIX): PREFIX = /usr

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$PREFIX/lib
} else {
    target.path = $$LIB_INSTALL_DIR
}

isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/$$TARGET
} else {
    includes.path = $$INCLUDE_INSTALL_DIR
}

includes.files += $$PWD/*.h
includes_private.path = $$includes.path/private
includes_private.files += $$PWD/private/*.h

INSTALLS += includes includes_private target

CONFIG += create_pc create_prl no_install_prl

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_NAME = $$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = UDisks2 Library with Qt5
QMAKE_PKGCONFIG_INCDIR = $$includes.path
