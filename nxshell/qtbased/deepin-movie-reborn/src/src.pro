
include($$PWD/install.pri)


TEMPLATE    = app
TARGET      = deepin-movie
#提供支持的额外的库
CONFIG      += c++11 link_pkgconfig
DESTDIR     = $$BUILD_DIST/bin

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += core network widgets sql concurrent dbus x11extras dtkwidget dtkcore

#包含目录
INCLUDEPATH += ../
INCLUDEPATH += ./common
INCLUDEPATH += ./widgets
INCLUDEPATH += ./backends/mpv
INCLUDEPATH += ./vendor
INCLUDEPATH += ./vendor/mpris-qt
INCLUDEPATH += ./vendor/dbusextended-qt
INCLUDEPATH += ../backends/mpv
#LIBS += -L$$BUILD_DIST/lib/ -ldmusic

INCLUDEPATH += $$PWD/libdmr
#DEPENDPATH += $$PWD/../libdmr


BUILD_DIST = ./dist
SYSTEMLIBDIR = /usr/lib/x64_64-linux-gnu/
#依赖目录
DEPENDPATH += $$BUILD_DIST/lib
#生成目标路径
DESTDIR     = $$BUILD_DIST/lib
#链接库
LIBS += -LSYSTEMLIBDIR -lX11
LIBS += -LSYSTEMLIBDIR -lxcb
LIBS += -LSYSTEMLIBDIR -lXtst
LIBS += -LSYSTEMLIBDIR -lxcb-shape
LIBS += -LSYSTEMLIBDIR -lmpv
LIBS += -LSYSTEMLIBDIR -lffmpegthumbnailer
LIBS += -LSYSTEMLIBDIR -lavformat
LIBS += -LSYSTEMLIBDIR -lavutil
LIBS += -LSYSTEMLIBDIR -lavcodec

LIBS += -L$$BUILD_DIST/lib/ -llibdmr



# Input
HEADERS += common/actions.h \
           common/dbus_adpator.h \
           common/dmr_settings.h \
           common/event_monitor.h \
           common/event_relayer.h \
           common/mainwindow.h \
           common/options.h \
           common/shortcut_manager.h \
           common/singleton.h \
           common/threadpool.h \
           common/thumbnail_worker.h \
           common/utility.h \
           common/volumemonitoring.h \
           vendor/movieapp.h \
           vendor/presenter.h \
           widgets/animationlabel.h \
           widgets/burst_screenshots_dialog.h \
           widgets/dmr_lineedit.h \
           widgets/movie_progress_indicator.h \
           widgets/movieinfo_dialog.h \
           widgets/notification_widget.h \
           widgets/playlist_widget.h \
           widgets/slider.h \
           widgets/tip.h \
           widgets/titlebar.h \
           widgets/toolbox_proxy.h \
           widgets/toolbutton.h \
           widgets/url_dialog.h \
           widgets/videoboxbutton.h \
           backends/mpv/mpv_glwidget.h \
           backends/mpv/mpv_proxy.h \
           vendor/dbusextended-qt/dbusextended.h \
           vendor/dbusextended-qt/dbusextendedabstractinterface.h \
           vendor/dbusextended-qt/dbusextendedpendingcallwatcher_p.h \
           vendor/mpris-qt/mpris.h \
           vendor/mpris-qt/mpriscontroller.h \
           vendor/mpris-qt/mpriscontroller_p.h \
           vendor/mpris-qt/mprismanager.h \
           vendor/mpris-qt/mprisplayer.h \
           vendor/mpris-qt/mprisplayer_p.h \
           vendor/mpris-qt/mprisqt.h
SOURCES += main.cpp \
           common/actions.cpp \
           common/dbus_adpator.cpp \
           common/dmr_settings.cpp \
           common/event_monitor.cpp \
           common/event_relayer.cpp \
           common/mainwindow.cpp \
           common/options.cpp \
           common/settings_translation.cpp \
           common/shortcut_manager.cpp \
           common/threadpool.cpp \
           common/thumbnail_worker.cpp \
           common/utility_x11.cpp \
           common/volumemonitoring.cpp\
           vendor/movieapp.cpp \
           vendor/presenter.cpp \
           widgets/animationlabel.cpp \
           widgets/burst_screenshots_dialog.cpp \
           widgets/dmr_lineedit.cpp \
           widgets/movie_progress_indicator.cpp \
           widgets/movieinfo_dialog.cpp \
           widgets/notification_widget.cpp \
           widgets/playlist_widget.cpp \
           widgets/slider.cpp \
           widgets/tip.cpp \
           widgets/titlebar.cpp \
           widgets/toolbox_proxy.cpp \
           widgets/toolbutton.cpp \
           widgets/url_dialog.cpp \
           widgets/videoboxbutton.cpp \
           backends/mpv/mpv_glwidget.cpp \
           backends/mpv/mpv_proxy.cpp \
           vendor/dbusextended-qt/dbusextendedabstractinterface.cpp \
           vendor/dbusextended-qt/dbusextendedpendingcallwatcher.cpp \
           vendor/mpris-qt/mpris.cpp \
           vendor/mpris-qt/mpriscontroller.cpp \
           vendor/mpris-qt/mprismanager.cpp \
           vendor/mpris-qt/mprisplayer.cpp \
           vendor/mpris-qt/mprisplayeradaptor.cpp \
           vendor/mpris-qt/mprisplayerinterface.cpp \
           vendor/mpris-qt/mprisrootadaptor.cpp \
           vendor/mpris-qt/mprisrootinterface.cpp
RESOURCES += resources.qrc theme.qrc icons/theme-icons.qrc

