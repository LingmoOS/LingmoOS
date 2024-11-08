QT += widgets

TARGET = testusermanual
TEMPLATE = app

CONFIG += c++11 link_pkgconfig

PKGCONFIG += lingmosdk-ukenv

SOURCES += main.cpp