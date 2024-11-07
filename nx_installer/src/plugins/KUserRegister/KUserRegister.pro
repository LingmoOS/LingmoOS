#-------------------------------------------------
#
# Project created by QtCreator 2020-07-15T14:36:03
#
#-------------------------------------------------

QT       += widgets
QT += dbus

TARGET = KUserRegister
TEMPLATE = lib
DESTDIR = $$PWD/../../KPlugins
DEFINES += KUSERREGISTER_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    checkedlineedit.cpp \
    clineeditlabel.cpp \
    kuserregister.cpp \
    userframe.cpp

HEADERS += \
    clineeditlabel.h \
    kuserregister.h \
    kuserregister_global.h \
    userframe.h \
    checkedlineedit.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!macx: LIBS += -L$$PWD/../../KPlugins/ -lReflex

INCLUDEPATH += $$PWD/../Reflex
DEPENDPATH += $$PWD/../Reflex

unix:!macx: LIBS += -L$$PWD/../../KPlugins/ -lPluginDll

INCLUDEPATH += $$PWD/../PluginDll
DEPENDPATH += $$PWD/../PluginDll

unix:!macx: LIBS += -L$$PWD/../../KPlugins/ -lPluginService

INCLUDEPATH += $$PWD/../PluginService
DEPENDPATH += $$PWD/../PluginService

RESOURCES += \
    resource.qrc

unix:!macx: LIBS += -lpwquality
unix:!macx: LIBS += -llingmo_chkname
