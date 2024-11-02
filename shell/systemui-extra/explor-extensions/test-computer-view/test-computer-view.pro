QT       += core gui dbus network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += debug c++11 no_keywords link_pkgconfig plugin
PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 explor udisks2 libnotify

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

include(../common.pri)
include(../explor-extension-computer-view/computer-view/computer-view.pri)

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
