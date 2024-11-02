QT       += core gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = lingmo-login-sound

target.path = /usr/bin

CONFIG += \
    c++17 \
    no_keywords link_pkgconfig debug\
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../common/user_config.cpp \
    main.cpp \
    lingmo_login_sound.cpp \
    ../common/sound_effect_json.cpp \
    ../common/user_info_json.cpp \
    lingmo_login_sound_user_config.cpp

HEADERS += \
    formats.h \
    lingmo_login_sound.h \
    ../common/json.h \
    ../common/sound_effect_json.h \
    ../common/user_info_json.h \
    ../common/user_config.h \
    lingmo_login_sound_user_config.h

PKGCONFIG += alsa

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target
INSTALLS += target \
