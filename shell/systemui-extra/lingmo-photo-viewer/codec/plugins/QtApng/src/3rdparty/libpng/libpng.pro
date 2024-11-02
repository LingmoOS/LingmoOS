TEMPLATE = lib

TARGET = qtapng_png

#QT =

CONFIG -= qt
CONFIG += static hide_symbols exceptions_off rtti_off warn_off

DEFINES += PNG_ARM_NEON_OPT=0

HEADERS += $$files(src/*.h)
SOURCES += $$files(src/*.c)
SOURCES -= src/example.c

MODULE_INCLUDEPATH = $$PWD/src
load(qt_helper_lib)

QMAKE_USE_PRIVATE += apng_zlib
