#-------------------------------------------------
#
# Project created by QtCreator 2021-05-14T10:44:28
#
#-------------------------------------------------

QT       += core gui dtkcore network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deepin-custom-screensaver
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 link_pkgconfig

PKGCONFIG += x11 dtkwidget

SOURCES += \
        src/main.cpp \
    src/slideshowscreensaver.cpp \
    src/commandlinemanager.cpp \
    src/slideshowconfig.cpp \
    src/slideshowconfigdialog.cpp \
    src/config/contenttitle.cpp \
    src/config/selectpathwidget.cpp \
    src/config/truncatelineedit.cpp \
    src/config/timeintervalwidget.cpp \
    src/singleinstance.cpp

HEADERS += \ 
    src/slideshowscreensaver.h \
    src/commandlinemanager.h \
    src/slideshowconfig.h \
    src/slideshowconfigdialog.h \
    src/config/contenttitle.h \
    src/config/selectpathwidget.h \
    src/config/truncatelineedit.h \
    src/config/timeintervalwidget.h \
    src/singleinstance.h


#!! 放开注释来更新json配置文件所生成的ts文件
#DTK_SETTINGS = $${QT.dtkcore.tools}/dtk-settings
#system($$DTK_SETTINGS -o deepin-custom-screensaver_translation.cpp $$PWD/data/deepin-custom-screensaver.json)
#SOURCES += deepin-custom-screensaver_translation.cpp

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

CONFIG(release, debug|release) {
    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
}

target.path = /usr/lib/deepin-screensaver/modules/

translations.path = /usr/share/$${TARGET}/translations
translations.files = translations/*.qm

desktop.path = /etc/deepin-screensaver/$${TARGET}/
desktop.files = data/$${TARGET}.desktop

icons.path = /usr/lib/deepin-screensaver/modules/cover/
icons.files = src/icons/*.jpg

INSTALLS += target translations desktop icons

# DConfig
meta_file.files += \
    $$PWD/data/org.deepin.customscreensaver.json
meta_file.base = $$PWD/data
meta_file.appid = org.deepin.screensaver

DCONFIG_META_FILES += meta_file
load(dtk_install_dconfig)

RESOURCES += \
    icon.qrc
