QT       += widgets network dbus gui core
TEMPLATE = lib
CONFIG += plugin

TARGET = $$qtLibraryTarget(mobilehotspot)
DESTDIR = ../..

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

target.path = $$[QT_INSTALL_LIBS]/lingmo-control-center
trans.files = translations/*
trans.path = /usr/share/lingmo-nm/mobilehotspot/

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/ukcc/interface \
                 /usr/include/ukcc/widgets

LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lukcc

CONFIG += c++11 \
          link_pkgconfig \

PKGCONFIG += gsettings-qt \
             lingmosdk-qtwidgets \

#DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    blacklistpage.cpp \
    blacklistitem.cpp \
    connectdevpage.cpp \
    connectdevlistitem.cpp \
    mobilehotspot.cpp \
    mobilehotspotwidget.cpp

HEADERS += \
    blacklistpage.h \
    blacklistitem.h \
    connectdevpage.h \
    connectdevlistitem.h \
    mobilehotspot.h \
    mobilehotspotwidget.h \
    libukcc_global.h


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
        translations/bo_CN.ts \


