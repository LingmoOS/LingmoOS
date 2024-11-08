QT += core gui widgets concurrent

include(../common.pri)

TARGET = explorer-drive-rename
TEMPLATE = lib

DEFINES += PEONYADMIN_LIBRARY

PKGCONFIG += explorer gio-2.0 udisks2 glib-2.0

CONFIG += link_pkgconfig no_keywords c++11 plugin debug


SOURCES += \
    drive-rename.cpp \
    driverenameplugin.cpp


HEADERS += \
    drive-rename.h \
    driverenameplugin.h


TRANSLATIONS += translations/explorer-drive-rename_zh_CN.ts \
                translations/explorer-drive-rename_de.ts \
                translations/explorer-drive-rename_es.ts \
                translations/explorer-drive-rename_fr.ts \
                translations/explorer-drive-rename_kk_KZ.ts \
                translations/explorer-drive-rename_ug_CN.ts \
                translations/explorer-drive-rename_ky_KG.ts \
                translations/explorer-drive-rename_tr.ts \
                translations/explorer-drive-rename_cs.ts \
                translations/explorer-drive-rename_bo_CN.ts \
                translations/explorer-drive-rename_mn.ts \
                translations/explorer-drive-rename_zh_HK.ts

#RESOURCES += \
#    resource.qrc

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/explorer-extensions

INSTALLS += target
