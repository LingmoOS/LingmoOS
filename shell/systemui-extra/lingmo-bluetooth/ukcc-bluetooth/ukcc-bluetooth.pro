QT += gui dbus x11extras

include(../environment.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets dbus KWindowSystem

TEMPLATE = lib
CONFIG += plugin \
          c++11 \
          x11extras \
          link_pkgconfig

PKGCONFIG     += gsettings-qt  \
                 lingmosdk-qtwidgets \
                 lingmosdk-waylandhelper \
                 lingmosdk-sysinfo \
                 x11 \
                 xi

LIBS += -lukcc

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS 
DEFINES += UKCC_BLUETOOTH_DATA_BURIAL_POINT
DEFINES += QT_NO_INFO_OUTPUT
DEFINES += QT_NO_DEBUG_OUTPUT
#exists(/etc/apt/ota_version)
#{
#    DEFINES += DEVICE_IS_INTEL
#}

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bluetooth.cpp \
    bluetoothbottomwindow.cpp \
    bluetoothdbusservice.cpp \
    bluetoothdevicefunc.cpp \
    bluetoothdeviceitem.cpp \
    bluetoothdevicewindowitem.cpp \
    bluetoothmainerrorwindow.cpp \
    bluetoothmainloadingwindow.cpp \
    bluetoothmainnormalwindow.cpp \
    bluetoothmainwindow.cpp \
    bluetoothmiddlewindow.cpp \
    bluetoothnamelabel.cpp \
    bluetoothtopwindow.cpp \
    common.cpp \
    devicebase.cpp \
    devremovedialog.cpp \
    devrenamedialog.cpp \
    loadinglabel.cpp \
    ukccbluetoothconfig.cpp

HEADERS += \
    bluetooth.h \
    bluetoothbottomwindow.h \
    bluetoothdbusservice.h \
    bluetoothdevicefunc.h \
    bluetoothdeviceitem.h \
    bluetoothdevicewindowitem.h \
    bluetoothmainerrorwindow.h \
    bluetoothmainloadingwindow.h \
    bluetoothmainnormalwindow.h \
    bluetoothmainwindow.h \
    bluetoothmiddlewindow.h \
    bluetoothnamelabel.h \
    bluetoothtopwindow.h \
    common.h \
    config.h \
    devicebase.h \
    devremovedialog.h \
    devrenamedialog.h \
    loadinglabel.h \
    ukccbluetoothconfig.h

DISTFILES += ukcc-bluetooth.json

TRANSLATIONS += \
    translations/ukcc-bluetooth_zh_HK.ts    \
    translations/ukcc-bluetooth_zh_Hant.ts  \
    translations/ukcc-bluetooth_zh_CN.ts    \
    translations/ukcc-bluetooth_ug.ts       \
    translations/ukcc-bluetooth_mn.ts       \
    translations/ukcc-bluetooth_ky.ts       \
    translations/ukcc-bluetooth_kk.ts       \
    translations/ukcc-bluetooth_fr.ts       \
    translations/ukcc-bluetooth_es.ts       \
    translations/ukcc-bluetooth_en_US.ts    \
    translations/ukcc-bluetooth_de.ts       \
    translations/ukcc-bluetooth_bo_CN.ts

#system("lrelease translations/*.ts")

# Default rules for deployment.
unix {
#    target.path = $$[QT_INSTALL_PLUGINS]/generic
    target.path = $$FILES_INSTALL_DIR
}

qm_files.files += translations/*.qm
qm_files.path += $${SHARE_TRANSLATIONS_INSTALL_DIR}

ts_files.files += translations/*.ts
ts_files.path += $${SHARE_TRANSLATIONS_INSTALL_DIR}

INSTALLS += qm_files \
            ts_files \
            target

RESOURCES += \
    ukcc-bluetooth.qrc

OBJECTS_DIR = ./obj/
MOC_DIR = ./moc/
