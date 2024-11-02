QT += core gui widgets concurrent

include(../common.pri)

TARGET = explor-drive-rename
TEMPLATE = lib

DEFINES += PEONYADMIN_LIBRARY

PKGCONFIG += explor gio-2.0 udisks2 glib-2.0

CONFIG += link_pkgconfig no_keywords c++11 plugin debug


SOURCES += \
    drive-rename.cpp \
    driverenameplugin.cpp


HEADERS += \
    drive-rename.h \
    driverenameplugin.h


TRANSLATIONS += translations/explor-drive-rename_zh_CN.ts \
                translations/explor-drive-rename_de.ts \
                translations/explor-drive-rename_es.ts \
                translations/explor-drive-rename_fr.ts \
                translations/explor-drive-rename_kk_KZ.ts \
                translations/explor-drive-rename_ug_CN.ts \
                translations/explor-drive-rename_ky_KG.ts \
                translations/explor-drive-rename_tr.ts \
                translations/explor-drive-rename_cs.ts \
                translations/explor-drive-rename_bo_CN.ts \
                translations/explor-drive-rename_mn.ts \
                translations/explor-drive-rename_zh_HK.ts

#RESOURCES += \
#    resource.qrc

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/explor-extensions

INSTALLS += target
