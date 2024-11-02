INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
QT *= network
greaterThan(QT_MAJOR_VERSION, 4): QT *= widgets

qtsingleapplication-uselib:!qtsingleapplication-buildlib {
    LIBS += -L$$QTSINGLEAPPLICATION_LIBDIR -l$$QTSINGLEAPPLICATION_LIBNAME
} else {
    SOURCES +=
    HEADERS +=
}

win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QT_QTSINGLEAPPLICATION_EXPORT
    else:qtsingleapplication-uselib:DEFINES += QT_QTSINGLEAPPLICATION_IMPORT
}

HEADERS += \
    $$PWD/qt-local-peer.h \
    $$PWD/qt-locked-file.h \
    $$PWD/qt-single-application.h

SOURCES += \
    $$PWD/qt-local-peer.cpp \
    $$PWD/qt-locked-file-unix.cpp \
    $$PWD/qt-single-application.cpp \
    $$PWD/qt-locked-file.cpp
