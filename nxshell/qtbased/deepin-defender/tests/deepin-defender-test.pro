QT       += widgets core gui dbus gui-private testlib sql charts

TARGET = deepin-defender-test
TEMPLATE = app

PKGCONFIG += dtkwidget dtkgui dtkcore gio-qt dframeworkdbus polkit-qt5-1 gio-unix-2.0
CONFIG += c++11 link_pkgconfig testcase no_testcase_installs
CONFIG -= app_bundle

LIBS += -lgtest -lgmock

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug|release) {
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -fsanitize=address -O2
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -fsanitize=address -O2
MAKE_CXX += -g -fprofile-arcs -ftest-coverage -fsanitize=address -O2
}

SOURCES += $$PWD/main.cpp \
    ../deepin-defender/src/widgets/safetyprotectionitem.cpp \
    ../deepin-defender/src/widgets/defendertable.cpp \
    ../deepin-defender/src/widgets/titlebuttonitem.cpp \
    ../deepin-defender/src/widgets/titledslideritem.cpp \
    ../deepin-defender/src/widgets/dccslider.cpp \
    ../deepin-defender/src/widgets/settingsitem.cpp \
    ../deepin-defender/src/widgets/cleaneritem.cpp \
    ../deepin-defender/src/widgets/titlelabel.cpp \
    ../deepin-defender/src/widgets/defendertableheaderview.cpp \
    ../deepin-defender/src/widgets/tipwidget.cpp  \
    ../deepin-defender/src/window/modules/common/common.cpp \
    ../deepin-defender/src/window/modules/cleaner/widgets/cleanerresultitemwidget.cpp \
    ../deepin-defender/src/window/modules/cleaner/trashcleanitem.cpp \
    ../deepin-defender/src/window/modules/cleaner/trashcleanwidget.cpp \
    ../deepin-defender/src/window/modules/securitylog/securitylogdialog.cpp \
    ../deepin-defender/src/window/modules/securitytools/startup/startupwidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/datausage/datausagemodel.cpp \
    ../deepin-defender/src/window/modules/securitytools/datausage/datausagewidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/datausage/appdatadetaildialog.cpp \
    ../deepin-defender/src/window/modules/securitytools/datausage/flowwidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/datausage/speedwidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/datausage/dateusagewidgets/appdatachart.cpp \
    ../deepin-defender/src/window/modules/safetyprotection/safetyprotectionwidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/loginsafetymodel.cpp \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/loginsafetywidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/pwdlevelchangeddlg.cpp \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/pwdlimitlevelitem.cpp \
    ../deepin-defender/src/window/modules/securitytools/internetcontrol/netcontrolwidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfolabel.cpp \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfodialog.cpp \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/usbconnectionwork.cpp \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/usbconnectionwidget.cpp \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/policykithelper.cpp \
    global/mockdbusinvoker/mockinvoker.cpp \
    global/mockdbusinvoker/mockinvokerfactory.cpp \
    global/mockgsettings/mockgsettings.cpp \
    testcase/Cleaner/ut_cleanerresultitemwidget.cpp \
    testcase/Cleaner/ut_transcleanitem.cpp \
    testcase/Cleaner/ut_trashcleanwidget.cpp \
    testcase/SecurityLog/ut_securitylogdlg.cpp \
    testcase/securitytools/startup/ut_startupwidget.cpp \
    testcase/securitytools/safetyprotection/ut_safetyprotectionwidget.cpp \
    testcase/securitytools/loginsafety/ut_loginsafetywidget.cpp \
    testcase/securitytools/datausage/ut_datausagemodel.cpp \
    testcase/securitytools/internetcontrol/ut_netcontrolwidget.cpp \
    testcase/securitytools/usbconnection/ut_usbconnectonwidget.cpp \
    testcase/homepage/ut_homepagewidget.cpp

