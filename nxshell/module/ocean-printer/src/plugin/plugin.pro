QT += core dbus
QT -= gui

TARGET = oceanprinter
TEMPLATE = lib
CONFIG += c++11

SOURCES += \
    plugin.cpp

QMAKE_CFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_CXXFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_LFLAGS += -z noexecstack -pie -z lazy

unix:!macx:{
LIBS += -lusb-1.0
}

HEADERS += \
    service.h \
    signalusb.h \
    signalcups.h

DISTFILES +=
linux {
isEmpty(PREFIX){
    PREFIX = /usr
}

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$[QT_INSTALL_LIBS]/lingmo-service-manager
} else {
    target.path = $$LIB_INSTALL_DIR/lingmo-service-manager
}

jsonfile.path =  $${PREFIX}/share/lingmo-service-manager/user
jsonfile.files = $${PWD}/ocean-printer.json

INSTALLS += target jsonfile
}

