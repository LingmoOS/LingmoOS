TEMPLATE = subdirs

CONFIG += ordered \
    qt

SUBDIRS = \
    UIControl \
    registeredQDbus \

HEADERS += \
    UIControl/src/line.h

SOURCES += \
    UIControl/src/line.cpp

QT += widgets

