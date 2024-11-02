QT       += core gui sql widgets dbus x11extras KWindowSystem network svg 

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

INCLUDEPATH += /usr/include/mpv/
LIBS += -lmpv \

LIBS += -lexplor \

LIBS += -L/usr/lib/liblingmo-log4qt.so.1.0.0 -llingmo-log4qt

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

TRANSLATIONS += \
            translations/lingmo-music_zh_CN.ts \
            translations/lingmo-music_bo_CN.ts
!system($$PWD/translations/generate_translations_pm.sh): error("Failed to generate qm")

target.path = /usr/bin
target.source += $$TARGET

icon.path = /usr/share/pixmaps
icon.files = img/lingmo-music.png

desktop.path = /usr/share/applications/
desktop.files = lingmo-music.desktop

schemes.files += \
    data/org.lingmo-music-data.gschema.xml \
    data/org.lingmo.log4qt.lingmo-music.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/

simple.files = $$PWD/lingmo-music-plugins-simple/build/src/libsimple.so
simple.path = /usr/share/lingmo-music

dict.files +=$$PWD/lingmo-music-plugins-simple/build/cppjieba/dict
dict.path = /usr/bin/

qm_files.files = translations/*.qm
qm_files.path = /usr/share/lingmo-music/translations/

INSTALLS += \
    target  \
    icon    \
    desktop \
    schemes \
    simple \
    dict \
    qm_files



CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt taglib gio-unix-2.0 lingmosdk-qtwidgets  lingmosdk-waylandhelper
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += qtsingleapplication
DEPENDPATH += qtsingleapplication
# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11
# 虚拟键盘按键
LIBS +=-lXtst

# 解析音频文件
LIBS += -lavformat

#LIBS += -lsimple
LIBS += -lsqlite3

SOURCES += \
    UI/base/allpupwindow.cpp \
    UI/base/labedit.cpp \
    UI/base/mylabel.cpp \
    UI/base/widgetstyle.cpp \
    UI/dbusadapter.cpp \
    UI/globalsignal.cpp \
    UI/mainwidget.cpp \
    UI/player/miniwidget.cpp \
    UI/player/musicslider.cpp \
    UI/player/playbackmodewidget.cpp \
    UI/player/playsongarea.cpp \
    UI/player/searchedit.cpp \
    UI/player/searchresult.cpp \
    UI/player/sliderwidget.cpp \
    UI/base/xatom-helper.cpp \
    UI/search/musicsearchlistdelegate.cpp \
    UI/search/musicsearchlistmodel.cpp \
    UI/search/musicsearchlistview.cpp \
    UI/sidebar/leftsiderbarwidget.cpp \
    UI/sidebar/myscrollarea.cpp \
    UI/sidebar/mytoolbutton.cpp \
    UI/sidebar/sidebarwidget.cpp \
    UI/tableview/tablebaseview.cpp \
    UI/tableview/tablehistory.cpp \
    UI/tableview/tableone.cpp \
    UI/tableview/tableviewdelegate.cpp \
    UI/titlebar/menumodule.cpp \
    UI/titlebar/titlebar.cpp \
    UIControl/base/daemonipcdbus.cpp \
    UIControl/base/musicDataBase.cpp \
    UIControl/base/musicfileinformation.cpp \
    UIControl/global/global.cpp \
    UIControl/player/coreplayer/mmediaplayer.cpp \
    UIControl/player/coreplayer/mmediaplaylist.cpp \
    UIControl/player/player.cpp \
    UIControl/tableview/musiclistmodel.cpp \
    main.cpp



HEADERS += \
    UI/base/allpupwindow.h \
    UI/base/labedit.h \
    UI/base/mylabel.h \
    UI/base/widgetstyle.h \
    UI/dbusadapter.h \
    UI/globalsignal.h \
    UI/mainwidget.h \
    UI/player/miniwidget.h \
    UI/player/musicslider.h \
    UI/player/playbackmodewidget.h \
    UI/player/playsongarea.h \
    UI/player/searchedit.h \
    UI/player/searchresult.h \
    UI/player/sliderwidget.h \
    UI/base/xatom-helper.h \
    UI/search/musicsearchlistdelegate.h \
    UI/search/musicsearchlistmodel.h \
    UI/search/musicsearchlistview.h \
    UI/sidebar/leftsiderbarwidget.h \
    UI/sidebar/myscrollarea.h \
    UI/sidebar/mytoolbutton.h \
    UI/sidebar/sidebarwidget.h \
    UI/tableview/tablebaseview.h \
    UI/tableview/tablehistory.h \
    UI/tableview/tableone.h \
    UI/tableview/tableviewdelegate.h \
    UI/titlebar/menumodule.h \
    UI/titlebar/titlebar.h \
    UIControl/base/daemonipcdbus.h \
    UIControl/base/musicDataBase.h \
    UIControl/base/musicfileinformation.h \
    UIControl/global/global.h \
    UIControl/player/coreplayer/mmediaplayer.h \
    UIControl/player/coreplayer/mmediaplaylist.h \
    UIControl/player/player.h \
    UIControl/tableview/musiclistmodel.h \


# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    data/org.lingmo-music-data.gschema.xml \
    data/org.lingmo.log4qt.lingmo-music.gschema.xml \
    lingmo-music.desktop \
    translations/generate_translations_pm.sh \
    translations/lingmo-music_bo_CN.ts \
    translations/lingmo-music_zh_CN.ts

