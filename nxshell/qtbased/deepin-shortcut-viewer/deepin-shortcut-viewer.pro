#-------------------------------------------------
#
# Project created by QtCreator 2016-07-27T09:14:07
#
#-------------------------------------------------

QT       += core gui core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkcore

DEFINES += QT_MESSAGELOGCONTEXT

isEmpty(TARGET) {
    TARGET = deepin-shortcut-viewer
}


TEMPLATE = app


SOURCES += main.cpp\
    view/mainwidget.cpp \
    commandlinemanager.cpp \
    singleapplication.cpp \
    view/shortcutitem.cpp \
    view/shortcutview.cpp

HEADERS  += \
    view/mainwidget.h \
    commandlinemanager.h \
    singleapplication.h \
    view/shortcutitem.h \
    view/shortcutview.h


SUBDIRS += \
    QLogger/test/test.pro

DISTFILES += \
    QLogger/LICENSE \
    QLogger/README.md


isEmpty(PREFIX){
    PREFIX = /usr
}
BINDIR = $$PREFIX/bin

target.path = $$BINDIR

INSTALLS += target

