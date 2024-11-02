INCLUDEPATH += $$PWD
include(../../src/frontend/tools/tools.pri)
include(list-items/list-items.pri)
include(vpndetails/vpndetails.pri)
include(single-pages/single-pages.pri)

HEADERS += \
    $$PWD/vpnobject.h

SOURCES += \
    $$PWD/vpnobject.cpp
