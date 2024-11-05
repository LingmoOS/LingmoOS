#-------------------------------------------------
#
# Project created by QtCreator 2020-06-23T10:30:39
#
#-------------------------------------------------

QT       += dbus

QT       += gui network core widgets

TARGET = PluginService
TEMPLATE = lib
DESTDIR = $$PWD/../../KPlugins
DEFINES += PLUGINSERVICE_LIBRARY

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
    backendThread/custominstallerprocess.cpp \
    backendThread/oeminstallerprocess.cpp \
    backendThread/quickinsatllerprocess.cpp \
    base_unit/checkedlineedit.cpp \
    newdemand_unit.cpp \
    pluginservice.cpp \
    kdir.cpp \
    kyamlcpp.cpp \
    klogger.cpp \
    kcommand.cpp \
    klinguist.cpp \
    kthread.cpp \
    kgeoipinfo.cpp \
    ksystemenv.cpp \
    sysInfo/kernelfilesystemfs.cpp \
    sysInfo/machine.cpp \
    ui_unit/clineeditlabel.cpp \
    ui_unit/keyeventcontrol.cpp \
    ui_unit/messagebox.cpp \
    unpacks/copyfiles.cpp \
    sysInfo/meminfo.cpp \
    sysInfo/swaps.cpp \
    sysInfo/mount.cpp \
    sysInfo/device_disk.cpp \
    base_unit/string_unit.cpp \
    base_unit/file_unit.cpp \
    ui_unit/comboxdelegate.cpp \
    ui_unit/comboxlistmodel.cpp \
    ksystemsetting_unit.cpp \
    ui_unit/middleframemanager.cpp \
    sysInfo/kpasswordcheck.cpp \
    ui_unit/arrowtooltip.cpp \
    ui_unit/AutoResize.cpp \
    ui_unit/xrandrobject.cpp


HEADERS += \
    backendThread/custominstallerprocess.h \
    backendThread/oeminstallerprocess.h \
    backendThread/quickinsatllerprocess.h \
    base_unit/checkedlineedit.h \
    newdemand_unit.h \
    pluginservice.h \
    pluginservice_global.h \
    kdir.h \
    kyamlcpp.h \
    klogger.h \
    serverdll.h \
    kcommand.h \
    klinguist.h \
    kthread.h \
    kgeoipinfo.h \
    ksystemenv.h \
    sysInfo/kernelfilesystemfs.h \
    sysInfo/machine.h \
    ui_unit/clineeditlabel.h \
    ui_unit/keyeventcontrol.h \
    ui_unit/messagebox.h \
    unpacks/copyfiles.h \
    sysInfo/meminfo.h \
    sysInfo/swaps.h \
    sysInfo/mount.h \
    sysInfo/device_disk.h \
    base_unit/string_unit.h \
    base_unit/file_unit.h \
    ui_unit/comboxdelegate.h \
    ui_unit/comboxlistmodel.h \
    ksystemsetting_unit.h \
    ui_unit/middleframemanager.h \
    sysInfo/kpasswordcheck.h \
    ui_unit/arrowtooltip.h \
    ui_unit/AutoResize.h \
    ui_unit/xrandrobject.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}


unix:!macx: LIBS += -L$$PWD/../../KPlugins/ -lReflex

INCLUDEPATH += $$PWD/../Reflex
DEPENDPATH += $$PWD/../Reflex


SUBDIRS += \
    PluginService.pro

unix:!macx: LIBS += -lpwquality

RESOURCES +=
