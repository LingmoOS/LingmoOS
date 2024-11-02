INCLUDEPATH += $$PWD
include(tools/tools.pri)
include(xatom/xatom.pri)
include(tab-pages/tab-pages.pri)
include(list-items/list-items.pri)
include(netdetails/netdetails.pri)
include(enterprise-wlan/enterprise-wlan.pri)
include(connectivity/connectivity.pri)
include(networkmode/networkmode.pri)

FORMS +=

HEADERS += \
    $$PWD/customstyle.h \
    $$PWD/mainwindow.h

SOURCES += \
    $$PWD/customstyle.cpp \
    $$PWD/mainwindow.cpp

DISTFILES += \
    $$PWD/networkmode/networkmode.pri
