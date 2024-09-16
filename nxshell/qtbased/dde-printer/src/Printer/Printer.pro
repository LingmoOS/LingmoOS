QT += core gui network dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-printer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS QT_MESSAGELOGCONTEXT

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui

SOURCES += \
        main.cpp \
    ui/dprintersshowwindow.cpp \
    util/dprintermanager.cpp \
    util/dprinter.cpp \
    ui/printersearchwindow.cpp \
    util/dprintclass.cpp \
    util/ddestination.cpp \ 
    ui/dpropertysetdlg.cpp \
    ui/installdriverwindow.cpp \
    ui/renameprinterwindow.cpp \
    ui/jobmanagerwindow.cpp \
    ui/printerapplication.cpp \
    ui/installprinterwindow.cpp \
    ui/permissionswindow.cpp \
    util/connectedtask.cpp \
    ui/printertestpagedialog.cpp \
    ui/troubleshootdialog.cpp\
    util/dprintertanslator.cpp \
    ui/dprintersupplyshowdlg.cpp \
    util/refreshsnmpbackendtask.cpp \
    ui/printerhelpwindow.cpp \
    ui/switchwidget.cpp \
    ui/advancedsharewidget.cpp

RESOURCES +=         resources.qrc \
    icons/theme-icons.qrc

FORMS +=

HEADERS += \
    ui/dprintersshowwindow.h \
    util/dprintermanager.h \
    util/dprinter.h \
    ui/printersearchwindow.h \
    util/ddestination.h \
    util/dprintclass.h \   
    ui/dpropertysetdlg.h \
    ui/installdriverwindow.h \
    ui/renameprinterwindow.h \
    ui/jobmanagerwindow.h \
    ui/printerapplication.h \
    ui/installprinterwindow.h \
    ui/uisourcestring.h \
    ui/permissionswindow.h \
    util/connectedtask.h \
    ui/printertestpagedialog.h \
    ui/troubleshootdialog.h\
    util/dprintertanslator.h \
    ui/dprinterpropertytemplate.h \
    ui/dprintersupplyshowdlg.h \
    util/refreshsnmpbackendtask.h \
    ui/printerhelpwindow.h \
    ui/switchwidget.h \
    ui/advancedsharewidget.h

include(../Common/Common.pri)

INCLUDEPATH +=  \
                $$PWD/../cppcups \
                $$PWD/../Common/vendor \
                ui \
                util \
                $$PWD/../Common

DEPENDPATH += $$PWD/../cppcups

QMAKE_CFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fPIE -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_CXXFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fPIE -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_LFLAGS += -Wl,--as-needed,-O1 -z noexecstack -pie -z lazy

DESTDIR += $$PWD

unix:!macx:{
LIBS += -L$$PWD/../cppcups/ -lcppcups
LIBS += -lcups
}

DISTFILES +=

TRANSLATIONS  +=  translations/dde-printer.ts

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

linux {

isEmpty(PREFIX){
    PREFIX = /usr
}

target.path = $${PREFIX}/bin


desktop.path = $${PREFIX}/share/applications
desktop.files = $${PWD}/platform/linux/dde-printer.desktop

hicolor.path =  $${PREFIX}/share/icons/hicolor/48x48/apps
hicolor.files = $${PWD}/images/dde-printer.svg

trans.path =  $${PREFIX}/share/dde-printer/translations
trans.files = $${PWD}/translations/*.qm

polkit.path = $${PREFIX}/share/polkit-1/actions
polkit.files = $${PWD}/policy/com.deepin.pkexec.devPrinter.policy

dconfigfile.path = $${PREFIX}/share/dsg/configs/dde-printer
dconfigfile.files = $${PWD}/logconf/org.deepin.dde.printer.json

debugconf.path = $${PREFIX}/share/deepin-debug-config/deepin-debug-config.d
debugconf.files = $${PWD}/logconf/dde-printer_debug.json

logconf.path = $${PREFIX}/share/deepin-log-viewer/deepin-log.conf.d
logconf.files = $${PWD}/logconf/dde-printer.json

canonscript.path = /opt/deepin/dde-printer/printer-drivers/cndrvcups-capt/
canonscript.files = $${PWD}/policy/canon*

INSTALLS += target desktop hicolor trans polkit dconfigfile debugconf logconf canonscript
}

#从debian/rules里面获取定义好的VERSION，然后转成字符串
DEFINES += QMAKE_VERSION=\\\"$$VERSION\\\"
