QT += core gui dbus sql dtkwidget

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lingmo-defender-datainterface
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dframeworkdbus gsettings-qt xcb xcb-util gio-unix-2.0

INCLUDEPATH += \
    ../../lingmo-defender/src/window/modules

SOURCES += \
    main.cpp \
    operatedbusdata.cpp \
    csingleteon.cpp \
    window/modules/startup/dbustartmanager.cpp \
    window/modules/analysisapp/defenderprocessinfolist.cpp \
    window/modules/analysisapp/defenderprocessinfo.cpp \
    window/modules/common/defenderprocinfo.cpp \
    window/modules/common/defenderprocinfolist.cpp \
    window/modules/resourcemanager/socketcreatemanager.cpp \
    window/modules/resourcemanager/socketlistenmanager.cpp \
    window/modules/resourcemanager/appsstarttoend.cpp \
    window/modules/resourcemanager/resourcedialog/netaskdialog.cpp \
    window/modules/resourcemanager/resourcedialog/netdisabledialog.cpp\
    window/modules/resourcemanager/resourcedialog/remaskdialog.cpp \
    window/modules/resourcemanager/resourcedialog/remdisabledialog.cpp \
    window/modules/loginsafety/loginsafetysrvmodel.cpp \
    ../../lingmo-defender/src/window/modules/common/invokers/invokerfactory.cpp \
    ../../lingmo-defender/src/window/modules/common/invokers/dbusinvoker.cpp \
    ../../lingmo-defender/src/window/modules/common/invokers/lingmodefendersettingsInvoker.cpp \
    accessible/appaccessiblebase.cpp \
    accessible/appaccessiblebutton.cpp \
    accessible/appaccessiblelabel.cpp \
    accessible/appaccessibleqabstractbutton.cpp

HEADERS += \
    operatedbusdata.h \
    common.h \
    csingleteon.h \
    window/namespace.h \
    window/modules/startup/dbustartmanager.h \
    window/modules/analysisapp/defenderprocessinfolist.h \
    window/modules/analysisapp/defenderprocessinfo.h \
    window/modules/common/defenderprocinfo.h \
    window/modules/common/defenderprocinfolist.h \
    window/modules/resourcemanager/socketcreatemanager.h \
    window/modules/resourcemanager/socketlistenmanager.h \
    window/modules/resourcemanager/appsstarttoend.h \
    window/modules/resourcemanager/resourcedialog/netaskdialog.h \
    window/modules/resourcemanager/resourcedialog/netdisabledialog.h\
    window/modules/resourcemanager/resourcedialog/remaskdialog.h \
    window/modules/resourcemanager/resourcedialog/remdisabledialog.h \
    window/modules/loginsafety/loginsafetysrvmodel.h \
    ../../lingmo-defender/src/window/modules/common/invokers/dbusinvoker.h \
    ../../lingmo-defender/src/window/modules/common/invokers/invokerfactory.h  \
    ../../lingmo-defender/src/window/modules/common/invokers/invokerinterface.h \
    ../../lingmo-defender/src/window/modules/common/invokers/lingmodefendersettingsInvoker.h \
    accessible/appaccessiblelabel.h \
    accessible/appaccessiblebase.h \
    accessible/appaccessiblebutton.h \
    accessible/appaccessibleqabstrractbutton.h \
    accessible/appaccessible.h

LIBS += -L"libprocps" -lprocps

# 数据处理dbus接口相关文件
datainterfacedbus.files = com.lingmo.defender.datainterface.xml
datainterfacedbus.header_flags += \
        -i window/modules/common/defenderprocinfolist.h \
        -i /usr/include/libdframeworkdbus-2.0/types/defenderprocessinfo.h \
        -i /usr/include/libdframeworkdbus-2.0/types/defenderprocesslist.h

# 界面dbus接口相关文件
hmiscreendbus.files = ../../lingmo-defender/com.lingmo.defender.hmiscreen.xml
# 流量监控dbus接口相关文件
monitornetflowdbus.files = ../lingmo-monitornetflow/com.lingmo.defender.MonitorNetFlow.xml
monitornetflowdbus.header_flags += \
        -i window/modules/common/defenderprocinfolist.h

DBUS_ADAPTORS += datainterfacedbus
DBUS_INTERFACES += datainterfacedbus hmiscreendbus monitornetflowdbus

CONFIG(release, debug|release) {
    TRANSLATIONS = $$files($$PWD/translations/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    #将qm文件添加到安装包
    dtk_translations.path = /usr/share/$$TARGET/translations
    dtk_translations.files = $$PWD/translations/*.qm
    INSTALLS += dtk_translations
}

target.path = /usr/bin/

install_service.files = com.lingmo.defender.datainterface.service
install_service.path = /usr/share/dbus-1/services

INSTALLS += target install_service

DISTFILES += \
    com.lingmo.defender.datainterface.service \
    window/modules/trafficdetails/lingmo-defender-trafficdetails.desktop
