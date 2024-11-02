#-------------------------------------------------
#
# Project created by QtCreator 2020-12-04T10:41:29
#
#-------------------------------------------------

QT += core gui sql xml
CONFIG += c++11 link_pkgconfig
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -L$$[QT_INSTALL_LIBS] -lpolkit-qt5-core-1
LIBS += -ldl

PKGCONFIG += glib-2.0 lingmosdk-qtwidgets

TARGET = lingmo-uninstaller
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 适配窗口管理器圆角阴影
QT += KWindowSystem dbus x11extras

DEFINES += QT_NO_INFO_OUTPUT #屏蔽信息输出
DEFINES += QT_NO_WARNING_OUTPUT #屏蔽警告输出

DEFINES += QT_NO_DEBUG_OUTPUT  #屏蔽qDebug输出

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11

PKGCONFIG += lingmosdk-qtwidgets lingmosdk-alm lingmosdk-log lingmosdk-waylandhelper
PKGCONFIG += lingmosdk-kabase
LIBS +=  -L/usr/lib/lingmosdk/lingmosdk-base -lkyconf


# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

QMAKE_CXXFLAGS += -g

SOURCES += \
    adaptscreeninfo.cpp \
    lib_fun.cpp \
        main.cpp \
        main_window.cpp \
    parts_title.cpp \
    parts_result.cpp \
    single.cpp \
    single_window.cpp \
    tool_thread.cpp \
    picturetowhite.cpp \
    debinfowidget.cpp \
    xatom-helper.cpp

HEADERS += \
    adaptscreeninfo.h \
    lib_fun.h \
    main_window.h \
    parts_title.h \
    parts_result.h \
    single.h \
    single_window.h \
    tool_thread.h \
    picturetowhite.h \
    debinfowidget.h \
    xatom-helper.h

TRANSLATIONS += ../translations/kre_uninstall_zh_CN.ts
TRANSLATIONS += ../translations/kre_uninstall_bo_CN.ts
TRANSLATIONS += ../translations/kre_uninstall_mn_MN.ts
TRANSLATIONS += ../translations/kre_uninstall_zh_HK.ts

configFile.files += ./config/irregularPackage.list
configFile.path = /usr/share/lingmo-installer/

translation.files += ./../translations/kre_uninstall_zh_CN.qm
translation.files += ./../translations/kre_uninstall_bo_CN.qm
translation.files += ./../translations/kre_uninstall_mn_MN.qm
translation.files += ./../translations/kre_uninstall_zh_HK.qm
translation.files += ./../translations/kre_uninstall*.qm

translation.path = /usr/share/kre/kre-install/translations/

polkit.files += org.lingmo.uninstaller.policy
polkit.path = /usr/share/polkit-1/actions/

target.path = /usr/bin

INSTALLS += target polkit translation configFile

RESOURCES += \
    image.qrc

DISTFILES += \
    ../translations/kre_uninstall_bo_CN.ts \
    ../translations/kre_uninstall_zh_CN.ts \
    ../translations/kre_uninstall_mn_MN.ts
