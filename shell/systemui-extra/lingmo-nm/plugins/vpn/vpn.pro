#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T13:53:10
#
#-------------------------------------------------

QT       += widgets dbus

TEMPLATE = lib
CONFIG += plugin \
          c++11 \
          link_pkgconfig

include(../component/addbtn.pri)

PKGCONFIG   += gsettings-qt \
             lingmosdk-qtwidgets \

TARGET = $$qtLibraryTarget(vpn)
DESTDIR = ../..
target.path = $$[QT_INSTALL_LIBS]/lingmo-control-center
trans.files = translations/*
trans.path = /usr/share/lingmo-nm/vpn/

INCLUDEPATH   +=  \
                 $$PROJECT_COMPONENTSOURCE \
                 $$PROJECT_ROOTDIR \
                 /usr/include/ukcc/interface \
                 /usr/include/ukcc/widgets

LIBS += -L$$[QT_INSTALL_LIBS] -lukcc

SOURCES += \
    vpn.cpp \
    itemframe.cpp \
    vpnitem.cpp

HEADERS += \
    vpn.h \
    itemframe.h \
    vpnitem.h

FORMS += \
    vpn.ui

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

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}
