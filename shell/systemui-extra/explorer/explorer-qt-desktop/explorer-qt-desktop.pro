#-------------------------------------------------
#
# Project created by QtCreator 2019-10-28T14:12:53
#
#-------------------------------------------------

QT       += core gui x11extras dbus concurrent KWindowSystem KWaylandClient KScreen

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../common.pri)

TARGET = explorer-qt-desktop
TEMPLATE = app
QMAKE_CXXFLAGS += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label
PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/explorer-extensions
DEFINES += PLUGIN_INSTALL_DIRS='\\"$${PLUGIN_INSTALL_DIRS}\\"'

DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../libexplorer-qt/libexplorer-qt-header.pri)
include(../3rd-parties/SingleApplication/singleapplication.pri)
include(../3rd-parties/qtsingleapplication/qtsingleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 gsettings-qt libcanberra wayland-client dconf
CONFIG += c++11 link_pkgconfig no_keywords lrelease

contains(DEFINES, LINGMO_SDK_KABASE) {
    PKGCONFIG += lingmosdk-kabase
}

contains(DEFINES, LINGMO_SDK_WAYLANDHELPER) {
    PKGCONFIG += lingmosdk-waylandhelper
}

LIBS += -L$$PWD/../libexplorer-qt/ -lexplorer -lX11 -llingmo-log4qt

TRANSLATIONS += ../translations/explorer-qt-desktop/explorer-qt-desktop_zh_CN.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_tr.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_cs.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_de.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_es.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_fr.ts \
#                ../translations/explorer-qt-desktop/explorer-qt-desktop_kk_KZ.ts \
#                ../translations/explorer-qt-desktop/explorer-qt-desktop_ug_CN.ts \
#                ../translations/explorer-qt-desktop/explorer-qt-desktop_ky_KG.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_bo_CN.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_mn.ts \
                ../translations/explorer-qt-desktop/explorer-qt-desktop_zh_HK.ts

SOURCES += \
    desktop-background-manager.cpp \
    desktopbackgroundwindow.cpp \
    main.cpp \
    explorer-desktop-application.cpp \
    fm-dbus-service.cpp \
    desktop-item-model.cpp \
    desktop-icon-view.cpp \
    desktop-icon-view-delegate.cpp \
    desktop-index-widget.cpp \
    desktop-menu.cpp \
    desktop-menu-plugin-manager.cpp \
    desktop-item-proxy-model.cpp \
    explorer-json-operation.cpp \
    bw-list-info.cpp \
    explorer-dbus-service.cpp \
    plasma-shell-manager.cpp \
    primary-manager.cpp \
    user-dir-manager.cpp \
    waylandoutputmanager.cpp \
    lingmo-output-core.c

HEADERS += \
    desktop-background-manager.h \
    desktopbackgroundwindow.h \
    explorer-desktop-application.h \
    fm-dbus-service.h \
    desktop-item-model.h \
    desktop-icon-view.h \
    desktop-icon-view-delegate.h \
    desktop-index-widget.h \
    desktop-menu.h \
    desktop-menu-plugin-manager.h \
    desktop-item-proxy-model.h \
    explorer-json-operation.h \
    bw-list-info.h \
    explorer-dbus-service.h \
    plasma-shell-manager.h \
    primary-manager.h \
    user-dir-manager.h \
    waylandoutputmanager.h \
    lingmo-output-client.h

target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

#QM_FILES_RESOURCE_PREFIX = ../translations/explorer-qt-desktop
QM_FILES_INSTALL_PATH = /usr/share/explorer-qt-desktop

desktop_data.path = /etc/xdg/autostart
desktop_data.files += ../data/explorer-desktop.desktop
INSTALLS += desktop_data

DISTFILES +=

RESOURCES += \
    explorer-qt-desktop-style.qrc
