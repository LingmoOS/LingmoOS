QT += widgets core dbus KWindowSystem

QMAKE_CXXFLAGS += -Wall -g

TEMPLATE = app

TARGET = lingmo-font-viewer

#check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

# about widget version
VERSION = 1.1.0.13-ok9
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += link_pkgconfig c++11

PKGCONFIG += gio-2.0 glib-2.0 gobject-2.0 gsettings-qt freetype2 fontconfig

# 配置lingmosdk
PKGCONFIG += lingmosdk-qtwidgets lingmosdk-waylandhelper lingmosdk-alm lingmosdk-ukenv lingmosdk-log lingmosdk-diagnostics

INCLUDEPATH += \
       kabase/ \
       kabase/Qt

LIBS += -lfontconfig -lfreetype 

SOURCES += $$PWD/src/main.cc \
           $$PWD/src/core.cc \
           $$PWD/src/libfun.cc \
           $$PWD/src/external.cc \
           $$PWD/src/view/baseiconbutton.cc \
           $$PWD/src/view/basepopuptitle.cc \
           $$PWD/src/view/basepreviewarea.cc \
           $$PWD/src/view/basesearchedit.cc \
           $$PWD/src/view/fontlistdelegate.cc \
           $$PWD/src/view/fontlistfilter.cc \
           $$PWD/src/view/fontlistmodel.cc \
           $$PWD/src/view/fontlistview.cc \
           $$PWD/src/view/fontwid.cc \
           $$PWD/src/view/functionwid.cc \
           $$PWD/src/view/globalsizedata.cc \
           $$PWD/src/view/mainview.cc \
           $$PWD/src/view/picturetowhite.cc \
           $$PWD/src/view/popupfontinfo.cc \
           $$PWD/src/view/popupinstallfail.cc \
           $$PWD/src/view/popupinstallsuccess.cc \
           $$PWD/src/view/popupremove.cc \
           $$PWD/src/view/titlebar.cc \
           $$PWD/src/view/getfontthread.cc \
           $$PWD/src/view/popuptips.cc \
           $$PWD/src/view/blankpage.cc \
           $$PWD/src/view/rotatechangeinfo.cc \
           $$PWD/src/view/popupmessage.cc  \
           $$PWD/src/view/fontlistdelegateug.cpp

HEADERS += $$PWD/include/core.h \
           $$PWD/include/libfun.h \
           $$PWD/include/external.h \
           $$PWD/src/view/baseiconbutton.h \
           $$PWD/src/view/basepopuptitle.h \
           $$PWD/src/view/basepreviewarea.h \
           $$PWD/src/view/basesearchedit.h \
           $$PWD/src/view/fontlistdelegate.h \
           $$PWD/src/view/fontlistfilter.h \
           $$PWD/src/view/fontlistmodel.h \
           $$PWD/src/view/fontlistview.h \
           $$PWD/src/view/fontwid.h \
           $$PWD/src/view/functionwid.h \
           $$PWD/src/view/globalsizedata.h \
           $$PWD/src/view/mainview.h \
           $$PWD/src/view/picturetowhite.h \
           $$PWD/src/view/popupfontinfo.h \
           $$PWD/src/view/popupinstallfail.h \
           $$PWD/src/view/popupinstallsuccess.h \
           $$PWD/src/view/popupremove.h \
           $$PWD/src/view/titlebar.h \
           $$PWD/src/view/getfontthread.h \
           $$PWD/src/view/popuptips.h \
           $$PWD/src/view/blankpage.h \
           $$PWD/src/view/rotatechangeinfo.h \
           $$PWD/src/view/popupmessage.h  \
           $$PWD/src/view/fontlistdelegateug.h

# 翻译
TRANSLATIONS += data/translations/lingmo-font-viewer_zh_CN.ts \
                data/translations/lingmo-font-viewer_bo_CN.ts \
                data/translations/lingmo-font-viewer_de.ts \
                data/translations/lingmo-font-viewer_fr.ts \
                data/translations/lingmo-font-viewer_kk_KZ.ts \
                data/translations/lingmo-font-viewer_ky_KG.ts \
                data/translations/lingmo-font-viewer_mn.ts \
                data/translations/lingmo-font-viewer_ug_CN.ts \
                data/translations/lingmo-font-viewer_zh_HK.ts \
                data/translations/lingmo-font-viewer_es.ts

!system($$PWD/data/translations/generate_translations_pm.sh): error("Failed to generate pm")

# 安装
qm_file.files = $$PWD/data/translations/*.qm
qm_file.path = /usr/share/lingmo-font-viewer/data/translations/

gsettings.files = $$PWD/data/org.lingmo.font.viewer.gschema.xml
gsettings.path = /usr/share/glib-2.0/schemas/

logger.files = $$PWD/data/org.lingmo.log4qt.lingmo-font-view.gschema.xml
logger.path = /usr/share/glib-2.0/schemas/

bin.files = $$TARGET
bin.path = /usr/bin/

guide.files = $$PWD/data/guide/lingmo-font-viewer/
guide.path = /usr/share/lingmo-user-guide/data/guide

desktop.files = $$PWD/data/lingmo-font-viewer.desktop
desktop.path = /usr/share/applications/

INSTALLS += qm_file \
            gsettings \
            bin \
            guide \
            logger \
            desktop \

RESOURCES += \
    image.qrc

DISTFILES += \
    data/translations/lingmo-font-viewer_bo_CN.ts \
    data/translations/lingmo-font-viewer_de.ts \
    data/translations/lingmo-font-viewer_fr.ts \
    data/translations/lingmo-font-viewer_kk.ts \
    data/translations/lingmo-font-viewer_kk_KZ.ts \
    data/translations/lingmo-font-viewer_ky.ts \
    data/translations/lingmo-font-viewer_ky_KG.ts \
    data/translations/lingmo-font-viewer_mn.ts \
    data/translations/lingmo-font-viewer_ug.ts \
    data/translations/lingmo-font-viewer_ug_CN.ts \
    data/translations/lingmo-font-viewer_zh_CN.ts \
    data/translations/lingmo-font-viewer_zh_HK.ts
