INCLUDEPATH += $$PWD/../../src/Common

include($$PWD/../../src/Common/Common.pri)

SOURCES += $$PWD/ut_common.cpp \
    $$PWD/ut_qtconvert.cpp \
    $$PWD/ut_zsettings.cpp \
    $$PWD/ut_zjobmanager.cpp \
    $$PWD/ut_ztroubleshoot.cpp \
    $$PWD/ut_zdrivermanager.cpp
