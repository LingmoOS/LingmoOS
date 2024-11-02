QT += quick quickcontrols2 widgets core gui qml dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    appwidgetmodel.cpp \
    listmodel.cpp \
    main.cpp \


RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /usr/bin/
else: unix:!android: target.path = /usr/bin/
INSTALLS += target

HEADERS += \
    appwidgetmodel.h \
    listmodel.h

#QMAKE_LFLAGS += -Wl,-rpath=$$OUT_PWD/../../liblingmo-app-widget/liblingmo-appwidget-manager

LIBS += -L$$OUT_PWD/../../liblingmo-app-widget/liblingmo-appwidget-manager -llingmo-appwidget-manager
INCLUDEPATH += $$PWD/../../liblingmo-app-widget/liblingmo-appwidget-manager
DEPENDPATH += $$PWD/../../liblingmo-app-widget/liblingmo-appwidget-manager
