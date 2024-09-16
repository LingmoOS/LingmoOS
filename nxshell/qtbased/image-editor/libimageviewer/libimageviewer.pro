#-------------------------------------------------
#
# Project created by QtCreator 2021-07-20T13:38:06
#
#-------------------------------------------------

QT       += widgets core gui svg dbus concurrent printsupport

TARGET = imageviewer
TEMPLATE = lib

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkcore
DEFINES += IMAGEVIEWER_LIBRARY

LIBS += -lfreeimage
DEFINES += LITE_DIV
DEFINES += USE_UNIONIMAGE

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include (icons/icons.qrc)
include (service/service.pri)
include (slideshow/slideshow.pri)
include (unionimage/unionimage.pri)
include (viewpanel/viewpanel.pri)
include (widgets/widgets.pri)

SOURCES += \
        imageviewer.cpp \
        imageengine.cpp

HEADERS += \
        imageviewer.h \
        imageengine.h \
        image-viewer_global.h
DESTDIR = $$PWD/../../out/

TRANSLATIONS += \
    translations/libimageviewer_zh_CN.ts \

CONFIG(release, debug|release) {
    TRANSLATIONS = $$files($$PWD/translations/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
}
translations.path = /usr/share/libimageviewer/translations
translations.files = $$PWD/translations/*.qm


unix {
    target.path = /usr/lib/
    INSTALLS += target translations
}

