#include(../../../env.pri)

QT       += widgets dbus

TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(proxy)
DESTDIR = ../..

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

target.path = $$[QT_INSTALL_LIBS]/lingmo-control-center
trans.files = translations/*
trans.path = /usr/share/lingmo-nm/proxy/

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/ukcc/interface \
                 /usr/include/ukcc/widgets

LIBS          += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lukcc

##加载gio库和gio-unix库，用于获取和设置enum类型的gsettings
CONFIG        += link_pkgconfig \
                 C++11
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \
                 gsettings-qt \
                 lingmosdk-qtwidgets

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    applistwidget.cpp \
    aptproxydialog.cpp \
    proxy.cpp

HEADERS += \
    applistwidget.h \
    aptinfo.h \
    aptproxydialog.h \
    proxy.h \
    certificationdialog.h

FORMS +=

INSTALLS += target \
            trans

TRANSLATIONS += \
        translations/zh_Hant.ts \
        translations/zh_CN.ts \
        translations/ug.ts \
        translations/tr.ts \
        translations/mn.ts \
        translations/ky.ts \
        translations/kk.ts \
        translations/fr.ts \
        translations/es.ts \
        translations/en_US.ts \
        translations/de.ts \
        translations/bo.ts\
        translations/bo_CN.ts
