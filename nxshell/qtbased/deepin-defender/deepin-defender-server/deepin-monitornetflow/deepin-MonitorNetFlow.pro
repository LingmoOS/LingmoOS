QT += core gui dbus sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deepin-MonitorNetFlow
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dframeworkdbus gsettings-qt

INCLUDEPATH += \
    ../deepin-defender-datainterface/window/modules/common \
    ../../deepin-defender/src/window/modules/common

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

dbus.files += \
    com.deepin.defender.MonitorNetFlow.xml
dbus.header_flags += \
    -i ../deepin-defender-datainterface/window/modules/common/defenderprocinfolist.h

DBUS_ADAPTORS += dbus
DBUS_INTERFACES += dbus

SOURCES += \
    main.cpp \
    writedbusdata.cpp \
    disk/disk.cpp \
    getprocnetflow/dispatchnetpkgjob.cpp \
    getprocnetflow/getprocnetflow.cpp \
    getprocnetflow/netutils.cpp \
    usbconnection/usbmanagemonitor.cpp \
    localcache/securitypkgnamesql.cpp \
    localcache/netflowdatasql.cpp \
    localcache/settingssql.cpp \
    localcache/virusscanlogsql.cpp \
    localcache/usbconnlogsql.cpp \
    networkmanager/netmanagermodel.cpp \
    networkmanager/getprocinfojob.cpp \
    networkmanager/getprocinfointer.cpp \
    ../../deepin-defender/src/window/modules/common/common.cpp \
    localcache/securitylogsql.cpp \
    localcache/usbwhitelistsql.cpp

RESOURCES +=        

HEADERS += \
    writedbusdata.h \
    disk/disk.h \
    getprocnetflow/dispatchnetpkgjob.h \
    getprocnetflow/getprocnetflow.h \
    getprocnetflow/netutils.h \
    usbconnection/usbmanagemonitor.h \
    localcache/securitypkgnamesql.h \
    localcache/netflowdatasql.h \
    localcache/settingssql.h \
    localcache/virusscanlogsql.h \
    localcache/usbconnlogsql.h \
    networkmanager/netmanagermodel.h \
    networkmanager/getprocinfojob.h \
    networkmanager/getprocinfointer.h \
    ../../deepin-defender/src/window/modules/common/common.h \
    localcache/securitylogsql.h \
    localcache/usbwhitelistsql.h

DISTFILES += \
    com.deepin.defender.MonitorNetFlow.xml \
    com.deepin.defender.MonitorNetFlow.conf

message("current dir:" + $$PWD)
system_dbus_conf.files += \
    $$PWD/com.deepin.defender.MonitorNetFlow.conf
system_dbus_conf.path = /usr/share/dbus-1/system.d

install_service.files += \
    $$PWD/com.deepin.defender.MonitorNetFlow.service
install_service.path = /usr/share/dbus-1/system-services

target.path=/usr/bin
INSTALLS += target system_dbus_conf install_service

unix:!macx: LIBS += \
    # 流量截获库
    -L/lib/ -lpcap \
    # 管理usb设备库
    -L/lib/ -ludev \
    -L/lib/ -lrt \
    # 进程信息库
    -L/lib/ -lprocps \
    # 磁盘管理库
    -L/lib/ -lparted

INCLUDEPATH += /usr/include/pcap
DEPENDPATH += /usr/include/pcap
