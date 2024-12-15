#-------------------------------------------------
#
# Project created by QtCreator 2017-04-17T11:02:31
#
#-------------------------------------------------

QT       += core gui widgets dtkwidget

TARGET = lingmo-font-preview-plugin
TEMPLATE = lib
CONFIG += plugin link_pkgconfig
PKGCONFIG += freetype2 fontconfig ocean-file-manager

LIBS += -L$$OUT_PWD/../liblingmo-font-manager -llingmo-font-manager
INCLUDEPATH += $$PWD/../liblingmo-font-manager

isEqual(ARCH, sw_64){
    DEFINES += SW_CPUINFO
    QMAKE_CXXFLAGS += -mieee
}

isEmpty(LIB_INSTALL_DIR) {
    PLUGINDIR = $$[QT_INSTALL_LIBS]/ocean-file-manager/plugins
} else {
    PLUGINDIR = $$LIB_INSTALL_DIR/ocean-file-manager/plugins
}

SOURCES += \
    main.cpp \
    fontpreview.cpp

HEADERS += \
    fontpreview.h
DISTFILES += \
    lingmo-font-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/previews

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
