QT       += widgets core gui dbus gui-private testlib sql charts

TARGET = lingmo-defender-test
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
    ../lingmo-defender/src/widgets/safetyprotectionitem.cpp \
    ../lingmo-defender/src/widgets/defendertable.cpp \
    ../lingmo-defender/src/widgets/titlebuttonitem.cpp \
    ../lingmo-defender/src/widgets/titledslideritem.cpp \
    ../lingmo-defender/src/widgets/oceanuislider.cpp \
    ../lingmo-defender/src/widgets/settingsitem.cpp \
    ../lingmo-defender/src/widgets/cleaneritem.cpp \
    ../lingmo-defender/src/widgets/titlelabel.cpp \
    ../lingmo-defender/src/widgets/defendertableheaderview.cpp \
    ../lingmo-defender/src/widgets/tipwidget.cpp  \
    ../lingmo-defender/src/window/modules/common/common.cpp \
    ../lingmo-defender/src/window/modules/cleaner/widgets/cleanerresultitemwidget.cpp \
    ../lingmo-defender/src/window/modules/cleaner/trashcleanitem.cpp \
    ../lingmo-defender/src/window/modules/cleaner/trashcleanwidget.cpp \
    ../lingmo-defender/src/window/modules/securitylog/securitylogdialog.cpp \
    ../lingmo-defender/src/window/modules/securitytools/startup/startupwidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/datausage/datausagemodel.cpp \
    ../lingmo-defender/src/window/modules/securitytools/datausage/datausagewidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/datausage/appdatadetaildialog.cpp \
    ../lingmo-defender/src/window/modules/securitytools/datausage/flowwidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/datausage/speedwidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/datausage/dateusagewidgets/appdatachart.cpp \
    ../lingmo-defender/src/window/modules/safetyprotection/safetyprotectionwidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/loginsafetymodel.cpp \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/loginsafetywidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/pwdlevelchangeddlg.cpp \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/pwdlimitlevelitem.cpp \
    ../lingmo-defender/src/window/modules/securitytools/internetcontrol/netcontrolwidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfolabel.cpp \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfodialog.cpp \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/usbconnectionwork.cpp \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/usbconnectionwidget.cpp \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/policykithelper.cpp \
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
    ../lingmo-defender/src/widgets/labels/tipslabel.h \
    ../lingmo-defender/src/widgets/labels/normallabel.h \
    ../lingmo-defender/src/widgets/labels/smalllabel.h \
    ../lingmo-defender/src/widgets/labels/largelabel.h \
    ../lingmo-defender/src/widgets/titlebuttonitem.h \
    ../lingmo-defender/src/widgets/titledslideritem.h \
    ../lingmo-defender/src/widgets/oceanuislider.h \
    ../lingmo-defender/src/widgets/settingsitem.h \
    ../lingmo-defender/src/widgets/cleaneritem.h \
    ../lingmo-defender/src/widgets/titlelabel.h \
    ../lingmo-defender/src/widgets/defendertable.h  \
    ../lingmo-defender/src/widgets/defendertableheaderview.h \
    ../lingmo-defender/src/window/modules/cleaner/widgets/cleanerresultitemwidget.h \
    ../lingmo-defender/src/window/modules/cleaner/trashcleanitem.h \
    ../lingmo-defender/src/window/modules/cleaner/trashcleanwidget.h \
    ../lingmo-defender/src/widgets/tipwidget.h  \
    ../lingmo-defender/src/window/modules/common/common.h \
    ../lingmo-defender/src/window/namespace.h \
    ../lingmo-defender/src/window/modules/cleaner/cleanerdbusadaptorinterface.h \
    ../lingmo-defender/src/window/modules/securitylog/securitylogadaptorinterface.h \
    ../lingmo-defender/src/window/modules/securitylog/securitylogdialog.h \
    ../lingmo-defender/src/window/modules/securitytools/startup/startupwidget.h \
    ../lingmo-defender/src/window/modules/securitytools/datausage/datausagemodel.h \
    ../lingmo-defender/src/window/modules/securitytools/datausage/datausagewidget.h \
    ../lingmo-defender/src/window/modules/securitytools/datausage/appdatadetaildialog.h \
    ../lingmo-defender/src/window/modules/securitytools/datausage/speedwidget.h \
    ../lingmo-defender/src/window/modules/securitytools/datausage/dateusagewidgets/appdatachart.h \
    ../lingmo-defender/src/window/modules/securitytools/datausage/flowwidget.h \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/loginsafetymodel.h \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/loginsafetywidget.h \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/pwdlevelchangeddlg.h \
    ../lingmo-defender/src/window/modules/securitytools/loginsafety/pwdlimitlevelitem.h \
    ../lingmo-defender/src/window/modules/securitytools/internetcontrol/netcontrolwidget.h \
    ../lingmo-defender/src/widgets/safetyprotectionitem.h \
    ../lingmo-defender/src/window/modules/safetyprotection/safetyprotectionwidget.h \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfolabel.h \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/widgets/usbinfodialog.h \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/usbconnectionwork.h \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/usbconnectionwidget.h \
    ../lingmo-defender/src/window/modules/securitytools/usbconnection/policykithelper.h \
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
    ../lingmo-defender/src \
    ../build/lingmo-defender \
    ../lingmo-defender/src/window \
    ../lingmo-defender/src/window/modules/common \
    ../lingmo-defender-server/lingmo-defender-datainterface/window/modules/common

RESOURCES += \
    testfiles.qrc
