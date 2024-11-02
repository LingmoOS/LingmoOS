QT       += core gui widgets dbus qml quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include(../../../lingmo-appwidget-manager/QtSingleApplication/qtsingleapplication.pri)


CONFIG += C++11

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
    main.cpp \
    mainwindow.cpp


HEADERS += \
    mainwindow.h


FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    translations/democlock_zh_CN.ts \
    translations/democlock_bo_CN.ts

# Default rules for deployment.
qnx: target.path = /usr/bin/
else: unix:!android: target.path = /usr/bin/

qml.files += data/democlock.qml
qml.path = /usr/share/appwidget/qml/
appwidgetcong.files  += data/democlock.conf
appwidgetcong.path = /usr/share/appwidget/config/
preview.files += data/democlock.png
preview.path = /usr/share/appwidgetdemo/clock/
qm_files.files += translations/*
qm_files.path = /usr/share/appwidget/translations/
INSTALLS += target qml appwidgetcong preview qm_files



RESOURCES += \
    src.qrc


DISTFILES += \ \
    QtSingleApplication/qtsingleapplication.pri \
    QtSingleApplication/qtsinglecoreapplication.pri
