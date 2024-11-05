INCLUDEPATH += $$PWD

include(intel/intel-sidebar.pri)

HEADERS += \
    $$PWD/file-label-box.h \
    $$PWD/header-bar.h \
    $$PWD/label-box-delegate.h \
    $$PWD/navigation-side-bar.h \
    $$PWD/navigation-tab-bar.h \
    $$PWD/operation-menu.h \
    $$PWD/search-widget.h \
    $$PWD/sort-type-menu.h \
    $$PWD/tab-status-bar.h \
    $$PWD/tab-widget.h \
    $$PWD/view-type-menu.h \
    $$PWD/float-pane-widget.h

SOURCES += \
    $$PWD/file-label-box.cpp \
    $$PWD/header-bar.cpp \
    $$PWD/label-box-delegate.cpp \
    $$PWD/navigation-side-bar.cpp \
    $$PWD/navigation-tab-bar.cpp \
    $$PWD/operation-menu.cpp \
    $$PWD/search-widget.cpp \
    $$PWD/sort-type-menu.cpp \
    $$PWD/tab-status-bar.cpp \
    $$PWD/tab-widget.cpp \
    $$PWD/view-type-menu.cpp \
    $$PWD/float-pane-widget.cpp
