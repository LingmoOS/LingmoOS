#-------------------------------------------------
#
# Project created by QtCreator 2017-06-26T14:20:36
#
#-------------------------------------------------

TEMPLATE = subdirs
SUBDIRS += \
    src/cppcups/cppcups.pro \
    src/Printer/Printer.pro \
    src/Deamon              \
    src/plugin              \
    src/service

CONFIG += ordered
DEFINES += QT_MESSAGELOGCONTEXT

manual_dir.files = $$PWD/assets/dde-printer
manual_dir.path = /usr/share/deepin-manual/manual-assets/application

INSTALLS += manual_dir
