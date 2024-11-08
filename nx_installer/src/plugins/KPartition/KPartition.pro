#-------------------------------------------------
#
# Project created by QtCreator 2020-07-15T16:12:23
#
#-------------------------------------------------

QT       += widgets core

TARGET = KPartition
TEMPLATE = lib
DESTDIR = $$PWD/../../KPlugins
DEFINES += KPARTITION_LIBRARY

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
    coexistpartitionframe.cpp \
    delegate/full_partition_delegate.cpp \
    delegate/partitionmodel.cpp \
    delegate/validatestate.cpp \
    frames/encryptsetframe.cpp \
    frames/lineeditaddreduce.cpp \
    frames/mylabelicon.cpp \
    kpartition.cpp \
    modifypartitionframe.cpp \
    partman/datastruct.cpp \
    partman/device.cpp \
    partman/partition_server.cpp \
    operate/quikPartition.cpp \
    frames/diskinfoview.cpp \
    frames/levelscrolldiskview.cpp \
    createpartitionframe.cpp \
    frames/tableWidget/cbasetabledelegate.cpp \
    frames/tableWidget/cbasetableitem.cpp \
    frames/tableWidget/cbasetablewidget.cpp \
    frames/progresslabel.cpp \
    partman/filesystem.cpp \
    partman/proberos.cpp \
    mainpartframe.cpp \
    partman/partition_unit.cpp \
    partman/partition.cpp \
    quickpartitionframe.cpp \
    units/info_unit.cpp \
    partman/libparted_unit.cpp \
    partman/partitionrole.cpp \
    manualpartition_operator.cpp \
    partman/operationdisk.cpp \
    partman/partition_format.cpp \
    frames/disktype_name.cpp \
    partman/partition_usage.cpp \
    custompartitionframe.cpp \
    frames/tablewidgetview.cpp \
    prepareinstallframe.cpp \
    delegate/partition_delegate.cpp \
    frames/pushbuttonicon.cpp \
    delegate/custom_partition_delegate.cpp \
    frames/diskpartitioncolorprogress.cpp \
    delegate/customize_device_model.cpp

HEADERS += \
    coexistpartitionframe.h \
    delegate/full_partition_delegate.h \
    delegate/partitionmodel.h \
    delegate/validatestate.h \
    frames/encryptsetframe.h \
    frames/lineeditaddreduce.h \
    frames/mylabelicon.h \
    kpartition.h \
    kpartition_global.h \
    modifypartitionframe.h \
    partman/datastruct.h \
    partman/device.h \
    partman/partition_server.h \
    operate/quikPartition.h \
    frames/diskinfoview.h \
    frames/levelscrolldiskview.h \
    createpartitionframe.h \
    frames/tableWidget/cbasetabledelegate.h \
    frames/tableWidget/cbasetableitem.h \
    frames/tableWidget/cbasetablewidget.h \
    frames/progresslabel.h \
    partman/filesystem.h \
    partman/proberos.h \
    mainpartframe.h \
    partman/partition_unit.h \
    partman/partition.h \
    quickpartitionframe.h \
    units/info_unit.h \
    partman/libparted_unit.h \
    partman/partitionrole.h \
    manualpartition_operator.h \
    partman/operationdisk.h \
    partman/partition_format.h \
    frames/disktype_name.h \
    partman/partition_usage.h \
    custompartitionframe.h \
    frames/tablewidgetview.h \
    prepareinstallframe.h \
    delegate/partition_delegate.h \
    frames/pushbuttonicon.h \
    delegate/custom_partition_delegate.h \
    frames/diskpartitioncolorprogress.h \
    delegate/customize_device_model.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lparted

INCLUDEPATH += /usr/include/parted
DEPENDPATH += /usr/include/parted

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





