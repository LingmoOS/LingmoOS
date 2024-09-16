#include(../common.pri)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
QT *= network
greaterThan(QT_MAJOR_VERSION, 4): QT *= widgets

    SOURCES += $$PWD/*.cpp
    HEADERS += $$PWD/*.h
