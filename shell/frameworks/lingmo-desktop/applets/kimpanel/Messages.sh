#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml -o -name \*.js` -o $podir/lingmo_applet_org.kde.lingmo.kimpanel.pot
rm -f rc.cpp

