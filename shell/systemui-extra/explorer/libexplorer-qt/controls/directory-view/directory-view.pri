INCLUDEPATH += $$PWD

include(directory-view-factory/directory-view-factory.pri)
include(view/view.pri)
include(delegate/delegate.pri)

HEADERS += \
    $$PWD/directory-view-container.h \
    $$PWD/directory-view-widget.h \
    $$PWD/directoryviewhelper.h

SOURCES += \
    $$PWD/directory-view-container.cpp \
    $$PWD/directory-view-widget.cpp \
    $$PWD/directoryviewhelper.cpp
