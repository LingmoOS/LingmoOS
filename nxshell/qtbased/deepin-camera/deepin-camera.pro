QT += core gui printsupport svg dbus concurrent dtkwidget multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

  qtHaveModule(opengl): QT += opengl


CONFIG += c++11 link_pkgconfig
TEMPLATE = app
TARGET = deepin-camera

isEmpty(PREFIX){
    PREFIX = /usr
}

QMAKE_CFLAGS_ISYSTEM = -I

DEFINES += LIBCHEESE_LIBRARY
#DEFINES +=QT_NO_WARNING_OUTPUT
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += DEEPIN_CAMERA_PRO

#SUBDIRS += src
include(src/src/basepub/basepub.pri)
include(libcam/libcam.pri)
include(src/src/qtsingleapplication/qtsingleapplication.pri)

SOURCES += \
    $$PWD/src/src/*.cpp \
    $$PWD/src/src/*.c \
    $$PWD/src/*.cpp \


HEADERS += \
    $$PWD/src/src/*.h \
    $$PWD/src/*.h


INCLUDEPATH +=/usr/include/libusb-1.0\
                /usr/include \
                $$PWD/src \
                $$PWD/src/src \
                $$PWD/libcam/libcam_v4l2core\
                $$PWD/libcam/libcam_render\
                $$PWD/libcam/libcam_encoder \
                $$PWD/libcam/libcam_audio \
                $$PWD/libcam/libcam \
                $$PWD/src/src/basepub \                
                $$PWD/src/src/accessibility \
                $$PWD/src/src/accessible \
                $$PWD/src/src/qtsingleapplication

LIBS += -L/lib/x86_64-linux-gnu\
        -ldl

LIBS += -LSYSTEMLIBDIR

RESOURCES += \
    $$PWD/src/assets/assets.qrc

isEmpty(BINDIR):BINDIR=/usr/bin
isEmpty(APPDIR):APPDIR=/usr/share/applications
isEmpty(DSRDIR):DSRDIR=/usr/share/deepin-camera
isEmpty(PREFIX){
    PREFIX = /usr
}


target.path = $$INSTROOT$$BINDIR
icon_files.path = $$INSTROOT$$PREFIX/share/icons/hicolor/scalable/apps/
icon_files.files = $$PWD/src/assets/deepin-camera.svg

desktop.path = $$INSTROOT$$APPDIR
desktop.files = $$PWD/src/deepin-camera.desktop

manual_dir.files = $$PWD/src/assets/deepin-camera
manual_dir.path=/usr/share/deepin-manual/manual-assets/application/

dbus_service.files = $$PWD/src/com.deepin.Camera.service
dbus_service.path = $$PREFIX/share/dbus-1/services
unix {
    target.path = /usr/bin
#    INSTALLS += target
}

CONFIG(release, debug|release) {
    TRANSLATIONS = $$files($$PWD/src/translations/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
}

translations.path = $$PREFIX/share/deepin-camera/translations
translations.files = $$PWD/src/translations/*.qm

INSTALLS = target desktop dbus_service icon_files translations manual_dir
QMAKE_CXXFLAGS += -Wl,-as-need -fPIE
QMAKE_LFLAGS+=-pie

host_sw_64: {
# 在 sw_64 平台上添加此参数，否则会在旋转图片时崩溃
    QMAKE_CFLAGS += -mieee
    QMAKE_CXXFLAGS += -mieee
}

host_mips64:{
   QMAKE_CXX += -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi
   QMAKE_CXXFLAGS += -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,as-need -fPIE
   QMAKE_LFLAGS+=-pie
}

