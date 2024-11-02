#-------------------------------------------------
#
# Project created by QtCreator 2018-10-19T15:29:47
#
#-------------------------------------------------

QT       += core gui x11extras dbus KWindowSystem svg concurrent network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lingmo-vpn
TEMPLATE = app

CONFIG += c++14 qt warn_on link_pkgconfig no_keywords
#CONFIG += release

PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 libnm libnma libsecret-1 gtk+-3.0 gsettings-qt libcap lingmosdk-qtwidgets lingmosdk-waylandhelper
PKGCONFIG +=lingmosdk-sysinfo

INCLUDEPATH += /usr/include/KF5/NetworkManagerQt

LIBS    +=  -L/usr/lib/ -lgsettings-qt -lX11 -lKF5NetworkManagerQt -llingmo-log4qt
#LIBS    += -lkysec

target.path = /usr/bin
target.source += $$TARGET
desktop.path = /etc/xdg/autostart/
desktop.files = lingmo-vpn.desktop
gschema.files = org.lingmo.lingmo-vpn.switch.gschema.xml
gschema.path = /usr/share/glib-2.0/schemas/
qm_files.path = $${PREFIX}/share/lingmo-nm/lingmo-vpn/
qm_files.files = translations/*.qm

INSTALLS += target \
        desktop \
        gschema \
        qm_files \

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CPPFLAGS *= $(shell dpkg-buildflags --get CPPFLAGS)
QMAKE_CFLAGS   *= $(shell dpkg-buildflags --get CFLAGS)
QMAKE_CXXFLAGS *= $(shell dpkg-buildflags --get CXXFLAGS)
QMAKE_LFLAGS   *= $(shell dpkg-buildflags --get LDFLAGS)

include(singleapplication/qt-single-application.pri)
include(backend/backend.pri)
include(frontend/frontend.pri)

SOURCES += \
    main.cpp

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

DISTFILES += \
    org.lingmo.lingmo-vpn.switch.gschema.xml

TRANSLATIONS += \
        translations/lingmo-vpn_zh_Hant.ts \
        translations/lingmo-vpn_zh_CN.ts \
        translations/lingmo-vpn_ug.ts \
        translations/lingmo-vpn_tr.ts \
        translations/lingmo-vpn_mn.ts \
        translations/lingmo-vpn_ky.ts \
        translations/lingmo-vpn_kk.ts \
        translations/lingmo-vpn_fr.ts \
        translations/lingmo-vpn_es.ts \
        translations/lingmo-vpn_de.ts \
        translations/lingmo-vpn_bo.ts\
        translations/lingmo-vpn_bo_CN.ts

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}
