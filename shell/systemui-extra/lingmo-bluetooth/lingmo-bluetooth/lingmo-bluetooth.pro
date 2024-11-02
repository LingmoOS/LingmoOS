TARGET = lingmo-bluetooth
DESTDIR = .
TEMPLATE = app

include(../qtsingleapplication/qtsingleapplication.pri)
include(../environment.pri)

QT       += core gui dbus KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig USE_OBEX

PKGCONFIG += gsettings-qt \
             lingmosdk-waylandhelper \
             gio-2.0 \
             explor \
             lingmosdk-sysinfo \
	     lingmosdk-qtwidgets

# 适配窗口管理器圆角阴影
LIBS += -lpthread
LIBS += -lX11 -lXrandr -lXinerama -lXi -lXcursor
LIBS += -L /usr/lib/x86_64-linux-gnu -l KF5BluezQt -lgio-2.0 -lglib-2.0 -llingmo-log4qt

QMAKE_LFLAGS += -D_FORTIFY_SOURCE=2 -O2

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

inst1.files += ../data/org.bluez.Agent1.conf
inst1.path = $$CONF_INSTALL_DIR
inst2.files += ../data/org.lingmo.bluetooth.gschema.xml
inst2.path = $$SCHEMAS_INSTALL_DIR
inst3.files += ../data/no-bluetooth.svg
inst3.path = $$SHARE_INSTALL_DIR
inst4.files += ../data/file-transfer-success.svg
inst4.path = $$SHARE_INSTALL_DIR
inst5.files += ../data/file-transfer-failed.svg
inst5.path = $$SHARE_INSTALL_DIR
inst6.files +=../data/connection-failed.svg
inst6.path = $$SHARE_INSTALL_DIR
qm_files.files += translations/*.qm
qm_files.path += $${SHARE_TRANSLATIONS_INSTALL_DIR}
target.source += $$TARGET
target.path = $$BIN_INSTALL_DIR


INSTALLS += inst1 \
    inst2 \
    inst3 \
    inst4 \
    inst5 \
    inst6 \
    qm_files \
    target

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS 
DEFINES += QT_NO_INFO_OUTPUT
DEFINES += QT_NO_DEBUG_OUTPUT
exists(/usr/include/kysec/libkysec.h){
        DEFINES += KYSEC
        LIBS += -lkysec
}


# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

SOURCES += \
    config/kqpushbutton.cpp \
    config/x11devices.cpp \
    dbus/sysdbusinterface.cpp \
    popwidget/activeconnectionwidget.cpp \
    component/bluetoothsettinglabel.cpp \
    component/kyfiledialog.cpp \
    component/qdevitem.cpp \
    component/kyhline.cpp \
    config/config.cpp \
    popwidget/beforeturnoffhintwidget.cpp \
    popwidget/bluetoothfiletransferwidget.cpp \
    popwidget/errormessagewidget.cpp \
    popwidget/filereceivingpopupwidget.cpp \
    main/main.cpp \
    main/mainprogram.cpp \
    popwidget/trayicon.cpp \
    popwidget/traywidget.cpp \
    popwidget/pincodewidget.cpp \
    config/xatom-helper.cpp \
    dbus/bluetoothdbus.cpp \
    dbus/sessiondbusinterface.cpp

HEADERS += \
    config/kqpushbutton.h \
    config/x11devices.h \
    dbus/sysdbusinterface.h \
    popwidget/activeconnectionwidget.h \
    component/bluetoothsettinglabel.h \
    component/kyfiledialog.h \
    component/qdevitem.h \
    component/kyhline.h \
    config/config.h \
    popwidget/beforeturnoffhintwidget.h \
    popwidget/bluetoothfiletransferwidget.h \
    popwidget/errormessagewidget.h \
    popwidget/filereceivingpopupwidget.h \
    main/mainprogram.h \
    popwidget/trayicon.h \
    popwidget/traywidget.h \
    popwidget/pincodewidget.h \
    config/xatom-helper.h \
    dbus/bluetoothdbus.h \
    dbus/sessiondbusinterface.h


TRANSLATIONS += \
    translations/lingmo-bluetooth_zh_CN.ts \
    translations/lingmo-bluetooth_bo_CN.ts \
    translations/lingmo-bluetooth_zh_HK.ts \
    translations/lingmo-bluetooth_mn.ts \
    translations/lingmo-bluetooth_de.ts \
    translations/lingmo-bluetooth_es.ts \
    translations/lingmo-bluetooth_fr.ts \
    translations/lingmo-bluetooth_kk.ts \
    translations/lingmo-bluetooth_ky.ts \
    translations/lingmo-bluetooth_ug.ts \
    translations/lingmo-bluetooth_zh_Hant.ts

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    lingmo-bluetooth.qrc

FORMS += \
    popwidget/beforeturnoffhintwidget.ui
