HEADERS += \
    $$PWD/Model.h \
    $$PWD/PDFModel.h \
    $$PWD/DjVuModel.h

SOURCES += \
    $$PWD/PDFModel.cpp \
    $$PWD/DjVuModel.cpp \
    $$PWD/Model.cpp

INCLUDEPATH += $$PWD

DEFINES += INSTALL_PREFIX=\\\"$$PREFIX\\\"
