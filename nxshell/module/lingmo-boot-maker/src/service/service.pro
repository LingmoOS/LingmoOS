include($$PWD/vendor.pri)
QT += core dbus
QT -= gui

CONFIG += c++11

TARGET = lingmo-boot-maker-service
CONFIG += console link_pkgconfig
CONFIG -= app_bundle

#添加安全编译参数
QMAKE_LFLAGS += -z noexecstack -pie -fPIC -z relro -z now
QMAKE_CFLAGS += -fstack-protector-all
QMAKE_CXXFLAGS += -fstack-protector-all


TEMPLATE = app

RESOURCES +=  \
    ../translate.qrc \

SOURCES += main.cpp \
    bootmakerservice.cpp \
    LogManager.cpp

HEADERS += \
    bootmakerservice.h \
    LogManager.h \
    bootmakerservice_p.h

unix{
binary.path = $${PREFIX}/lib/lingmo-daemon/
binary.files = $${OUT_PWD}/lingmo-boot-maker-service

service.path = $${PREFIX}/share/dbus-1/system-services/
service.files = $$PWD/data/com.lingmo.bootmaker.service

dbus.path = $${PREFIX}/share/dbus-1/system.d/
dbus.files = $$PWD/data/com.lingmo.bootmaker.conf

systemd.path = $${PREFIX}/lib/systemd/system
systemd.files = $$PWD/data/lingmo-boot-maker.service

policy.path = $${PREFIX}/share/polkit-1/actions
policy.files = $$PWD/data/com.lingmo.bootmaker.policy

INSTALLS += service dbus binary systemd policy
}
