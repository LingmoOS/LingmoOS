TARGET = qdbusxml2cpp-fix

TEMPLATE = app
QT += core dbus-private

SOURCES += qdbusxml2cpp.cpp

DESTDIR = $$PWD/../../bin/

CONFIG += c++11

load(dtk_qmake)

host_sw_64 {
    QMAKE_CXXFLAGS += -mieee
}
