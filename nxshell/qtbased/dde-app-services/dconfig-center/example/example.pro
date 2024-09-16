TEMPLATE = app
TARGET = dconfig-example
QT -= gui

QT += dtkcore

CONFIG += c++11 console
CONFIG -= app_bundle

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
    $$PWD/main.cpp

QT += dbus

meta_file.files += \
    $$PWD/configs/example.json \
    $$PWD/configs/a/example.json

meta_file.base = $$PWD/configs
meta_file.appid = $$TARGET

meta_file2.files += \
    $$PWD/configs/example.json \

meta_file2.base = $$PWD/configs

meta_file3.files += \
    $$PWD/configs/example.json \

meta_file3.base = $$PWD/configs
meta_file3.commonid=true

DCONFIG_META_FILES += meta_file meta_file2 meta_file3

override_file.files += \
    $$PWD/configs/dconf-example.override.json \
    $$PWD/configs/a/dconf-example.override.a.json \
    $$PWD/configs/a/b/dconf-example.override.a.b.json

override_file.base = $$PWD/configs
override_file.meta_name = example
override_file.appid = $$TARGET

DCONFIG_OVERRIDE_FILES += override_file

load(dtk_install_dconfig)
