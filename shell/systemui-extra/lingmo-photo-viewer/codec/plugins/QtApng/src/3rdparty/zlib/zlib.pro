TEMPLATE = lib

TARGET = qtapng_zlib

QT =

CONFIG -= qt
CONFIG += static exceptions_off rtti_off warn_off

HEADERS += $$files(src/*.h)
SOURCES += $$files(src/*.c)

MODULE_INCLUDEPATH = $$PWD/src
load(qt_helper_lib)
