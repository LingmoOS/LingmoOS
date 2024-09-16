QT              += widgets dbus
TEMPLATE        = lib
CONFIG          += c++11 plugin link_pkgconfig
QT              += dtkgui
QT              += dtkwidget
PKGCONFIG += dframeworkdbus

#TARGET          = deepin-screen-recorder-plugin
TARGET          = $$qtLibraryTarget(deepin-screen-recorder-plugin)
DISTFILES       += recordtime.json

HEADERS += \
    recordtimeplugin.h \
    timewidget.h \
    dbusservice.h

SOURCES += \
    recordtimeplugin.cpp \
    timewidget.cpp \
    dbusservice.cpp

target.path = /usr/lib/dde-dock/plugins/

INSTALLS += target

RESOURCES += res.qrc
