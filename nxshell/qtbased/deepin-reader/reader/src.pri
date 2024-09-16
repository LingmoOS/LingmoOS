INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/uiframe
INCLUDEPATH += $$PWD/widgets
INCLUDEPATH += $$PWD/sidebar
INCLUDEPATH += $$PWD/browser
INCLUDEPATH += $$PWD/document

include ($$PWD/app/app.pri)
include ($$PWD/browser/browser.pri)
include ($$PWD/sidebar/sidebar.pri)
include ($$PWD/widgets/widgets.pri)
include ($$PWD/document/document.pri)
include ($$PWD/uiframe/uiframe.pri)

SOURCES += \
    $$PWD/Application.cpp \
    $$PWD/MainWindow.cpp \
    $$PWD/load_libs.c

HEADERS +=\
    $$PWD/Application.h\
    $$PWD/MainWindow.h \
    $$PWD/load_libs.h

RESOURCES    += $$PWD/../resources/resources.qrc

#link
#由于自动化构建暂时无法自动下载sub module,目前手动内置第三方库
INCLUDEPATH += $$PWD/../3rdparty/deepin-pdfium/include
LIBS += -L$$PWD/../3rdparty/deepin-pdfium/lib -ldeepin-pdfium -ldl
