QT += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lingmo-defender-daemonservice
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dframeworkdbus gsettings-qt

SOURCES += \
        main.cpp \
        servicedbus.cpp \
        common/common.cpp \
    dbususermanager.cpp

HEADERS += \
        servicedbus.h \
        common/common.h \
        dbususermanager.h \

# 心跳服务dbus接口相关文件
daemondbus.files = com.lingmo.defender.daemonservice.xml
# 数据处理dbus接口相关文件
datainterfacedbus.files = ../lingmo-defender-datainterface/com.lingmo.defender.datainterface.xml
datainterfacedbus.header_flags += \
        -i ../lingmo-defender-datainterface/window/modules/common/defenderprocinfolist.h \
        -i /usr/include/libdframeworkdbus-2.0/types/defenderprocessinfo.h \
        -i /usr/include/libdframeworkdbus-2.0/types/defenderprocesslist.h

# 界面dbus接口相关文件
hmiscreendbus.files = ../../lingmo-defender/com.lingmo.defender.hmiscreen.xml
# 流量监控dbus接口相关文件
monitornetflowdbus.files = ../lingmo-monitornetflow/com.lingmo.defender.MonitorNetFlow.xml
monitornetflowdbus.header_flags += \
        -i ../lingmo-defender-datainterface/window/modules/common/defenderprocinfolist.h

DBUS_ADAPTORS += daemondbus
DBUS_INTERFACES += daemondbus datainterfacedbus hmiscreendbus monitornetflowdbus

target.path = /usr/bin/

install_autostart.files = ./lingmo-defender-daemonservice.desktop
install_autostart.path = /etc/xdg/autostart/

install_service.files = ./com.lingmo.defender.daemonservice.service
install_service.path = /usr/share/dbus-1/services

INSTALLS += target install_autostart install_service
