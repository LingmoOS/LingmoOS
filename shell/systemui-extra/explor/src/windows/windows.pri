INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/trash-warn-dialog.h \
    $$PWD/about-dialog.h \
#    $$PWD/main-window-factory.h \
#    $$PWD/main-window.h \
    $$PWD/x11-window-manager.h

SOURCES += \
    $$PWD/trash-warn-dialog.cpp \
    $$PWD/about-dialog.cpp \
#    $$PWD/main-window-factory.cpp \
#    $$PWD/main-window.cpp \
    $$PWD/x11-window-manager.cpp

FORMS += \
    $$PWD/about-dialog.ui
