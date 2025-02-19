QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lingmo-welcome
TEMPLATE = app

# 添加翻译支持
TRANSLATIONS += translations/zh_CN.ts \
                translations/zh_TW.ts \
                translations/zh_HK.ts \
                translations/ja_JP.ts \
                translations/ko_KR.ts \
                translations/ru_RU.ts \
                translations/fr_FR.ts \
                translations/de_DE.ts \
                translations/es_ES.ts \
                translations/it_IT.ts

# 自动更新和生成翻译文件
!system(lupdate $$PWD/welcome.pro): error("Failed to update translations")
!system(lrelease $$PWD/welcome.pro): error("Failed to release translations")

SOURCES += \
    main.cpp \
    welcome_window.cpp \
    ripple_button.cpp \
    theme_page.cpp \
    welcome_manager.cpp \
    dock_page.cpp \
    finish_page.cpp

HEADERS += \
    welcome_window.h \
    ripple_button.h \
    theme_page.h \
    welcome_manager.h \
    dock_page.h \
    finish_page.h

RESOURCES += \
    welcome.qrc \
    translations.qrc 