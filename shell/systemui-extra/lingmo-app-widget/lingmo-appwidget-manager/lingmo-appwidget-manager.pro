QT       += core gui dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_WIDGET_LIST=\\\"/usr/share/appwidget/qml\\\"
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
include(QtSingleApplication/qtsingleapplication.pri)
include(../liblingmo-appwidget-common/liblingmo-appwidget-common.pri)
SOURCES += \
    appwidgetconfiguration.cpp \
    appwidgetproviderproxy.cpp \
    appwidgetproxy.cpp \
    asyncproviderproxycreator.cpp \
    main.cpp \
    appwidgetmanager.cpp \
    mappingmanager.cpp

HEADERS += \
    appwidgetcommon.h \
    appwidgetconfiguration.h \
    appwidgetmanager.h \
    appwidgetproviderproxy.h \
    appwidgetproxy.h \
    asyncproviderproxycreator.h \
    mappingmanager.h

# Default rules for deployment.
target.path = /usr/bin/
managerservice.files += data/org.lingmo.appwidget.manager.service
managerservice.path += /usr/share/dbus-1/services/
desktop.files += data/lingmo-appwidget-manager.desktop
desktop.path += /etc/xdg/autostart/
INSTALLS += target managerservice desktop

DISTFILES += \
    data/org.lingmo.appwidget.manager.service \
    data/lingmo-appwidget-manager.desktop

LIBS += -llingmo-log4qt
