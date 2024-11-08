QT += core gui widgets concurrent dbus

TARGET = explorer-share
TEMPLATE = lib

DEFINES += PEONYSHARE_LIBRARY
DEFINES += DBUS_NAME=\\\"org.lingmo.samba.share.config\\\"
DEFINES += DBUS_PATH=\\\"/org/lingmo/samba/share\\\"

include(../../common.pri)

PKGCONFIG += explorer lingmosdk-qtwidgets polkit-gobject-1
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

TRANSLATIONS += translations/explorer-share-extension_cs.ts \
                translations/explorer-share-extension_de.ts \
                translations/explorer-share-extension_es.ts \
                translations/explorer-share-extension_fr.ts \
                translations/explorer-share-extension_kk_KZ.ts \
                translations/explorer-share-extension_ug_CN.ts \
                translations/explorer-share-extension_ky_KG.ts \
                translations/explorer-share-extension_tr.ts \
                translations/explorer-share-extension_zh_CN.ts \
                translations/explorer-share-extension_bo_CN.ts \
                translations/explorer-share-extension_mn.ts \
                translations/explorer-share-extension_zh_HK.ts

#RESOURCES += resources.qrc
CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/explorer-extensions

INSTALLS += target

DISTFILES += \
    translations/explorer-share-extension_bo_CN.ts
