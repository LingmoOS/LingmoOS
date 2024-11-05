QT += remoteobjects

INCLUDEPATH += \
    $$PWD

HEADERS += \
    $$PWD/remote-file-event-helper.h

SOURCES += \
    $$PWD/remote-file-event-helper.cpp

REPC_REPLICA += \
    $$PWD/fileeventhandler.rep