HEADERS += \
    ../deepin-defender/src/widgets/labels/tipslabel.h \
    ../deepin-defender/src/widgets/labels/normallabel.h \
    ../deepin-defender/src/widgets/labels/smalllabel.h \
    ../deepin-defender/src/widgets/labels/largelabel.h \
    ../deepin-defender/src/widgets/titlebuttonitem.h \
    ../deepin-defender/src/widgets/titledslideritem.h \
    ../deepin-defender/src/widgets/dccslider.h \
    ../deepin-defender/src/widgets/settingsitem.h \
    ../deepin-defender/src/widgets/cleaneritem.h \
    ../deepin-defender/src/widgets/titlelabel.h \
    ../deepin-defender/src/widgets/defendertable.h  \
    ../deepin-defender/src/widgets/defendertableheaderview.h \
    ../deepin-defender/src/window/modules/cleaner/widgets/cleanerresultitemwidget.h \
    ../deepin-defender/src/window/modules/cleaner/trashcleanitem.h \
    ../deepin-defender/src/window/modules/cleaner/trashcleanwidget.h \
    ../deepin-defender/src/widgets/tipwidget.h  \
    ../deepin-defender/src/window/modules/common/common.h \
    ../deepin-defender/src/window/namespace.h \
    ../deepin-defender/src/window/modules/cleaner/cleanerdbusadaptorinterface.h \
    ../deepin-defender/src/window/modules/securitylog/securitylogadaptorinterface.h \
    ../deepin-defender/src/window/modules/securitylog/securitylogdialog.h \
    ../deepin-defender/src/window/modules/securitytools/startup/startupwidget.h \
    ../deepin-defender/src/window/modules/securitytools/datausage/datausagemodel.h \
    ../deepin-defender/src/window/modules/securitytools/datausage/datausagewidget.h \
    ../deepin-defender/src/window/modules/securitytools/datausage/appdatadetaildialog.h \
    ../deepin-defender/src/window/modules/securitytools/datausage/speedwidget.h \
    ../deepin-defender/src/window/modules/securitytools/datausage/dateusagewidgets/appdatachart.h \
    ../deepin-defender/src/window/modules/securitytools/datausage/flowwidget.h \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/loginsafetymodel.h \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/loginsafetywidget.h \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/pwdlevelchangeddlg.h \
    ../deepin-defender/src/window/modules/securitytools/loginsafety/pwdlimitlevelitem.h \
    ../deepin-defender/src/window/modules/securitytools/internetcontrol/netcontrolwidget.h \
    ../deepin-defender/src/widgets/safetyprotectionitem.h \
    ../deepin-defender/src/window/modules/safetyprotection/safetyprotectionwidget.h \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfolabel.h \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfodialog.h \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/usbconnectionwork.h \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/usbconnectionwidget.h \
    ../deepin-defender/src/window/modules/securitytools/usbconnection/policykithelper.h \
    global/cleanerenvironment.h \
    mock/mock_cleanerdbusinterface.h \
    mock/mock_securitylogdatainterface.h \
    global/securitylogenvironment.h \
    global/mockdbusinvoker/mockinvoker.h \
    global/mockdbusinvoker/mockinvokerfactory.h \
    global/mockgsettings/mockgsettings.h \
    testcase/Cleaner/ut_cleanerresultitemwidget.h \
    testcase/Cleaner/ut_transcleanitem.h \
    testcase/Cleaner/ut_trashcleanwidget.h \
    testcase/SecurityLog/ut_securitylogdlg.h  \
    testcase/securitytools/startup/ut_startupwidget.h \
    testcase/securitytools/safetyprotection/ut_safetyprotectionwidget.h \
    testcase/securitytools/loginsafety/ut_loginsafetywidget.h \
    testcase/securitytools/datausage/ut_datausagemodel.h \
    testcase/securitytools/internetcontrol/ut_netcontrolwidget.h \
    testcase/securitytools/usbconnection/ut_usbconnectonwidget.h \
    testcase/homepage/ut_homepagewidget.h

INCLUDEPATH += \
    ../deepin-defender/src \
    ../build/deepin-defender \
    ../deepin-defender/src/window \
    ../deepin-defender/src/window/modules/common \
    ../deepin-defender-server/deepin-defender-datainterface/window/modules/common

RESOURCES += \
    testfiles.qrc
