#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg | grep -v '/tests/'` >> rc.cpp
$XGETTEXT `find . -name \*.js -o -name \*.qml -o -name \*.cpp | grep -v '/tests/'` -o $podir/lingmo_applet_org.kde.lingmo.private.grouping.pot
rm -f rc.cpp
