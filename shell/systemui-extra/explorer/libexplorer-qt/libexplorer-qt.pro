#-------------------------------------------------
#
# Project created by QtCreator 2019-07-20T10:23:11
#
#-------------------------------------------------

QT       += network core widgets gui concurrent xml KWindowSystem dbus x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport

include(../common.pri)

TARGET = explorer
TEMPLATE = lib
CONFIG += create_pc create_prl no_install_prl
QMAKE_CXXFLAGS += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label
CONFIG += link_pkgconfig no_keywords c++11 lrelease hide_symbols
PKGCONFIG += glib-2.0 gio-2.0 gio-unix-2.0 poppler-qt5 gsettings-qt udisks2 libnotify libcanberra openssl x11-xcb dconf
DEFINES += PEONYCORE_LIBRARY

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

schemes.files += org.lingmo.explorer.settings.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/

PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/explorer-extensions
DEFINES += PLUGIN_INSTALL_DIRS='\\"$${PLUGIN_INSTALL_DIRS}\\"'

PROPERTIES_WINDOW_PLUGIN_INSTALL_DIRS = $$[QT_INSTALL_LIBS]/explorer-properties-window
DEFINES += PROPERTIES_WINDOW_PLUGIN_INSTALL_DIRS='\\"$${PROPERTIES_WINDOW_PLUGIN_INSTALL_DIRS}\\"'

QMAKE_CXXFLAGS += -execution-charset:utf-8

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(libexplorer-qt.pri)

TRANSLATIONS += ../translations/libexplorer-qt/libexplorer-qt_zh_CN.ts \
                ../translations/libexplorer-qt/libexplorer-qt_tr.ts \
                ../translations/libexplorer-qt/libexplorer-qt_cs.ts \
                ../translations/libexplorer-qt/libexplorer-qt_de.ts \
                ../translations/libexplorer-qt/libexplorer-qt_es.ts \
                ../translations/libexplorer-qt/libexplorer-qt_fr.ts \
#                ../translations/libexplorer-qt/libexplorer-qt_kk_KZ.ts \
#                ../translations/libexplorer-qt/libexplorer-qt_ug_CN.ts \
#                ../translations/libexplorer-qt/libexplorer-qt_ky_KG.ts \
                ../translations/libexplorer-qt/libexplorer-qt_bo_CN.ts \
                ../translations/libexplorer-qt/libexplorer-qt_mn.ts \
                ../translations/libexplorer-qt/libexplorer-qt_zh_HK.ts

DESTDIR += $$PWD

unix {
    target.path = $$[QT_INSTALL_LIBS]

    QMAKE_PKGCONFIG_NAME = explorer
    QMAKE_PKGCONFIG_DESCRIPTION = Peony Header files
    QMAKE_PKGCONFIG_VERSION = $$VERSION
    QMAKE_PKGCONFIG_LIBDIR = $$target.path
    QMAKE_PKGCONFIG_DESTDIR = pkgconfig
    QMAKE_PKGCONFIG_INCDIR = /usr/include/explorer-qt
    QMAKE_PKGCONFIG_CFLAGS += -I/usr/include/explorer-qt/fileop -I/usr/include/explorer-qt/model -I/usr/include/explorer-qt/controls
    QMAKE_PKGCONFIG_REQUIRES = Qt5Widgets Qt5Network glib-2.0 gio-2.0 gio-unix-2.0 poppler-qt5 gsettings-qt udisks2 libnotify libcanberra

    INSTALLS += target \
                schemes

    # fixme:// format_dialog.h
    header.path = /usr/include/explorer-qt
    header.files += *.h model/*.h file-operation/*.h vfs/*.h controls/ ../plugin-iface/*.h convenient-utils/*.h convenient-utils/disc/*.h windows/format_dialog.h windows/FMWindowIface.h \
                    libexplorer-qt/usershare-manager.h windows/udfFormatDialog.h windows/udfAppendBurnDataDialog.h windows/format-dlg-create-delegate.h file-launcher/*.h private/*.h
#    header.depends = header2
    header.files += development-files/header-files/*
    INSTALLS += header

#    header2.commands = sh $$PWD/copy-headers.sh
#    INSTALLS += header2

#    QMAKE_EXTRA_TARGETS += header header2

#    pcfile.path = $$[QT_INSTALL_LIBS]/pkgconfig
#    pcfile.files = development-files/explorer.pc
#    INSTALLS += pcfile

    #QM_FILES_RESOURCE_PREFIX = ../translations/libexplorer-qt
    QM_FILES_INSTALL_PATH = /usr/share/libexplorer-qt
}
