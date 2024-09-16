#-------------------------------------------------
#
# Project created by QtCreator 2017-04-17T11:02:31
#
#-------------------------------------------------

QT       += core gui widgets dtkwidget

TARGET = deepin-font-preview-plugin
TEMPLATE = lib
CONFIG += plugin link_pkgconfig
PKGCONFIG += freetype2 fontconfig dde-file-manager

LIBS += -L$$OUT_PWD/../libdeepin-font-manager -ldeepin-font-manager
INCLUDEPATH += $$PWD/../libdeepin-font-manager

isEqual(ARCH, sw_64){
    DEFINES += SW_CPUINFO
    QMAKE_CXXFLAGS += -mieee
}

isEmpty(LIB_INSTALL_DIR) {
    PLUGINDIR = $$[QT_INSTALL_LIBS]/dde-file-manager/plugins
} else {
    PLUGINDIR = $$LIB_INSTALL_DIR/dde-file-manager/plugins
}

SOURCES += \
    main.cpp \
    fontpreview.cpp

HEADERS += \
    fontpreview.h
DISTFILES += \
    deepin-font-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/previews

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
