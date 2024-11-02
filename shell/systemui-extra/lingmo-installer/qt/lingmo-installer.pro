QT       += core gui xml dbus sql network QApt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lingmo-installer

CONFIG += c++11

LIBS          += -L$$[QT_INSTALL_LIBS] -lpolkit-qt5-core-1

# 适配窗口管理器圆角阴影
QT += KWindowSystem dbus x11extras

# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11
LIBS +=-ldl -lapt-pkg -lQApt

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt lingmosdk-soundeffects lingmosdk-qtwidgets

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS


#DEFINES += QT_NO_INFO_OUTPUT #屏蔽信息输出
#DEFINES += QT_NO_WARNING_OUTPUT #屏蔽警告输出

DEFINES += QT_NO_DEBUG_OUTPUT #屏蔽qDebug输出

#配置sdk
PKGCONFIG += lingmosdk-qtwidgets lingmosdk-alm lingmosdk-log lingmosdk-waylandhelper
PKGCONFIG += lingmosdk-kabase
LIBS +=  -L/usr/lib/lingmosdk/lingmosdk-base -lkyconf

INCLUDEPATH += /usr/include/lingmosdk/applications/
INCLUDEPATH += /usr/include/lingmosdk/lingmosdk-system/
INCLUDEPATH += /usr/include/lingmosdk/applications/kabase/lingmo_system/

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adaptscreeninfo.cpp \
    batch-install.cpp \
    dbuscallthread.cpp \
    debinfowidget.cpp \
    kareclass.cpp \
    lingmoinstalldbuscallthread.cpp \
    main.cpp \
    kreinstall.cpp \
    picturetowhite.cpp \
    titlewidget.cpp \
    widbox.cpp \
    xatom-helper.cpp \
    qtlocalpeer.cpp \
    qtlockedfile.cpp \
    qtlockedfile_unix.cpp \
    qtsingleapplication.cpp

HEADERS += \
    adaptscreeninfo.h \
    batch-install.h \
    dbuscallthread.h \
    debinfowidget.h \
    kareclass.h \
    kreinstall.h \
    lingmoinstalldbuscallthread.h \
    picturetowhite.h \
    titlewidget.h \
    widbox.h \
    xatom-helper.h \
    qtlocalpeer.h \
    qtlockedfile.h \
    qtsingleapplication.h


TRANSLATIONS += \
                ../translations/kre-install_zh_CN.ts\
                ../translations/kre-install_bo_CN.ts \
                ../translations/kre-install_mn_MN.ts \
                ../translations/kre-install_zh_HK.ts


polkit.path = /usr/share/polkit-1/actions/
polkit.files += org.lingmo.installer.policy
polkit.files += org.lingmo.install1.policy

translation.path = /usr/share/kre/kre-install/translations/
translation.files += ../translations/kre-install_zh_CN.qm
translation.files += ../translations/kre-install_bo_CN.qm
translation.files += ../translations/kre-install_mn_MN.qm
translation.files += ../translations/kre-install_zh_HK.qm
translation.files += ../translations/kre-install*.qm

desktop.path = /usr/share/applications/
desktop.files += lingmo-installer.desktop

guideCN.files += ./data/lingmo-installer/zh_CN/
guideCN.path += /usr/share/lingmo-user-guide/data/guide/lingmo-installer/

guideEN.files += ./data/lingmo-installer/en_US/
guideEN.path += /usr/share/lingmo-user-guide/data/guide/lingmo-installer/

imageformats.files += ./data/lingmo-installer/lingmo-installer.png
imageformats.path = /usr/share/lingmo-user-guide/data/guide/lingmo-installer/

# Default rules for deployment.
qnx: target.path = /usr/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target polkit translation desktop guideCN guideEN imageformats

RESOURCES += \
    kre-install.qrc

FORMS +=

DISTFILES += \
    ../translations/kre-install_bo_CN.ts \
    ../translations/kre-install_zh_CN.ts \
    ../translations/kre-install_mn_MN.ts \
    ../translations/kre-install_zh_HK.ts



