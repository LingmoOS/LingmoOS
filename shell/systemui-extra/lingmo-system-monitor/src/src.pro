#-------------------------------------------------
#
# lingmo-system-monitor
#
#-------------------------------------------------

QT += core x11extras gui dbus network KWindowSystem charts xml concurrent

isEqual(QT_MAJOR_VERSION, 5) {
    QT += widgets gui svg x11extras
}

include(QtSingleApplication/qtsingleapplication.pri)
include(procnet-monitor/procnet-monitor.pri)

TARGET = lingmo-system-monitor
TEMPLATE = app
DESTDIR = ..
DEFINES += QT_MESSAGELOGCONTEXT
DEFINES += ENBALE_TRAY

LIBS += -L/usr/lib/ -lX11 -lpcap -lm -lpthread -llingmo-log4qt

CONFIG += link_pkgconfig \
                 c++11

PKGCONFIG += lingmosdk-waylandhelper \
             libgtop-2.0 \
             libsystemd \
             gsettings-qt \
             x11 \
             lingmosdk-utils \
             lingmosdk-qtwidgets \
             lingmosdk-diagnostics \
             lingmosdk-sysinfo \
             lingmosdk-proc \
             lingmosdk-hardware \
             lingmosdk-net \
             lingmosdk-realtime

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

ICON.files = img/lingmo-system-monitor.png
ICON.path = /usr/share/icons/hicolor/

desktop.files += ../lingmo-system-monitor.desktop
desktop.path = /usr/share/applications/

config.files += ../lingmo-system-monitor.conf
config.path = /usr/share/lingmo/

target.source += $$TARGET
target.path = /usr/bin

