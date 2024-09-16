#-------------------------------------------------
#
# Project created by QtCreator 2019-03-18T11:05:46
#
#-------------------------------------------------

QT       -= gui

TARGET = disomaster
TEMPLATE = lib
CONFIG += no_keywords link_pkgconfig create_prl no_install_prl create_pc

DEFINES += DISOMASTER_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PKGCONFIG += libisoburn-1

SOURCES += \
        disomaster.cpp

HEADERS += \
        disomaster.h

isEmpty(PREFIX) {
    PREFIX = /usr
}

isEmpty(LIBDIR) {
    LIBDIR = $$PREFIX/lib
}

includes.files += disomaster.h
includes.path = $$PREFIX/include/disomaster

QMAKE_PKGCONFIG_NAME = libdisomaster
QMAKE_PKGCONFIG_DESCRIPTION = Wrapper class for libisoburn
QMAKE_PKGCONFIG_INCDIR = $$includes.path
QMAKE_PKGCONFIG_LIBDIR = $$LIBDIR
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

unix {
    target.path = $$PREFIX/lib
    INSTALLS += target includes
}
