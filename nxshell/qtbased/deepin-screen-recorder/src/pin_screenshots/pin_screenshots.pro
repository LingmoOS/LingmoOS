#-------------------------------------------------
#
# Project created by QtCreator 2021-12-16T10:00:05
#
#-------------------------------------------------

QT       += dtkgui dtkwidget dbus
TARGET = deepin-pin-screenshots
TEMPLATE = app
INCLUDEPATH += .

#下面的定义会让你的编译器在你使用任何被标记为deprecated的Qt特性时发出警告(确切的警告取决于你的编译器)。
#请查阅已弃用API的文档，以了解如何将代码从它那里移植出去。
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    service/dbuspinscreenshotsadaptor.cpp \
    service/pinscreenshotsinterface.cpp \
    mainwindow.cpp \
    main.cpp \
    pinscreenshots.cpp \
    service/ocrinterface.cpp \
    ui/menucontroller.cpp \
    ui/toolbarwidget.cpp \
#    ui/toolbar.cpp \
    ui/subtoolwidget.cpp \
    utils.cpp \
    ui/mainToolWidget.cpp \
    settings.cpp

HEADERS += \
    service/dbuspinscreenshotsadaptor.h \
    service/pinscreenshotsinterface.h \
    mainwindow.h \
    pinscreenshots.h \
    service/ocrinterface.h \
    ui/menucontroller.h \
    ui/toolbarwidget.h \
#    ui/toolbar.h \
    ui/subtoolwidget.h \
    ui/toolbutton.h \
    utils.h \
    ui/mainToolWidget.h \
    settings.h

QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -Wno-error=deprecated-declarations -Wno-deprecated-declarations
##安装路径
isEmpty(PREFIX){
    PREFIX = /usr
}

target.path=$$PREFIX/bin
isEmpty(DSRDIR):DSRDIR=$$PREFIX/share/deepin-pin-screenshots

#Dbus文件
dbus_service.path=$$PREFIX/share/dbus-1/services
dbus_service.files=./com.deepin.PinScreenShots.service
include(accessibility/accessible.pri)
#安装
INSTALLS += target dbus_service

#RESOURCES += \
#    ../../assets/icons/pin_icons.qrc\
#    ../../assets/image/pin_screenshots.qrc

RESOURCES += \
    ./icons/icons.qrc