schemes.files += ../data/*.xml
schemes.path = /usr/share/glib-2.0/schemas/

INSTALLS += target \
            desktop \
            schemes \
            ICON \
            config

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

HEADERS += \
    controls/kchartview.h \
    controls/klabel.h \
    controls/kprogressbar.h \
    dialog/customservicenamedlg.h \
    dialog/uslider.h \
    filesystem/datacdrom.h \
    filesystem/filesysteminfoitem.h \
    filesystem/filesystemwidget.h \
    imageutil.h \
    kleftsideitem.h \
    kleftwidget.h \
    krightwidget.h \
    macro.h \
    mainwindow.h \
    process/processwidget.h \
    process/processwndinfo.h \
    service/kerror.h \
    service/dbuspropertiesinterface.h \
    service/servicecommon.h \
    service/servicedataworker.h \
    service/servicefileinfo.h \
    service/serviceinfo.h \
    service/servicemanager.h \
    service/servicesortfilterproxymodel.h \
    service/servicetablemodel.h \
    service/servicetableview.h \
    service/servicewidget.h \
    service/systemd1managerinterface.h \
    service/systemd1serviceinterface.h \
    sysresource/basedetailviewwidget.h \
    sysresource/chartviewwidget.h \
    sysresource/commoninfo.h \
    sysresource/cpudetailsmodel.h \
    sysresource/cpudetailswidget.h \
    sysresource/cpuhistorywidget.h \
    sysresource/detaillisttable.h \
    sysresource/lightlabel.h \
    sysresource/memdetailsmodel.h \
    sysresource/memdetailswidget.h \
    sysresource/memhistorywidget.h \
    sysresource/netdetailsmodel.h \
    sysresource/netdetailswidget.h \
    sysresource/nethistorywidget.h \
    sysresource/netviewscrollarea.h \
    trayicon.h \
    xatom-helper.h \
    util.h \
    desktopfileinfo.h \
    controls/mydialog.h \
    controls/myactiongroup.h \
    controls/myactiongroupitem.h \
    controls/myaction.h \
    controls/kitemdelegate.h \
    controls/kheaderview.h \
    controls/ktableview.h \
    controls/kgroupbutton.h \
    filesystem/filesystemworker.h \
    filesystem/filesystemdata.h \
    filesystem/filesystemwatcher.h \
    dialog/renicedialog.h \
    dialog/procpropertiesdlg.h \
    dialog/usmaboutdialog.h \
    process/processtableview.h \
    process/process_data.h \
    process/process_list.h \
    process/process_monitor.h \
    process/process_network.h \
    process/process_oplimit.h \
    process/processtablemodel.h \
    process/processsortfilterproxymodel.h \
    style/usmproxystyle.h
    

SOURCES += \
    controls/kchartview.cpp \
    controls/klabel.cpp \
    controls/kprogressbar.cpp \
    dialog/customservicenamedlg.cpp \
    dialog/uslider.cpp \
    filesystem/datacdrom.cpp \
    filesystem/filesysteminfoitem.cpp \
    filesystem/filesystemwidget.cpp \
    imageutil.cpp \
    kleftsideitem.cpp \
    kleftwidget.cpp \
    krightwidget.cpp \
    mainwindow.cpp \
    process/processwidget.cpp \
    process/processwndinfo.cpp \
    service/kerror.cpp \
    service/dbuspropertiesinterface.cpp \
    service/servicedataworker.cpp \
    service/servicefileinfo.cpp \
    service/serviceinfo.cpp \
    service/servicemanager.cpp \
    service/servicesortfilterproxymodel.cpp \
    service/servicetablemodel.cpp \
    service/servicetableview.cpp \
    service/servicewidget.cpp \
    service/systemd1managerinterface.cpp \
    service/systemd1serviceinterface.cpp \
    sysresource/basedetailviewwidget.cpp \
    sysresource/chartviewwidget.cpp \
    sysresource/commoninfo.cpp \
    sysresource/cpudetailsmodel.cpp \
    sysresource/cpudetailswidget.cpp \
    sysresource/cpuhistorywidget.cpp \
    sysresource/detaillisttable.cpp \
    sysresource/lightlabel.cpp \
    sysresource/memdetailsmodel.cpp \
    sysresource/memdetailswidget.cpp \
    sysresource/memhistorywidget.cpp \
    sysresource/netdetailsmodel.cpp \
    sysresource/netdetailswidget.cpp \
    sysresource/nethistorywidget.cpp \
    sysresource/netviewscrollarea.cpp \
    trayicon.cpp \
    xatom-helper.cpp \
    main.cpp \
    util.cpp \
    desktopfileinfo.cpp \
    controls/mydialog.cpp \
    controls/myactiongroup.cpp \
    controls/myactiongroupitem.cpp \
    controls/myaction.cpp \
    controls/kitemdelegate.cpp \
    controls/kheaderview.cpp \
    controls/ktableview.cpp \
    controls/kgroupbutton.cpp \
    filesystem/filesystemworker.cpp \
    filesystem/filesystemdata.cpp \
    filesystem/filesystemwatcher.cpp \
    dialog/renicedialog.cpp \
    dialog/procpropertiesdlg.cpp \
    dialog/usmaboutdialog.cpp \
    process/processtableview.cpp \
    process/process_list.cpp \
    process/process_monitor.cpp \
    process/process_network.cpp \
    process/process_oplimit.cpp \
    process/processtablemodel.cpp \
    process/processsortfilterproxymodel.cpp \
    style/usmproxystyle.cpp

RESOURCES += \
    res.qrc

TRANSLATIONS += \
    translation/lingmo-system-monitor_bo_CN.ts \
    translation/lingmo-system-monitor_zh_CN.ts \
    translation/lingmo-system-monitor_en_US.ts \
    translation/lingmo-system-monitor_mn.ts   \
    translation/lingmo-system-monitor_zh_HK.ts \
    translation/lingmo-system-monitor_kk.ts \    # 哈萨克
    translation/lingmo-system-monitor_ug.ts \    # 维吾尔
    translation/lingmo-system-monitor_ky.ts \    # 柯尔克孜
    translation/lingmo-system-monitor_de.ts \    # 德语
    translation/lingmo-system-monitor_es.ts \    # 西班牙
    translation/lingmo-system-monitor_fr.ts \    # 法语

DISTFILES += \
    img/nosearchresult_black.png \
    img/nosearchresult_white.png
