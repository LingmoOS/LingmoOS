INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/disc

include(disc/disc.pri)

HEADERS += \
    $$PWD/audio-play-manager.h \
    $$PWD/clipboard-utils.h \
    $$PWD/datacdrom.h \
    $$PWD/file-operation-utils.h \
    $$PWD/systemd-bus-accounts.h \
    $$PWD/xdg-portal-helper.h

SOURCES += \
    $$PWD/audio-play-manager.cpp \
    $$PWD/clipboard-utils.cpp \
    $$PWD/datacdrom.cpp \
    $$PWD/file-operation-utils.cpp \
    $$PWD/systemd-bus-accounts.cpp \
    $$PWD/xdg-portal-helper.cpp
