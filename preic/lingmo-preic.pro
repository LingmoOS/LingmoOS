QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lingmo-preic
TEMPLATE = app

# 确保使用 C++11
CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    welcomepage.cpp \
    installpage.cpp \
    translator.cpp \
    ripplebutton.cpp

HEADERS += \
    mainwindow.h \
    welcomepage.h \
    installpage.h \
    translator.h \
    ripplebutton.h

RESOURCES += \
    resources.qrc 

TRANSLATIONS += \
    translations/lingmo-preic_zh_CN.ts \
    translations/lingmo-preic_es.ts

# 添加翻译文件到资源
RESOURCES += \
    resources.qrc 

# 添加生成 .qm 文件的规则
system(lrelease translations/lingmo-preic_zh_CN.ts -qm translations/lingmo-preic_zh_CN.qm)
system(lrelease translations/lingmo-preic_es.ts -qm translations/lingmo-preic_es.qm) 