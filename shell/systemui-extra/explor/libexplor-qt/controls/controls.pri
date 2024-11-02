INCLUDEPATH += $$PWD

include(directory-view/directory-view.pri)
include(menu/menu.pri)
include(navigation-bar/navigation-bar.pri)
include(preview-page/preview-page.pri)
include(property-page/property-page.pri)
include(side-bar/side-bar.pri)
include(status-bar/status-bar.pri)
include(tool-bar/tool-bar.pri)
include(tab-page/tab-page.pri)

HEADERS += \
    $$PWD/icon-container.h              \
    $$PWD/tag-management.h

SOURCES += \
    $$PWD/icon-container.cpp            \
    $$PWD/tag-management.cpp

