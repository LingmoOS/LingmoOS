TEMPLATE = app

QT += qml quick widgets gui dbus network

SOURCES += src/main.cpp \
    src/qmlloader.cpp \
    src/dataconverter.cpp \
    src/adjunctaide.cpp \
    src/adjunctuploader.cpp \
    src/adjunctuploadthread.cpp \
    src/datasender.cpp

RESOURCES += src/views.qrc

CONFIG += c++11

include (src/cutelogger/cutelogger.pri)

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

HEADERS += \
    src/qmlloader.h \
    src/dataconverter.h \
    src/adjunctaide.h \
    src/adjunctuploader.h \
    src/adjunctuploadthread.h \
    src/datasender.h

isEmpty(PREFIX){
    PREFIX = /usr
}

BINDIR = $$PREFIX/bin

target.path = $$BINDIR
INSTALLS += target
