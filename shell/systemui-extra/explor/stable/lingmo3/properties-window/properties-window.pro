#-------------------------------------------------
#
# Project created by QtCreator 2023-11-29T09:45:20
#
#-------------------------------------------------

QT       += network core widgets gui concurrent xml KWindowSystem dbus x11extras

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label
include(../../../common.pri)
include(../../../plugin-iface/unstable/window-plugin-iface.pri)
include(controls/property-page.pri)
INCLUDEPATH += $$PWD/../../../libexplor-qt/file-operation
INCLUDEPATH += $$PWD/../../../libexplor-qt/file-launcher
INCLUDEPATH += $$PWD/../../../libexplor-qt/model
INCLUDEPATH += $$PWD/../../../libexplor-qt/vfs
INCLUDEPATH += $$PWD/../../../libexplor-qt/convenient-utils
INCLUDEPATH += $$PWD/../../../libexplor-qt/convenient-utils/disc
INCLUDEPATH += $$PWD/../../../libexplor-qt/effects
INCLUDEPATH += $$PWD/../../../plugin-iface/
INCLUDEPATH += $$PWD/../../../libexplor-qt/
INCLUDEPATH += $$PWD/../../../libexplor-qt/thumbnail


DISTFILES += $$PWD/common.json

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 poppler-qt5 gsettings-qt libcanberra libnotify udisks2 openssl dconf
LIBS +=-L$$PWD/../../../libexplor-qt/ -lexplor -lX11 -llingmo-log4qt

CONFIG += debug link_pkgconfig plugin no_keywords
TARGET = stable-properties-window
TEMPLATE = lib
DEFINES += STABLEPROPERTIESWINDOW_LIBRARY

CONFIG += c++11

#contains(DEFINES, LINGMO_FILE_DIALOG) {
#    PKGCONFIG += lingmosdk-qtwidgets
#}

#contains(DEFINES, LINGMO_SDK_SYSINFO) {
#    PKGCONFIG += lingmosdk-sysinfo
#}

#contains(DEFINES, LINGMO_SDK_QT_WIDGETS) {
#    PKGCONFIG += lingmosdk-qtwidgets
#}

#contains(DEFINES, LINGMO_SDK_WAYLANDHELPER) {
#    PKGCONFIG += lingmosdk-waylandhelper
#}

#contains(DEFINES, LINGMO_SDK_SYSINFO) {
#    PKGCONFIG += lingmosdk-sysinfo
#}

#schemes.files += org.lingmo.explor.settings.gschema.xml
#schemes.path = /usr/share/glib-2.0/schemas/

PROPERTIES_WINDOW_PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/explor-properties-window
DEFINES += PROPERTIES_WINDOW_PLUGIN_INSTALL_DIRS='\\"$${PROPERTIES_WINDOW_PLUGIN_INSTALL_DIRS}\\"'

#QMAKE_CXXFLAGS += -execution-charset:utf-8

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
    properties-window-factory.cpp \
    properties-window.cpp

HEADERS += \
    properties-window.h \
    properties-window_global.h \
    properties-window-factory.h \

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_LIBS]/explor-properties-window
}
!isEmpty(target.path): INSTALLS += target
