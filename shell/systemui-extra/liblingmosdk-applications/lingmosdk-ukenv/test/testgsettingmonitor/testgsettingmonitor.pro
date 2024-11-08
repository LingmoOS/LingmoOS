QT += core

TARGET = testgsettingmonitor
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += gsettings-qt lingmosdk-ukenv

SOURCES += main.cpp