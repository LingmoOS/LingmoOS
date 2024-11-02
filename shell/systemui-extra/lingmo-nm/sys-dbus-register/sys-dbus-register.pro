QT += dbus network
TARGET = lingmo-nm-sysdbus
TEMPLATE = app
CONFIG += c++14 qt warn_on link_pkgconfig no_keywords
CONFIG -= app_bundle

TARGET = lingmo-nm-sysdbus
TEMPLATE = app

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 libcap

inst1.files += conf/com.lingmo.network.qt.systemdbus.service
inst1.path = /usr/share/dbus-1/system-services/
inst2.files += conf/com.lingmo.network.qt.systemdbus.conf
inst2.path = /etc/dbus-1/system.d/
target.source += $$TARGET
target.path = /usr/bin

INSTALLS += \
         target \
         inst1 \
         inst2 \

include(kyarping/kyarping.pri)

SOURCES += \
        kynmsystemdbus.cpp \
        main.cpp

HEADERS += \
        kynmsystemdbus.h
