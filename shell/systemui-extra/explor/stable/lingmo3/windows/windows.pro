#-------------------------------------------------
#
# Project created by QtCreator 2023-11-29T09:45:20
#
#-------------------------------------------------

QT       += network core widgets gui concurrent xml KWindowSystem dbus x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#QMAKE_CXXFLAGS += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label

include(../../../common.pri)
include(../../../libexplor-qt/libexplor-qt-header.pri)
include(../../../3rd-parties/SingleApplication/singleapplication.pri)
include(../../../src/windows/windows.pri)
include(../control/control.pri)
include(../../../plugin-iface/unstable/window-plugin-iface.pri)

DISTFILES += $$PWD/common.json
DEFINES += QAPPLICATION_CLASS=QApplication

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 gsettings-qt libcanberra libnotify udisks2 openssl dconf
LIBS +=-L$$PWD/../../../libexplor-qt/ -lgio-2.0 -lglib-2.0 -lX11 -llingmo-log4qt -lexplor
#CONFIG += c++11 link_pkgconfig no_keywords lrelease
CONFIG += debug link_pkgconfig plugin
TARGET = stable-main-window
TEMPLATE = lib
DEFINES += DEVELCONTROL_LIBRARY

CONFIG += c++11

contains(DEFINES, LINGMO_FILE_DIALOG) {
    PKGCONFIG += lingmosdk-qtwidgets
}

contains(DEFINES, LINGMO_SDK_SYSINFO) {
    PKGCONFIG += lingmosdk-sysinfo
}

contains(DEFINES, LINGMO_SDK_QT_WIDGETS) {
    PKGCONFIG += lingmosdk-qtwidgets
}

contains(DEFINES, LINGMO_SDK_WAYLANDHELPER) {
    PKGCONFIG += lingmosdk-waylandhelper
}

contains(DEFINES, LINGMO_SDK_SYSINFO) {
    PKGCONFIG += lingmosdk-sysinfo
}

schemes.files += org.lingmo.explor.settings.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/

WINDOW_PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/explor-main-window
DEFINES += WINDOW_PLUGIN_INSTALL_DIRS='\\"$${WINDOW_PLUGIN_INSTALL_DIRS}\\"'

QMAKE_CXXFLAGS += -execution-charset:utf-8

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main-window.cpp \
    main-window-factory.cpp \
    explor-main-window-style.cpp

HEADERS += \
    main-window-factory.h \
    main-window.h \
    explor-main-window-style.h \
    windows_global.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_LIBS]/explor-main-window
}
!isEmpty(target.path): INSTALLS += target
