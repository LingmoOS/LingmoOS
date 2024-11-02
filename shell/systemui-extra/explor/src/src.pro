#-------------------------------------------------
#
# Project created by QtCreator 2019-06-20T09:45:20
#
#-------------------------------------------------

QT       += core gui x11extras dbus KWindowSystem concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../common.pri)

TARGET = explor

TEMPLATE = app
QMAKE_CXXFLAGS += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label
include(../libexplor-qt/libexplor-qt-header.pri)
include(../3rd-parties/SingleApplication/singleapplication.pri)
include(windows/windows.pri)
include(windows/windows-explor.pri)
include(control/control.pri)
include(../plugin-iface/unstable/window-plugin-iface.pri)
#include(view/view.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 gsettings-qt libcanberra libnotify udisks2 openssl dconf polkit-gobject-1
LIBS +=-lgio-2.0 -lglib-2.0 -lX11 -llingmo-log4qt
CONFIG += c++11 link_pkgconfig no_keywords lrelease

contains(DEFINES, LINGMO_SDK_KABASE) {
    PKGCONFIG += lingmosdk-kabase
}

contains(DEFINES, LINGMO_SDK_DATACOLLECT) {
    PKGCONFIG += lingmosdk-datacollect
}

LIBS += -L$$PWD/../libexplor-qt/ -lexplor

contains(DEFINES, LINGMO_SDK_QT_WIDGETS) {
    PKGCONFIG += lingmosdk-qtwidgets
}

contains(DEFINES, LINGMO_SDK_WAYLANDHELPER) {
    PKGCONFIG += lingmosdk-waylandhelper
}

WINDOW_PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/explor-main-window
DEFINES += WINDOW_PLUGIN_INSTALL_DIRS='\\"$${WINDOW_PLUGIN_INSTALL_DIRS}\\"'

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TRANSLATIONS += ../translations/explor-qt/explor-qt_zh_CN.ts \
                ../translations/explor-qt/explor-qt_tr.ts \
                ../translations/explor-qt/explor-qt_cs.ts \
                ../translations/explor-qt/explor-qt_de.ts \
                ../translations/explor-qt/explor-qt_es.ts \
                ../translations/explor-qt/explor-qt_fr.ts \
#                ../translations/explor-qt/explor-qt_kk_KZ.ts \
#                ../translations/explor-qt/explor-qt_ug_CN.ts \
#                ../translations/explor-qt/explor-qt_ky_KG.ts \
                ../translations/explor-qt/explor-qt_bo_CN.ts \
                ../translations/explor-qt/explor-qt_mn.ts \
                ../translations/explor-qt/explor-qt_zh_HK.ts

SOURCES += \
    main-window-factory-plugin-manager.cpp \
    explor-application.cpp \
    explor-main-window-style.cpp \
    explor-main.cpp \

HEADERS += \
    explor-application.h \
    explor-main-window-style.h
    main-window-factory-plugin-manager.h \

INCLUDEPATH    += ../plugin-iface

target.path = /usr/bin
INSTALLS += target

#QM_FILES_RESOURCE_PREFIX = ../translations/explor-qt
QM_FILES_INSTALL_PATH = /usr/share/explor-qt

data.path = /usr/share/applications
data.files += ../data/explor.desktop \
              ../data/explor-computer.desktop \
              ../data/explor-home.desktop \
              ../data/explor-trash.desktop
INSTALLS += data

RESOURCES += \
    custome.qrc
