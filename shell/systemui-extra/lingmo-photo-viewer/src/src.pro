include (../codec/lingmoimagecodec/lingmoimagecodec.pro)
include (../codec/plugins/OCR/OCR.pri)
QT       += core gui
QT       += svg
QT       += dbus quickcontrols2
QT       += x11extras KWindowSystem printsupport network quick quickwidgets sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += no_keywords
CONFIG += link_pkgconfig
CONFIG += c++17
#这个是QT内嵌的版本号关键字，只能使用a.b.c.d纯数字形式，与changelog中的版本号不完全一致
VERSION = 1.3.0.3
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
##定义一个字符串，可以将版本号数字之后的部分添加进去
DEFINES += APP_VERSION_DETAIL=\\\"$$VERSION-ok13\\\"

QMAKE_CXXFLAGS += -g

#INCLUDEPATH += \
#/usr/include/glib-2.0/ \
#/usr/lib/x86_64-linux-gnu/glib-2.0/include/ \

PKGCONFIG += gio-unix-2.0
LIBS += -lexplor \

LIBS += -lX11
#适配sdk
PKGCONFIG += lingmosdk-qtwidgets lingmosdk-waylandhelper lingmosdk-log lingmosdk-config lingmosdk-utils lingmosdk-kabase

#sdk不一样的版本路径有更改
INCLUDEPATH += /usr/include/kabase/
INCLUDEPATH += /usr/include/lingmosdk/applications/kabase/lingmo_system/
INCLUDEPATH += /usr/include/kabase/lingmo_system/
INCLUDEPATH += /usr/include/lingmosdk/applications/kabase/

#INCLUDEPATH += ../kabase/
#INCLUDEPATH += ../kabase/Qt

CONFIG += c++11

TARGET = lingmo-photo-viewer
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

DEFINES += QT_DEPRECATED_WARNINGS

PKGCONFIG +=gio-2.0
LIBS +=-lgio-2.0 -lglib-2.0
#lingmo-image-codec
#INCLUDEPATH += /usr/include/lingmo-image-codec/
#LIBS += -llingmoimagecodec \


INCLUDEPATH += ../codec/plugins/OCR

#opencv
INCLUDEPATH += /usr/include/opencv4/
LIBS += -lopencv_core \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_dnn \

contains(QMAKE_HOST.arch, x86_64){
INCLUDEPATH += /usr/local/include/onnxruntime
LIBS += -L/usr/local/lib -lonnxruntime
}

contains(QMAKE_HOST.arch, aarch64){
INCLUDEPATH += /usr/local/include/onnxruntime
LIBS += -L/usr/local/lib -lonnxruntime
}

target.path = /usr/bin
target.source +=$$TARGET
INSTALLS += target

SOURCES += main.cpp \
    controller/core/coreinteraction.cpp \
    controller/core/albumthumbnail.cpp \
    controller/core/multitiffoperate.cpp \
    global/computingtime.cpp \
    global/horizontalorverticalmode.cpp \
    global/interactiveqml.cpp \
    global/log.cpp \
    global/mattinginteractiveqml.cpp \
    global/scannerinteractiveqml.cpp \
    imageproviderthumb.cpp \
    mattingimageprovider.cpp \
    model/dbus.cpp \
    controller/core/core.cpp \
    model/file/file.cpp \
    model/marktool/marktool.cpp \
    model/processing/flip.cpp \
    model/processing/processing.cpp \
    view/brushsettings.cpp \
    view/colordialog.cpp \
    view/colorsliderstyle.cpp \
    view/colorsquare.cpp \
    view/gradientslider.cpp \
    view/marking.cpp \
    view/maskwidget.cpp \
    view/multitifftoolbtn.cpp \
    view/ocrresultwidget.cpp \
    view/cutwidget.cpp \
    view/edit.cpp \
    view/information.cpp \
    view/kyview.cpp \
    global/variable.cpp \
    view/menumodule.cpp \
    view/navigator.cpp \
    view/openimage.cpp \
    view/painterthick.cpp \
    view/replacebackgroundwidget.cpp \
    view/scanimagewidget.cpp \
    view/showimagewidget.cpp \
    view/sidebar.cpp \
    view/sidebardelegate.cpp \
    view/sidelist.cpp \
    view/titlebar.cpp \
    view/toolbar.cpp \
    view/multitifftoolwid.cpp \

HEADERS += \
    controller/core/coreinteraction.h \
    controller/core/albumthumbnail.h \
    controller/core/base.h \
    controller/core/multitiffoperate.h \
    global/computingtime.h \
    global/horizontalorverticalmode.h \
    global/interactiveqml.h \
    global/log.h \
    global/mattinginteractiveqml.h \
    global/scannerinteractiveqml.h \
    imageprovider.h \
    imageproviderthumb.h \
    mattingimageprovider.h \
    model/dbus.h \
    controller/interaction.h \
    controller/core/core.h \
    model/file/file.h \
    model/marktool/marktool.h \
    model/processing/processingbase.h \
    model/processing/flip.h \
    model/processing/processing.h \
    scannerimageprovider.h \
    view/colordialog.h \
    view/colorsliderstyle.h \
    view/colorsquare.h \
    view/gradientslider.h \
    view/maskwidget.h \
    view/multitifftoolbtn.h \
    view/brushsettings.h \
    view/marking.h \
    view/ocrresultwidget.h \
    view/cutwidget.h \
    view/edit.h \
    view/information.h \
    view/kyview.h \
    global/variable.h \
    view/menumodule.h \
    view/navigator.h \
    view/openimage.h \
    view/painterthick.h \
    view/replacebackgroundwidget.h \
    view/scanimagewidget.h \
    view/showimagewidget.h \
    view/sidebar.h \
    view/sidebardelegate.h \
    view/sidelist.h \
    view/sizedate.h \
    view/titlebar.h \
    view/toolbar.h \
    view/multitifftoolwid.h \

RESOURCES += \
    ../res.qrc
