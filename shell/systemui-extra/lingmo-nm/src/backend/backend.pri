INCLUDEPATH += $$PWD
include(hotspot/hotspot.pri)
include(dbus-interface/dbus-interface.pri)

HEADERS += \
    $$PWD/dbus.h \
    $$PWD/dbus_adaptor.h \
    $$PWD/dbus_interface.h \
    $$PWD/sysdbusregister.h \
    $$PWD/utils.h \
    $$PWD/wifi-auth-thread.h

SOURCES += \
    $$PWD/dbus.cpp \
    $$PWD/dbus_adaptor.cpp \
    $$PWD/dbus_interface.cpp \
    $$PWD/sysdbusregister.cpp \
    $$PWD/utils.cpp \
    $$PWD/wifi-auth-thread.cpp
