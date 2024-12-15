#!/bin/sh
QT_VERSION=0.3.0
export QT_VERSION
QT_VER=0.3
export QT_VER
QT_VERSION_TAG=030
export QT_VERSION_TAG
QT_INSTALL_DOCS=/usr/share/qt5/doc
export QT_INSTALL_DOCS
BUILDDIR=/home/archermind/Desktop/QtXlsxWriter/src/xlsx
export BUILDDIR
exec /usr/lib/qt5/bin/qdoc "$@"
