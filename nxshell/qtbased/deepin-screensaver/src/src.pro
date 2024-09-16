TARGET = deepin-screensaver
QT += core gui dbus quick x11extras dtkcore
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig
PKGCONFIG += x11 xscrnsaver xext xcb dframeworkdbus dtkwidget

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/screensaverwindow.cpp \
    $$PWD/dbusscreensaver.cpp \
    $$PWD/imageprovider.cpp \
    $$PWD/screensaverview.cpp \
    $$PWD/screensaversettingdialog.cpp \
    $$PWD/customconfig.cpp \
    $$PWD/utils.cpp \
    singlecustomsetting.cpp \
    commandlinehelper.cpp
HEADERS += \
    $$PWD/screensaverwindow.h \
    $$PWD/dbusscreensaver.h \
    $$PWD/imageprovider.h \
    $$PWD/screensaverview.h \
    $$PWD/screensaversettingdialog.h \
    $$PWD/customconfig.h \
    $$PWD/utils.h \
    singlecustomsetting.h \
    commandlinehelper.h

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
    !system($$PWD/update_translations.sh): error("Failed to generate translation")
}

translations.path = /usr/share/$${TARGET}/translations
translations.files = translations/*.qm

screensaver.files = $$PWD/com.deepin.ScreenSaver.xml
screensaver.header_flags += -l DBusScreenSaver -i $$PWD/dbusscreensaver.h
screensaver.source_flags += -l DBusScreenSaver

DBUS_ADAPTORS += screensaver

include(../common.pri)

screensaver_xmls.path = $$PREFIX/share/dbus-1/interfaces
screensaver_xmls.files = $$screensaver.files

screensaver_service.path = $$PREFIX/share/dbus-1/services
screensaver_service.files = $$PWD/com.deepin.ScreenSaver.service

INSTALLS += screensaver_service screensaver_xmls translations

DEFINES += LIB_PATH=\\\"$$LIB_PATH\\\" MODULE_PATH=\\\"$$MODULE_PATH\\\" RESOURCE_PATH=\\\"$$RESOURCE_PATH\\\"

target.path = $$PREFIX/bin
target.files = $$TARGET

INSTALLS += target

isEmpty(VERSION) {
    VERSION = 0.0.1
}
DEFINES += QMAKE_VERSION=\\\"$$VERSION\\\"

RESOURCES += \
    images.qrc

# DConfig
meta_file.files += \
    $$PWD/configs/org.deepin.screensaver.json
meta_file.base = $$PWD/configs
meta_file.appid = org.deepin.screensaver

DCONFIG_META_FILES += meta_file
load(dtk_install_dconfig)


