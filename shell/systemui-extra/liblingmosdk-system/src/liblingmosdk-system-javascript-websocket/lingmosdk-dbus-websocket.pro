#-------------------------------------------------
#
# Project created by QtCreator 2021-11-09T23:11:59
#
#-------------------------------------------------

QT       += core gui network  concurrent dbus webchannel websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lingmosdk-dbus-websocket
TEMPLATE = app

CONFIG += c++11 console

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

desktop.path = /etc/xdg/autostart/
desktop.files = lingmosdk-dbus-websocket.desktop

INSTALLS += desktop 

SOURCES += \
    lingmosdk/bios.cpp \
    lingmosdk/cpuinfo.cpp \
    lingmosdk/disk.cpp \
    lingmosdk/gps.cpp \
    lingmosdk/lingmosdkdbus.cpp \
    main.cpp \
    lingmosdk/mainboard.cpp \
    lingmosdk/net.cpp \
    lingmosdk/netcard.cpp \
    lingmosdk/netlink.cpp \
    lingmosdk/packageinfo.cpp \
    lingmosdk/peripheralsenum.cpp \
    lingmosdk/print.cpp \
    lingmosdk/process.cpp \
    lingmosdk/resolutionctl.cpp \
    lingmosdk/resource.cpp \
    lingmosdk/runinfo.cpp \
    lingmosdk/sysinfo.cpp \
    shared/websocketclientwrapper.cpp \
    shared/websockettransport.cpp 

HEADERS += \
    lingmosdk/bios.h \
    lingmosdk/cpuinfo.h \
    lingmosdk/dependence.h \
    lingmosdk/disk.h \
    lingmosdk/gps.h \
    lingmosdk/lingmosdkdbus.h \
    lingmosdk/mainboard.h \
    lingmosdk/net.h \
    lingmosdk/netcard.h \
    lingmosdk/netlink.h \
    lingmosdk/packageinfo.h \
    lingmosdk/peripheralsenum.h \
    lingmosdk/print.h \
    lingmosdk/process.h \
    lingmosdk/resolutionctl.h \
    lingmosdk/resource.h \
    lingmosdk/runinfo.h \
    lingmosdk/sysinfo.h \
    shared/websocketclientwrapper.h \
    shared/websockettransport.h 


