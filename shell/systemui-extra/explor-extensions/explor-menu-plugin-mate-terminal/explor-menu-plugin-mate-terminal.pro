#-------------------------------------------------
#
# Project created by QtCreator 2019-11-05T10:40:52
#
#-------------------------------------------------

QT       += widgets concurrent dbus

TARGET = explor-menu-plugin-mate-terminal
TEMPLATE = lib

DEFINES += PEONYMENUPLUGINMATETERMINAL_LIBRARY

include(../common.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 explor
CONFIG += debug c++11 link_pkgconfig no_keywords plugin

#DESTDIR += ../testdir

SOURCES += \
        mate-terminal-menu-plugin.cpp

HEADERS += \
        mate-terminal-menu-plugin.h \
        explor-menu-plugin-mate-terminal_global.h

TRANSLATIONS += translations/explor-mate-terminal-extension_zh_CN.ts \
                translations/explor-mate-terminal-extension_de.ts \
                translations/explor-mate-terminal-extension_es.ts \
                translations/explor-mate-terminal-extension_fr.ts \
                translations/explor-mate-terminal-extension_kk_KZ.ts \
                translations/explor-mate-terminal-extension_ug_CN.ts \
                translations/explor-mate-terminal-extension_ky_KG.ts \
                translations/explor-mate-terminal-extension_tr.ts \
                translations/explor-mate-terminal-extension_cs.ts \
                translations/explor-mate-terminal-extension_bo_CN.ts \
                translations/explor-mate-terminal-extension_mn.ts \
                translations/explor-mate-terminal-extension_zh_HK.ts

unix {
    target.path = $$[QT_INSTALL_LIBS]/explor-extensions
    INSTALLS += target
}

#RESOURCES += \
#    explor-menu-plugin-mate-terminal.qrc

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/
