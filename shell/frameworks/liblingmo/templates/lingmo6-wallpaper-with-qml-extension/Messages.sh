#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/lingmo_wallpaper_org.kde.lingmo.%{APPNAMELC}.pot
