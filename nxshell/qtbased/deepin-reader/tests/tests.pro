#config
QT += core gui sql printsupport dbus testlib widgets

###安全漏洞检测
#QMAKE_CXX += -g -fsanitize=undefined,address -O2
#QMAKE_CXXFLAGS += -g -fsanitize=undefined,address -O2
#QMAKE_LFLAGS += -g -fsanitize=undefined,address -O2

CONFIG += c++11 link_pkgconfig resources_big testcase no_testcase_installs

#访问私有方法 -fno-access-control
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -fno-access-control -O0 -fno-inline

QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

#安全编译参数
QMAKE_CFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy
QMAKE_CXXFLAGS += -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy

PKGCONFIG += ddjvuapi dtkwidget

CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

#定义宏定义给UT代码使用（UT工程路径）
DEFINES += UTSOURCEDIR=\"\\\"$$PWD\\\"\"

LIBS += -lgtest

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
message("APP_VERSION: "$$VERSION)

#target
TARGET = test-deepin-reader

QMAKE_RPATHDIR += $$OUT_PWD/../3rdparty/deepin-pdfium/lib

TEMPLATE = app

#code
SOURCES += \
    main.cpp \
    ut_mainwindow.cpp \
    ut_common.cpp \
    ut_application.cpp \
    app/ut_database.cpp \
    widgets/ut_attrscrollwidget.cpp \
    widgets/ut_basewidget.cpp \
    widgets/ut_colorwidgetaction.cpp \
    widgets/ut_encryptionpage.cpp \
    widgets/ut_fileattrwidget.cpp \
    widgets/ut_findwidget.cpp \
    widgets/ut_handlemenu.cpp \
    widgets/ut_pagingwidget.cpp \
    widgets/ut_progressdialog.cpp \
    widgets/ut_roundcolorwidget.cpp \
    widgets/ut_savedialog.cpp \
    widgets/ut_scalemenu.cpp \
    widgets/ut_scalewidget.cpp \
    widgets/ut_securitydialog.cpp \
    widgets/ut_shortcutshow.cpp \
    widgets/ut_slideplaywidget.cpp \
    widgets/ut_slidewidget.cpp \
    widgets/ut_texteditwidget.cpp \
    widgets/ut_tipswidget.cpp \
    widgets/ut_transparenttextedit.cpp \
    widgets/ut_wordwraplabel.cpp \
    uiframe/ut_central.cpp \
    sidebar/ut_bookmarkdelegate.cpp \
    sidebar/ut_bookmarkwidget.cpp \
    sidebar/ut_catalogtreeview.cpp \
    sidebar/ut_catalogwidget.cpp \
    sidebar/ut_notesdelegate.cpp \
    sidebar/ut_noteswidget.cpp \
    sidebar/ut_readerimagethreadpoolmanager.cpp \
    sidebar/ut_searchresdelegate.cpp \
    sidebar/ut_searchreswidget.cpp \
    sidebar/ut_sidebarimagelistview.cpp \
    sidebar/ut_sidebarimageviewmodel.cpp \
    sidebar/ut_sheetsidebar.cpp \
    sidebar/ut_thumbnaildelegate.cpp \
    sidebar/ut_thumbnailwidget.cpp \
    browser/ut_sheetbrowser.cpp \
    document/ut_model.cpp \
    document/ut_pdfmodel.cpp \
    document/ut_djvumodel.cpp \
    browser/ut_browserannotation.cpp \
    browser/ut_browsermagnifier.cpp \
    browser/ut_browsermenu.cpp \
    browser/ut_browserpage.cpp \
    browser/ut_browserword.cpp \
    browser/ut_pagerenderthread.cpp \
    browser/ut_pagesearchthread.cpp \
    app/ut_dbusobject.cpp \
    app/ut_global.cpp \
    app/ut_debugtimemanager.cpp \
    app/ut_utils.cpp \
    uiframe/ut_centraldocpage.cpp \
    uiframe/ut_docsheet.cpp \
    uiframe/ut_doctabbar.cpp

HEADERS +=\
    ut_defines.h \
    ut_common.h

include($$PWD/../reader/src.pri)

RESOURCES += \
    files.qrc

#stub头文件路径
INCLUDEPATH += $$PWD/include/gtest

#安全测试选项
if(contains(DEFINES, CMAKE_SAFETYTEST_ARG_ON)){
    QMAKE_CFLAGS += -g -fsanitize=undefined,address -O2
    QMAKE_LFLAGS += -g -fsanitize=undefined,address -O2
    QMAKE_CXXFLAGS += -g -fsanitize=undefined,address -O2
}
