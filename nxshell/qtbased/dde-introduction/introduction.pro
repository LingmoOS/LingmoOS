TARGET = dde-introduction
TEMPLATE = app
QT = core gui widgets dbus multimedia multimediawidgets
CONFIG += link_pkgconfig c++11
QT += dtkwidget
PKGCONFIG += dframeworkdbus libdmr gsettings-qt
DESTDIR    = $$_PRO_FILE_PWD_

greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}

QMAKE_CXXFLAGS+= -fPIE
QMAKE_LFLAGS += -pie

isEmpty(PREFIX){
    PREFIX = /usr
}

system("bash delete_video.sh")

HEADERS += \
    src/basemodulewidget.h \
    src/mainwindow.h \
    src/model.h \
    src/normalwindow.h \
    src/worker.h \
    src/modules/about.h \
    src/modules/desktopmodemodule.h \
    src/modules/iconmodule.h \
    src/modules/moduleinterface.h \
    src/modules/normalmodule.h \
    src/modules/support.h \
    src/modules/videowidget.h \
    src/modules/wmmodemodule.h \
    src/widgets/basewidget.h \
    src/widgets/borderwidget.h \
    src/widgets/bottomnavigation.h \
    src/widgets/navigationbutton.h \
    src/widgets/nextbutton.h \
    src/dvideowidget.h \
    src/modules/photoslide.h \
    src/widgets/bottombutton.h \
    src/widgets/coverphoto.h \
    src/widgets/iconbutton.h \
    src/widgets/closebutton.h \
    src/widgets/previousbutton.h


SOURCES += \
    src/basemodulewidget.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/model.cpp \
    src/normalwindow.cpp \
    src/worker.cpp \
    src/modules/about.cpp \
    src/modules/desktopmodemodule.cpp \
    src/modules/iconmodule.cpp \
    src/modules/moduleinterface.cpp \
    src/modules/normalmodule.cpp \
    src/modules/support.cpp \
    src/modules/videowidget.cpp \
    src/modules/wmmodemodule.cpp \
    src/widgets/basewidget.cpp \
    src/widgets/borderwidget.cpp \
    src/widgets/bottomnavigation.cpp \
    src/widgets/navigationbutton.cpp \
    src/widgets/nextbutton.cpp \
    src/dvideowidget.cpp \
    src/modules/photoslide.cpp \
    src/widgets/bottombutton.cpp \
    src/widgets/coverphoto.cpp \
    src/widgets/iconbutton.cpp \
    src/widgets/closebutton.cpp \
    src/widgets/previousbutton.cpp

RESOURCES += \
    dde-introduction.qrc

contains(TARGET_ARCH, x86_64) {

}else{
    DEFINES += DISABLE_VIDEO
    HEADERS -= src/modules/videowidget.h
    SOURCES -= src/modules/videowidget.cpp
    PKGCONFIG -= libdmr
}

desktop.path = $$PREFIX/share/applications/
desktop.files += $$PWD/dde-introduction.desktop

target.path = $$PREFIX/bin/

TRANSLATIONS = translations/dde-introduction.ts

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    system($$PWD/translate_generation.sh)
}

qm_files.path = $$PREFIX/share/dde-introduction/translations/
qm_files.files = translations/*.qm

icon.path = $$PREFIX/share/icons/hicolor/scalable/apps
icon.files = resources/dde-introduction.svg

INSTALLS += desktop target icon qm_files

#host_mips64 | host_sw_64 | host_aarch64: {
#    DEFINES += DISABLE_VIDEO
#    HEADERS -= src/modules/videowidget.h
#    SOURCES -= src/modules/videowidget.cpp
#    PKGCONFIG -= libdmr
#}

DEFINES += PROFESSIONAL
videos.path = $$PREFIX/share/dde-introduction/
videos.files += resources/video/*.mp4
videos.files += resources/video/*.ass
INSTALLS += videos

CONFIG(release, debug|release) {
    TRANSLATIONS = $$files($$PWD/translations/*.ts)
    #遍历目录中的ts文件，调用lrelease将其生成为qm文件
    for(tsfile, TRANSLATIONS) {
        qmfile = $$replace(tsfile, .ts$, .qm)
        system(lrelease $$tsfile -qm $$qmfile) | error("Failed to lrelease")
    }
    #将qm文件添加到安装包
    dtk_translations.path = /usr/share/$$TARGET/translations
    dtk_translations.files = $$PWD/translations/*.qm
    INSTALLS += dtk_translations
}
