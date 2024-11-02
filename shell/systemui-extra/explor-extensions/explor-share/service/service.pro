TEMPLATE = app
TARGET = explor-samba-service

QT += core dbus

PKGCONFIG += gio-2.0 polkit-gobject-1
CONFIG += link_pkgconfig c++11 no_keywords

DEFINES += DBUS_NAME=\\\"org.lingmo.samba.share.config\\\"
DEFINES += DBUS_PATH=\\\"/org/lingmo/samba/share\\\"

SOURCES += $$PWD/explor-samba-service.cpp  $$PWD/samba-config.cpp
HEADERS += $$PWD/samba-config.h
target.path = /usr/libexec/

INSTALLS += target
