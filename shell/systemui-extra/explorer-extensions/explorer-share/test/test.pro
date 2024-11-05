TARGET = test-auth
TEMPLATE = app

QT += core dbus widgets

PKGCONFIG += gio-2.0 polkit-gobject-1
CONFIG += link_pkgconfig c++11 no_keywords

DEFINES += DBUS_NAME=\\\"org.lingmo.samba.share.config\\\"
DEFINES += DBUS_PATH=\\\"/org/lingmo/samba/share\\\"

SOURCES += $$PWD/../app/samba-config-interface.cpp  $$PWD/test-auth.cpp
HEADERS += $$PWD/../app/samba-config-interface.h
