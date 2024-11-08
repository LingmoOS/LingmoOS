QT += widgets gui dbus

TEMPLATE = lib
DEFINES += LINGMOSDKWIDGETUTILS_LIBRARY

CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/kwidgetutils.cpp

HEADERS += \
    src/lingmosdk-widgetutils_global.h \
    src/kwidgetutils.h

# Default rules for deployment.
headers.files = $${HEADERS}
headers.path = /usr/include/lingmosdk/applications
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers
