QT += core gui widgets concurrent dbus

TARGET = explor-share
TEMPLATE = lib

DEFINES += PEONYSHARE_LIBRARY
DEFINES += DBUS_NAME=\\\"org.lingmo.samba.share.config\\\"
DEFINES += DBUS_PATH=\\\"/org/lingmo/samba/share\\\"

include(../../common.pri)

PKGCONFIG += explor lingmosdk-qtwidgets polkit-gobject-1
CONFIG += link_pkgconfig \
          c++11 \
	  plugin \
	  debug

SOURCES += share-page.cpp \
           advanced-share-page.cpp \
           share-emblem-provider.cpp \
           samba-config-thread.cpp \
           share-properties-page-plugin.cpp \
           samba-config-interface.cpp \
           systemd-bus-accounts.cpp


HEADERS += share-page.h \
           advanced-share-page.h \
           share-emblem-provider.h \
           samba-config-thread.h \
           share-properties-page-plugin.h \
           samba-config-interface.h \
           systemd-bus-accounts.h

TRANSLATIONS += translations/explor-share-extension_cs.ts \
                translations/explor-share-extension_de.ts \
                translations/explor-share-extension_es.ts \
                translations/explor-share-extension_fr.ts \
                translations/explor-share-extension_kk_KZ.ts \
                translations/explor-share-extension_ug_CN.ts \
                translations/explor-share-extension_ky_KG.ts \
                translations/explor-share-extension_tr.ts \
                translations/explor-share-extension_zh_CN.ts \
                translations/explor-share-extension_bo_CN.ts \
                translations/explor-share-extension_mn.ts \
                translations/explor-share-extension_zh_HK.ts

#RESOURCES += resources.qrc
CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/explor-extensions

INSTALLS += target

DISTFILES += \
    translations/explor-share-extension_bo_CN.ts
