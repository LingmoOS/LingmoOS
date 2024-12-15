QT              += core widgets dbus
CONFIG          += c++11 plugin link_pkgconfig
PKGCONFIG += dframeworkdbus dtkgui dtkwidget
include(../../../3rdparty/stub_linux/stub.pri)

TARGET = ut_record_time

QMAKE_CXXFLAGS += -g -Wno-error=deprecated-declarations -Wno-deprecated-declarations -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

#内存检测标签
TSAN_TOOL_ENABLE = true
equals(TSAN_TOOL_ENABLE, true ){
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
    message("lingmo-screen-recorder enabled TSAN function with set: " $$TSAN_TOOL_ENABLE)
    contains(DEFINES, TSAN_THREAD){
       QMAKE_CXXFLAGS+="-fsanitize=thread"
       QMAKE_CFLAGS+="-fsanitize=thread"
       QMAKE_LFLAGS+="-fsanitize=thread"
    } else {
       QMAKE_CXXFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_CFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_LFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
    }
}

HEADERS += \
    ut_mock_pluginproxyinterface.h \
    ../../../src/ocean-dock-plugins/recordtime/timewidget.h \
    ../../../src/ocean-dock-plugins/recordtime/dbusservice.h \
    ../../../src/ocean-dock-plugins/recordtime/recordtimeplugin.h

SOURCES += \
    main.cpp \
    ../../../src/ocean-dock-plugins/recordtime/recordtimeplugin.cpp \
    ../../../src/ocean-dock-plugins/recordtime/timewidget.cpp \
    ../../../src/ocean-dock-plugins/recordtime/dbusservice.cpp \
    ut_mock_stub_recordtimeplugin.cpp \
    ut_timewidget.cpp \
    ut_dbusservice.cpp

INCLUDEPATH += /usr/include/ocean-dock
include(../../../3rdparty/googletest/gtest_dependency.pri)

