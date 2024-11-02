QT -= gui
QT += dbus

CONFIG += c++11 console
CONFIG -= app_bundle
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
        clockprovider.cpp \
        main.cpp \

# Default rules for deployment.
qnx: target.path = /usr/bin
else: unix:!android: target.path = /usr/bin


clockservice.files += provider/org.lingmo.appwidget.provider.democlock.service
clockservice.path += /usr/share/dbus-1/services/
!isEmpty(target.path): INSTALLS += target clockservice


HEADERS += \
    clockprovider.h \

RESOURCES += \
    provider/src.qrc

#QMAKE_LFLAGS += -Wl,-rpath=$$OUT_PWD/../../../liblingmo-app-widget/liblingmo-appwidget-manager/
#QMAKE_LFLAGS += -Wl,-rpath=$$OUT_PWD/../../../liblingmo-app-widget/liblingmo-appwidget-provider/

LIBS += -L$$OUT_PWD/../../../liblingmo-app-widget/liblingmo-appwidget-manager -llingmo-appwidget-manager
LIBS += -L$$OUT_PWD/../../../liblingmo-app-widget/liblingmo-appwidget-provider -llingmo-appwidget-provider

INCLUDEPATH += $$PWD/../../../liblingmo-app-widget/liblingmo-appwidget-manager
DEPENDPATH += $$PWD/../../../liblingmo-app-widget/liblingmo-appwidget-manager

INCLUDEPATH += $$PWD/../../../liblingmo-app-widget/liblingmo-appwidget-provider
DEPENDPATH += $$PWD/../../../liblingmo-app-widget/liblingmo-appwidget-provider
