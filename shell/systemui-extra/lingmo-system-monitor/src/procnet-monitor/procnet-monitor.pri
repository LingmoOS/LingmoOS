INCLUDEPATH	+= $$PWD
DEPENDPATH      += $$PWD
DEFINES += 

SOURCES += \
    $$PWD/process.cpp \
    $$PWD/packet.cpp \
    $$PWD/nethogs.cpp \
    $$PWD/libnethogs.cpp \
    $$PWD/inode2prog.cpp \
    $$PWD/devices.cpp \
    $$PWD/conninode.cpp \
    $$PWD/connection.cpp \
    $$PWD/decpcap.c

HEADERS += \
    $$PWD/process.h \
    $$PWD/packet.h \
    $$PWD/nethogs.h \
    $$PWD/libnethogs.h \
    $$PWD/inode2prog.h \
    $$PWD/devices.h \
    $$PWD/conninode.h \
    $$PWD/connection.h \
    $$PWD/decpcap.h

