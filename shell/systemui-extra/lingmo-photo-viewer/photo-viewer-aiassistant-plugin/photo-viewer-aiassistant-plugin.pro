QT += core gui svg printsupport

#TARGET = photo-viewer-aiassistant-plugin
TEMPLATE = lib
QMAKE_PROJECT_NAME = photo-viewer-aiassistant-plugin

CONFIG += c++17 link_pkgconfig

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include (../codec/lingmoimagecodec/lingmoimagecodec.pro)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#opencv
INCLUDEPATH += /usr/include/opencv4/
LIBS += -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_dnn \

#onnxruntime
contains(QMAKE_HOST.arch, x86_64){
INCLUDEPATH += /usr/local/include/onnxruntime
LIBS += -L/usr/local/lib -lonnxruntime
}

contains(QMAKE_HOST.arch, aarch64){
INCLUDEPATH += /usr/local/include/onnxruntime
LIBS += -L/usr/local/lib -lonnxruntime
}

INCLUDEPATH += /usr/include/lingmo_image_codec/
#LIBS += -L/usr/lib -llingmoimagecodec

HEADERS += \
    photoviewerplugin.h

SOURCES += \
    photoviewerplugin.cpp

unix {
    target.path = $$[QT_INSTALL_LIBS]/
}
!isEmpty(target.path): INSTALLS += target

header.files += *.h
header.path = /usr/include/
INSTALLS += header
