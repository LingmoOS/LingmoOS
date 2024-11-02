QT += core

TARGET = testcurrency
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += lingmosdk-ukenv

SOURCES += main.cpp