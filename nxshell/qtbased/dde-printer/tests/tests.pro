TEMPLATE = app
CONFIG -= app_bundle
QT += core testlib dbus

CONFIG += testcase no_testcase_installs c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui

unix:QMAKE_RPATHDIR += $$OUT_PWD/../src
unix:LIBS += -lgtest -lcups

QMAKE_CXXFLAGS += -fno-access-control
QMAKE_LFLAGS += -fno-access-control

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g -fsanitize=address -Wall  -fprofile-arcs -ftest-coverage -O0
    QMAKE_LFLAGS += -g -fsanitize=address -Wall -fprofile-arcs -ftest-coverage  -O0
}

INCLUDEPATH += $$PWD/../src/cppcups

unix:LIBS += -L$$PWD/../src/cppcups -lcppcups

include($$PWD/cppcups/cppcups.pri)
include($$PWD/Common/Common.pri)

SOURCES += \
    $$PWD/main.cpp

DESTDIR += $$PWD

