INCLUDEPATH += $$PWD

include(location-bar/location-bar.pri)
include(path-bar/path-bar.pri)

HEADERS += \
    $$PWD/navigation-tool-bar.h \
    $$PWD/advanced-location-bar.h \
    $$PWD/navigation-bar.h \
    $$PWD/preview-option-toolbar.h

SOURCES += \
    $$PWD/navigation-tool-bar.cpp \
    $$PWD/advanced-location-bar.cpp \
    $$PWD/navigation-bar.cpp \
    $$PWD/preview-option-toolbar.cpp
