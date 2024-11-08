#-------------------------------------------------
#
# Project created by QtCreator 2020-06-17T13:22:56
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets xml

TARGET = lingmo-os-installer
TEMPLATE = app

CONFIG += c++11 \
          link_pkgconfig

# The following define makes your compiler emit warnings if you us
e
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    detailbutton.cpp \
    installermainwidget_auto.cpp \
    installingoemconfigframe.cpp \
    main.cpp \
    installermainwidget.cpp \
    ksetting.cpp \
    installingframe.cpp \
    myprobar.cpp \
    uilt/slidershow.cpp \
    uilt/showprogressbar.cpp \
    finishedInstall.cpp



HEADERS += \
    detailbutton.h \
    installermainwidget.h \
    installermainwidget_auto.h \
    installingoemconfigframe.h \
    ksetting.h \
    installingframe.h \
    myprobar.h \
    uilt/slidershow.h \
    uilt/showprogressbar.h \
    finishedInstall.h


RESOURCES += \
    resource.qrc


PKGCONFIG += x11 xrandr

unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lyaml-cpp

INCLUDEPATH += /usr/lib/x86_64-linux-gnu
DEPENDPATH += /usr/lib/x86_64-linux-gnu


unix:!macx: LIBS += -L$$PWD/../KPlugins/ -lReflex

INCLUDEPATH += $$PWD/../plugins/Reflex
DEPENDPATH += $$PWD/../plugins/Reflex

unix:!macx: LIBS += -L$$PWD/../KPlugins/ -lPluginDll

INCLUDEPATH += $$PWD/../plugins/PluginDll
DEPENDPATH += $$PWD/../plugins/PluginDll

unix:!macx: LIBS += -L$$PWD/../KPlugins/ -lPluginTask

INCLUDEPATH += $$PWD/../plugins/PluginTask
DEPENDPATH += $$PWD/../plugins/PluginTask

unix:!macx: LIBS += -L$$PWD/../KPlugins/ -lPluginService

INCLUDEPATH += $$PWD/../plugins/PluginService
DEPENDPATH += $$PWD/../plugins/PluginService



unix:!macx: LIBS += -L$$OUT_PWD/../plugins/VirtualKeyboard/ -lVirtualKeyboard

INCLUDEPATH += $$PWD/../plugins/VirtualKeyboard
DEPENDPATH += $$PWD/../plugins/VirtualKeyboard
