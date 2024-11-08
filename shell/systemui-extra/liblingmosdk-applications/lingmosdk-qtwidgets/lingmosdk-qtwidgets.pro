QT += widgets core sql x11extras KWindowSystem dbus

TEMPLATE = lib
DEFINES += GUI_LIBRARY

CONFIG += c++11

VERSION = 1.2.0

# 适配窗口管理器圆角阴影
LIBS +=-lX11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_LINGMO_PROTOCOL

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

INCLUDEPATH += ../lingmosdk-waylandhelper/src
LIBS += -L../lingmosdk-waylandhelper -llingmosdk-waylandhelper

TRANSLATIONS += ./translations/gui_zh_CN.ts \
                ./translations/gui_bo_CN.ts \
                ./translations/gui_zh_HK.ts \
                ./translations/gui_mn.ts \
                ./translations/gui_de.ts \
                ./translations/gui_es.ts \
                ./translations/gui_fr.ts \
                ./translations/gui_kk.ts \
                ./translations/gui_ug.ts \
                ./translations/gui_ky.ts

SOURCES += \
    src/parmscontroller.cpp\
    src/kbreadcrumb.cpp \
    src/kcolorcombobox.cpp \
    src/kpasswordedit.cpp \
    src/kpixmapcontainer.cpp \
    src/kprogressbar.cpp \
    src/kprogresscircle.cpp \
    src/ksecuritylevelbar.cpp \
    src/kbadge.cpp \
    src/kballontip.cpp \
    src/kdialog.cpp \
    src/kcommentpanel.cpp \
    src/kiconbar.cpp \
    src/kinputdialog.cpp \
    src/kmenubutton.cpp \
    src/knavigationbar.cpp \
    src/kprogressdialog.cpp \
    src/ksearchlineedit.cpp \
    src/kslider.cpp \
    src/ktag.cpp \
    src/ktoolbutton.cpp \
    src/kwindowbuttonbar.cpp \
    src/kuninstalldialog.cpp \
    src/kwidget.cpp \
    src/themeController.cpp \
    src/kaboutdialog.cpp \
    src/kborderbutton.cpp \
    src/kborderlessbutton.cpp \
    src/kswitchbutton.cpp \
    src/ktabbar.cpp \
    src/xatom-helper.cpp\
    src/kitemwidget.cpp\
    src/klistwidget.cpp\
	src/klistview.cpp\
    src/klistviewdelegate.cpp\
    src/kshadowhelper.cpp\
    src/kpressbutton.cpp\
    src/kpushbutton.cpp\
    src/ktranslucentfloor.cpp\
    src/kbubblewidget.cpp\
    src/kbuttonbox.cpp\
    src/kbackgroundgroup.cpp\
    src/klineframe.cpp\
    src/kcolorbutton.cpp\
    src/kmessagebox.cpp\
    src/kdragwidget.cpp\
    src/ksecurityquestiondialog.cpp

HEADERS += \
    src/parmscontroller.h\
    src/kbreadcrumb.h \
    src/kcolorcombobox.h \
    src/kpasswordedit.h \
    src/kpixmapcontainer.h \
    src/kprogressbar.h \
    src/kprogresscircle.h \
    src/ksecuritylevelbar.h \
    src/kbadge.h \
    src/kballontip.h \
    src/kdialog.h \
    src/kcommentpanel.h \
    src/kiconbar.h \
    src/kinputdialog.h \
    src/kmenubutton.h \
    src/knavigationbar.h \
    src/kprogressdialog.h \
    src/ksearchlineedit.h \
    src/kslider.h \
    src/ktag.h \
    src/ktoolbutton.h \
    src/kwindowbuttonbar.h \
    src/kuninstalldialog.h \
    src/kwidget.h \
    src/themeController.h \
    src/kaboutdialog.h \
    src/kborderbutton.h \
    src/kborderlessbutton.h \
    src/kswitchbutton.h \
    src/ktabbar.h \
    src/gui_g.h \
    src/xatom-helper.h \
    src/kitemwidget.h\
    src/klistwidget.h\
	src/klistview.h\
    src/klistviewdelegate.h\
    src/kshadowhelper.h\
    src/kpressbutton.h\
    src/kpushbutton.h\
    src/ktranslucentfloor.h\
    src/kbubblewidget.h\
    src/kbuttonbox.h\
    src/kbackgroundgroup.h\
    src/klineframe.h\
    src/kcolorbutton.h\
    src/kmessagebox.h\
    src/kdragwidget.h\
    src/ksecurityquestiondialog.h

   
# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/lingmosdk/applications/
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers

RESOURCES += \
    sdk_res.qrc

DISTFILES += \
    translations/gui_bo_CN.ts
