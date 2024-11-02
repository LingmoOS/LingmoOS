QT += core gui widgets concurrent

include(../common.pri)

TARGET = explor-send-to-device
TEMPLATE = lib

DEFINES += PEONYADMIN_LIBRARY

PKGCONFIG += explor

CONFIG += link_pkgconfig no_keywords c++11 plugin debug


SOURCES += \
    send-to-device-plugin.cpp
HEADERS += \
    send-to-device-plugin.h

TRANSLATIONS += \
    translations/explor-send-to-device_cs.ts \
    translations/explor-send-to-device_de.ts \
    translations/explor-send-to-device_es.ts \
    translations/explor-send-to-device_fr.ts \
    translations/explor-send-to-device_kk_KZ.ts \
    translations/explor-send-to-device_ug_CN.ts \
    translations/explor-send-to-device_ky_KG.ts \
    translations/explor-send-to-device_tr.ts \
    translations/explor-send-to-device_zh_CN.ts \
    translations/explor-send-to-device_bo_CN.ts \
    translations/explor-send-to-device_mn.ts \
    translations/explor-send-to-device_zh_HK.ts

#RESOURCES += \
#    resources.qrc

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/explor-extensions

INSTALLS += target
