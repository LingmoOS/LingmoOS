#! /usr/bin/env bash
$XGETTEXT `find . -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/lingmo_applet_org.kde.lingmo.%{APPNAMELC}.pot
