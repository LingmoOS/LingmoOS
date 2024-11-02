#-------------------------------------------------
#
# Project created by QtCreator 2019-05-30T09:45:54
#
#-------------------------------------------------

#include(../../../env.pri)

QT       += widgets xml dbus

TEMPLATE = lib
CONFIG += plugin

#include($$PROJECT_COMPONENTSOURCE/label.pri)
include(app-device-control/app-device-control.pri)
include(audio-device-control/audio-device-control.pri)

INCLUDEPATH += ../../.. \
               $$PROJECT_COMPONENTSOURCE \

TARGET = $$qtLibraryTarget(audio)
DESTDIR = ../
target.path = $$[QT_INSTALL_LIBS]/lingmo-control-center

CONFIG += c++11 \
          no_keywords link_pkgconfig

PKGCONFIG += gio-2.0 \
             libxml-2.0 \
             Qt5Multimedia \
             gsettings-qt \
             libcanberra \
             dconf  \
             libpulse \
             libpulse-mainloop-glib \
             lingmosdk-qtwidgets \
             lingmosdk-sysinfo \
             lingmosdk-hardware
#DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lukcc

SOURCES += \
    audio.cpp \
    ukmedia_input_widget.cpp \
    ukmedia_main_widget.cpp \
    ukmedia_output_widget.cpp \
    ukmedia_settings_widget.cpp \
    ukmedia_sound_effects_widget.cpp \
    ukmedia_volume_control.cpp \
    lingmo_custom_style.cpp \
    customstyle.cpp \
    ukmedia_slider_tip_label_helper.cpp \
    lingmo_list_widget_item.cpp \
    ../common/ukmedia_common.cpp

HEADERS += \
    audio.h \
    ukmedia_input_widget.h \
    ukmedia_main_widget.h \
    ukmedia_output_widget.h \
    ukmedia_settings_widget.h \
    ukmedia_sound_effects_widget.h \
    ukmedia_volume_control.h \
    lingmo_custom_style.h \
    customstyle.h \
    ukmedia_slider_tip_label_helper.h \
    lingmo_list_widget_item.h  \
    ../common/ukmedia_common.h

FORMS += \
    audio.ui

TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/tr.ts \
    translations/zh_HK.ts \
    translations/bo_CN.ts \
    translations/en_US.ts \
    translations/mn_MN.ts

isEmpty(PREFIX) {
    PREFIX = /usr
}

qm_files.path = $${PREFIX}/share/lingmo-media/translations/audio/
qm_files.files = translations/*

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}


INSTALLS += target \
            qm_files \
