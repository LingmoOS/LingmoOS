QT       += widgets network dbus gui core
TEMPLATE = lib
CONFIG += plugin

include(../component/drownlabel.pri)
include(../component/addbtn.pri)

TARGET = $$qtLibraryTarget(wlanconnect)
DESTDIR = ../..

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

target.path = $$[QT_INSTALL_LIBS]/lingmo-control-center
trans.files = translations/*
trans.path = /usr/share/lingmo-nm/wlanconnect/

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
    deviceframe.cpp \
#    drownlabel.cpp \
    itemframe.cpp \
    wlanconnect.cpp \
    wlanitem.cpp

HEADERS += \
    deviceframe.h \
#    drownlabel.h \
    itemframe.h \
    wlanconnect.h \
    wlanitem.h

FORMS += \
    wlanconnect.ui

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
