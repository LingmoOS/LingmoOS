TEMPLATE = app

QT += testlib

CONFIG += console
CONFIG -= app_bundle

TARGET = tst_apngplugin

DEFINES += SRCDIR=\\\"$$PWD/\\\"

SOURCES += \
	tst_apngplugin.cpp

RESOURCES += \
	tst_apngplugin.qrc

include(../../testrun.pri)
