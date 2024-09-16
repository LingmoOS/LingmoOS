
QT      += dbus

HEADERS += \
    $$PWD/DBusExtended \
    $$PWD/dbusextended.h \
    $$PWD/DBusExtendedAbstractInterface \
    $$PWD/dbusextendedabstractinterface.h \
    $$PWD/dbusextendedpendingcallwatcher_p.h

SOURCES += \
    $$PWD/dbusextendedabstractinterface.cpp \
    $$PWD/dbusextendedpendingcallwatcher.cpp

INCLUDEPATH += $$PWD
